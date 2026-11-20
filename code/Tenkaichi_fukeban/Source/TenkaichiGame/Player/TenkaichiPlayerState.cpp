// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiPlayerState.h"

#include "AbilitySystem/Attributes/TenkaichiCombatSet.h"
#include "AbilitySystem/Attributes/TenkaichiHealthSet.h"
#include "AbilitySystem/TenkaichiAbilitySet.h"
#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "Character/TenkaichiPawnData.h"
#include "Character/TenkaichiPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameModes/TenkaichiExperienceManagerComponent.h"
//@TODO: 想把这部分隔离得更好一点，让 PawnData 能进来而不必知道其它东西
#include "GameModes/TenkaichiGameMode.h"
#include "TenkaichiLogChannels.h"
#include "TenkaichiPlayerController.h"
#include "Messages/TenkaichiVerbMessage.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiPlayerState)

class AController;
class APlayerState;
class FLifetimeProperty;

// 能力系统就绪事件名（"能力就绪"）。
const FName ATenkaichiPlayerState::NAME_TenkaichiAbilityReady("TenkaichiAbilitiesReady");

// 构造函数：创建 ASC 和属性集子对象，设置初始状态。
ATenkaichiPlayerState::ATenkaichiPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MyPlayerConnectionType(ELyraPlayerConnectionType::Player)
{
	// 创建能力系统组件子对象，并开启复制（用 Mixed 模式）。
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UTenkaichiAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// 这两个属性集会被 ASC::InitializeComponent 检测到。先持有引用，防止在被检测到前被垃圾回收。
	HealthSet = CreateDefaultSubobject<UTenkaichiHealthSet>(TEXT("HealthSet"));
	CombatSet = CreateDefaultSubobject<UTenkaichiCombatSet>(TEXT("CombatSet"));

	// 能力系统需要高频更新。
	SetNetUpdateFrequency(100.0f);

	MyTeamID = FGenericTeamId::NoTeam;
	MySquadID = INDEX_NONE;
}

// 组件初始化前调用。
void ATenkaichiPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

// 重置玩家状态（重生时调用）。
void ATenkaichiPlayerState::Reset()
{
	Super::Reset();
}

// 客户端初始化：检查 Pawn 扩展组件的默认初始化。
void ATenkaichiPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);

	if (UTenkaichiPawnExtensionComponent* PawnExtComp = UTenkaichiPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
	{
		PawnExtComp->CheckDefaultInitialization();
	}
}

// 复制属性到另一个 PlayerState（用于连接转移）。
void ATenkaichiPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	//@TODO: 复制统计信息
}

// 玩家被停用（如掉线）时调用。
void ATenkaichiPlayerState::OnDeactivated()
{
	bool bDestroyDeactivatedPlayerState = false;

	switch (GetPlayerConnectionType())
	{
		case ELyraPlayerConnectionType::Player:
		case ELyraPlayerConnectionType::InactivePlayer:
			//@TODO: 询问经验系统是否要立即销毁掉线玩家，还是保留一段时间
			// （例如长期运行的服务器上，大量玩家进进出出可能堆积）
			bDestroyDeactivatedPlayerState = true;
			break;
		default:
			bDestroyDeactivatedPlayerState = true;
			break;
	}
	
	SetPlayerConnectionType(ELyraPlayerConnectionType::InactivePlayer);

	if (bDestroyDeactivatedPlayerState)
	{
		Destroy();
	}
}

// 玩家被重新激活时调用。
void ATenkaichiPlayerState::OnReactivated()
{
	if (GetPlayerConnectionType() == ELyraPlayerConnectionType::InactivePlayer)
	{
		SetPlayerConnectionType(ELyraPlayerConnectionType::Player);
	}
}

// 经验（Experience）加载完成后的回调：从游戏模式拿到 PawnData 并设置。
void ATenkaichiPlayerState::OnExperienceLoaded(const UTenkaichiExperienceDefinition* /*CurrentExperience*/)
{
	if (ATenkaichiGameMode* TenkaichiGameMode = GetWorld()->GetAuthGameMode<ATenkaichiGameMode>())
	{
		if (const UTenkaichiPawnData* NewPawnData = TenkaichiGameMode->GetPawnDataForController(GetOwningController()))
		{
			SetPawnData(NewPawnData);
		}
		else
		{
			UE_LOG(LogTenkaichi, Error, TEXT("ATenkaichiPlayerState::OnExperienceLoaded(): 无法找到用于初始化玩家状态的 PawnData [%s]!"), *GetNameSafe(this));
		}
	}
}

