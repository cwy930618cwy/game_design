// Copyright Epic Games, Inc. All Rights Reserved.

#if 0	// ⚠️ 挂起：依赖 UTenkaichiAbilitySystemComponent（GAS 断点），GAS 核心就位后放开

#include "Character/TenkaichiPawnExtensionComponent.h"

#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "TenkaichiGameplayTags.h"
#include "TenkaichiLogChannels.h"
#include "TenkaichiPawnData.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiPawnExtensionComponent)

class FLifetimeProperty;
class UActorComponent;

// 本特性的名字（整个模块的 Pawn 初始化特性的标识）
const FName UTenkaichiPawnExtensionComponent::NAME_ActorFeatureName("PawnExtension");

// 构造函数：关闭 Tick，默认开启复制，清空数据指针
UTenkaichiPawnExtensionComponent::UTenkaichiPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	PawnData = nullptr;
	AbilitySystemComponent = nullptr;
}

// 声明哪些属性需要网络复制：只有 PawnData 需要
void UTenkaichiPawnExtensionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTenkaichiPawnExtensionComponent, PawnData);
}

// 注册时：校验只能挂在 Pawn 上、只能有一个，然后注册到初始化状态系统
void UTenkaichiPawnExtensionComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("TenkaichiPawnExtensionComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> PawnExtensionComponents;
	Pawn->GetComponents(UTenkaichiPawnExtensionComponent::StaticClass(), PawnExtensionComponents);
	ensureAlwaysMsgf((PawnExtensionComponents.Num() == 1), TEXT("Only one TenkaichiPawnExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// 尽早注册到初始化状态系统（仅游戏世界才有效）
	RegisterInitStateFeature();
}

// BeginPlay：监听所有特性的状态变化，然后推进到 Spawned 状态
void UTenkaichiPawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听所有特性的状态变化
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);
	
	// 通知状态管理器"已生成"，然后继续默认初始化
	ensure(TryToChangeInitState(TenkaichiGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

// EndPlay：卸下能力系统 + 注销初始化状态，再调父类
void UTenkaichiPawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}
```// 设置角色数据：仅权威端能设，已有数据则报错，设完推进初始化
void UTenkaichiPawnExtensionComponent::SetPawnData(const UTenkaichiPawnData* InPawnData)
{
	check(InPawnData);

	APawn* Pawn = GetPawnChecked<APawn>();

	if (Pawn->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		UE_LOG(LogTenkaichi, Error, TEXT("Trying to set PawnData [%s] on pawn [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(Pawn), *GetNameSafe(PawnData));
		return;
	}

	PawnData = InPawnData;

	Pawn->ForceNetUpdate();

	CheckDefaultInitialization();
}

// PawnData 复制到客户端后触发：重新推进初始化
void UTenkaichiPawnExtensionComponent::OnRep_PawnData()
{
	CheckDefaultInitialization();
}

// 让本 Pawn 成为能力系统 avatar：初始化 ASC 的 Actor 信息并广播
void UTenkaichiPawnExtensionComponent::InitializeAbilitySystem(UTenkaichiAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	check(InASC);
	check(InOwnerActor);

	if (AbilitySystemComponent == InASC)
	{
		// 能力系统组件没变，直接返回
		return;
	}

	if (AbilitySystemComponent)
	{
		// 清理旧的能力系统组件
		UninitializeAbilitySystem();
	}

	APawn* Pawn = GetPawnChecked<APawn>();
	AActor* ExistingAvatar = InASC->GetAvatarActor();

	UE_LOG(LogTenkaichi, Verbose, TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

	if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
	{
		UE_LOG(LogTenkaichi, Log, TEXT("Existing avatar (authority=%d)"), ExistingAvatar->HasAuthority() ? 1 : 0);

		// 已经有一个 Pawn 是这个 ASC 的 avatar，得把它踢出去。
		// 这种情况会出现在客户端卡顿：新 Pawn 生成并拥有，而旧的还没被移除
		ensure(!ExistingAvatar->HasAuthority());

		if (UTenkaichiPawnExtensionComponent* OtherExtensionComponent = FindPawnExtensionComponent(ExistingAvatar))
		{
			OtherExtensionComponent->UninitializeAbilitySystem();
		}
	}

	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

	if (ensure(PawnData))
	{
		InASC->SetTagRelationshipMapping(PawnData->TagRelationshipMapping);
	}

	OnAbilitySystemInitialized.Broadcast();
}

// 卸下本 Pawn 作为能力系统 avatar 的身份（并取消相关技能/输入/表现）
void UTenkaichiPawnExtensionComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// 只有当我们还是 avatar actor 时才反初始化（否则别的 Pawn 已经在我们成为 avatar 时做过了）
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		FGameplayTagContainer AbilityTypesToIgnore;
		AbilityTypesToIgnore.AddTag(TenkaichiGameplayTags::Ability_Behavior_SurvivesDeath);

		AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
		AbilitySystemComponent->ClearAbilityInput();
		AbilitySystemComponent->RemoveAllGameplayCues();

		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		else
		{
			// 如果 ASC 没有有效 owner，则需要清掉*所有* Actor 信息，而不仅是 avatar 配对
			AbilitySystemComponent->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}

	AbilitySystemComponent = nullptr;
}
```// 控制器改变时：刷新能力系统信息，再推进初始化
void UTenkaichiPawnExtensionComponent::HandleControllerChanged()
{
	if (AbilitySystemComponent && (AbilitySystemComponent->GetAvatarActor() == GetPawnChecked<APawn>()))
	{
		ensure(AbilitySystemComponent->AbilityActorInfo->OwnerActor == AbilitySystemComponent->GetOwnerActor());
		if (AbilitySystemComponent->GetOwnerActor() == nullptr)
		{
			UninitializeAbilitySystem();
		}
		else
		{
			AbilitySystemComponent->RefreshAbilityActorInfo();
		}
	}

	CheckDefaultInitialization();
}

// PlayerState 复制到客户端后触发：重新推进初始化
void UTenkaichiPawnExtensionComponent::HandlePlayerStateReplicated()
{
	CheckDefaultInitialization();
}

// 输入组件设置好后触发：重新推进初始化
void UTenkaichiPawnExtensionComponent::SetupPlayerInputComponent()
{
	CheckDefaultInitialization();
}

// 推进默认初始化：先推进所依赖的其他特性，再沿状态链推进自己
void UTenkaichiPawnExtensionComponent::CheckDefaultInitialization()
{
	// 先推进可能依赖的其他特性
	CheckDefaultInitializationForImplementers();

	static const TArray<FGameplayTag> StateChain = { TenkaichiGameplayTags::InitState_Spawned, TenkaichiGameplayTags::InitState_DataAvailable, TenkaichiGameplayTags::InitState_DataInitialized, TenkaichiGameplayTags::InitState_GameplayReady };

	// 这会从 spawned（在 BeginPlay 里设置）开始推进，经过各数据初始化阶段直到 gameplay ready
	ContinueInitStateChain(StateChain);
}

// 状态机核心：判断能否从当前状态进入目标状态
bool UTenkaichiPawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();
	if (!CurrentState.IsValid() && DesiredState == TenkaichiGameplayTags::InitState_Spawned)
	{
		// 只要在合法 Pawn 上，就算作已生成
		if (Pawn)
		{
			return true;
		}
	}
	if (CurrentState == TenkaichiGameplayTags::InitState_Spawned && DesiredState == TenkaichiGameplayTags::InitState_DataAvailable)
	{
		// 需要角色数据
		if (!PawnData)
		{
			return false;
		}

		const bool bHasAuthority = Pawn->HasAuthority();
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();

		if (bHasAuthority || bIsLocallyControlled)
		{
			// 检查是否已被控制器拥有
			if (!GetController<AController>())
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == TenkaichiGameplayTags::InitState_DataAvailable && DesiredState == TenkaichiGameplayTags::InitState_DataInitialized)
	{
		// 所有特性都已"数据可用"才进入初始化
		return Manager->HaveAllFeaturesReachedInitState(Pawn, TenkaichiGameplayTags::InitState_DataAvailable);
	}
	else if (CurrentState == TenkaichiGameplayTags::InitState_DataInitialized && DesiredState == TenkaichiGameplayTags::InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

// 状态改变时的处理：目前 DataInitialized 阶段由其他监听该状态的组件处理
void UTenkaichiPawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (DesiredState == TenkaichiGameplayTags::InitState_DataInitialized)
	{
		// 目前都由其他组件监听此状态变化来处理
	}
}

// 别的特性状态改变时：若它到了 DataAvailable，看看我们是否要进入 DataInitialized
void UTenkaichiPawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// 不是本特性自己的变化才处理
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		if (Params.FeatureState == TenkaichiGameplayTags::InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

// 注册到"能力系统已初始化"：若已初始化则立即回调
void UTenkaichiPawnExtensionComponent::OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}

	if (AbilitySystemComponent)
	{
		Delegate.Execute();
	}
}

// 注册到"能力系统已反初始化"
void UTenkaichiPawnExtensionComponent::OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}

#endif	// ⚠️ GAS 核心（UTenkaichiAbilitySystemComponent）就位后放开