// 声明哪些属性需要复制到客户端。
void ATenkaichiPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyPlayerConnectionType, SharedParams)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyTeamID, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MySquadID, SharedParams);

	// 观察旋转跳过拥有者（自己不用看自己的视角）。
	SharedParams.Condition = ELifetimeCondition::COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedViewRotation, SharedParams);

	DOREPLIFETIME(ThisClass, StatTags);	
}

// 获取复制给旁观者的观察旋转。
FRotator ATenkaichiPlayerState::GetReplicatedViewRotation() const
{
	// 这里可以换成自定义复制
	return ReplicatedViewRotation;
}

// 设置复制的观察旋转（只在本机标记脏并赋值）。
void ATenkaichiPlayerState::SetReplicatedViewRotation(const FRotator& NewRotation)
{
	if (NewRotation != ReplicatedViewRotation)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedViewRotation, this);
		ReplicatedViewRotation = NewRotation;
	}
}

// 获取本玩家对应的 PlayerController。
ATenkaichiPlayerController* ATenkaichiPlayerState::GetTenkaichiPlayerController() const
{
	return Cast<ATenkaichiPlayerController>(GetOwner());
}

// 实现 IAbilitySystemInterface：返回 ASC。
UAbilitySystemComponent* ATenkaichiPlayerState::GetAbilitySystemComponent() const
{
	return GetTenkaichiAbilitySystemComponent();
}

// 组件初始化后调用：初始化 ASC 的 Actor 信息，并注册经验加载回调。
void ATenkaichiPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());

	UWorld* World = GetWorld();
	if (World && World->IsGameWorld() && World->GetNetMode() != NM_Client)
	{
		AGameStateBase* GameState = GetWorld()->GetGameState();
		check(GameState);
		UTenkaichiExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UTenkaichiExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnTenkaichiExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
}

// 设置 PawnData（仅服务器权威），并授予对应的能力集。
void ATenkaichiPlayerState::SetPawnData(const UTenkaichiPawnData* InPawnData)
{
	check(InPawnData);

	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		UE_LOG(LogTenkaichi, Error, TEXT("尝试给已有有效 PawnData [%s] 的玩家状态 [%s] 设置新的 PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	// 把 PawnData 里的每个能力集都授予 ASC。
	for (const UTenkaichiAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
		}
	}

	// 广播"能力就绪"事件。
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_TenkaichiAbilityReady);
	
	ForceNetUpdate();
}

// PawnData 复制到客户端后触发。
void ATenkaichiPlayerState::OnRep_PawnData()
{
}

// 设置玩家连接类型。
void ATenkaichiPlayerState::SetPlayerConnectionType(ELyraPlayerConnectionType NewType)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyPlayerConnectionType, this);
	MyPlayerConnectionType = NewType;
}

// 设置玩家所属小队 ID（仅服务器）。
void ATenkaichiPlayerState::SetSquadID(int32 NewSquadId)
{
	if (HasAuthority())
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MySquadID, this);

		MySquadID = NewSquadId;
	}
}

// 设置玩家队伍（仅服务器），并广播队伍变更。
void ATenkaichiPlayerState::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (HasAuthority())
	{
		const FGenericTeamId OldTeamID = MyTeamID;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyTeamID, this);
		MyTeamID = NewTeamID;
		ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
	}
	else
	{
		UE_LOG(LogTenkaichiTeams, Error, TEXT("不能在非权威端给 %s 设置队伍"), *GetPathName(this));
	}
}

// 获取玩家当前队伍。
FGenericTeamId ATenkaichiPlayerState::GetGenericTeamId() const
{
	return MyTeamID;
}

// 返回队伍变更委托的指针。
FOnTenkaichiTeamIndexChangedDelegate* ATenkaichiPlayerState::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

// 队伍复制到客户端后触发：广播队伍变更。
void ATenkaichiPlayerState::OnRep_MyTeamID(FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

// 小队复制到客户端后触发。
void ATenkaichiPlayerState::OnRep_MySquadID()
{
	//@TODO: 通知小队子系统（等它存在以后）
}

// 给指定标签加上 N 层。
void ATenkaichiPlayerState::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

// 给指定标签减掉 N 层。
void ATenkaichiPlayerState::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

// 查询指定标签的层数。
int32 ATenkaichiPlayerState::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

// 判断指定标签是否至少叠了 1 层。
bool ATenkaichiPlayerState::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

// 给单个玩家发一条消息（客户端实现）。
void ATenkaichiPlayerState::ClientBroadcastMessage_Implementation(const FTenkaichiVerbMessage Message)
{
	// 这个检查用于防止在单机模式下执行该动作
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
	}
}