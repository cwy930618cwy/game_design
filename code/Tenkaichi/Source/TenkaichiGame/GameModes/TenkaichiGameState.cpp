// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiGameState.h"

#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "Async/TaskGraphInterfaces.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameModes/TenkaichiExperienceManagerComponent.h"
#include "Messages/TenkaichiVerbMessage.h"
#include "Player/TenkaichiPlayerState.h"
#include "TenkaichiLogChannels.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameState)

class APlayerState;
class FLifetimeProperty;

// 引擎提供的"平均 FPS"全局变量（在 Engine 模块里）。
extern ENGINE_API float GAverageFPS;


// 构造函数：创建 AbilitySystemComponent + ExperienceManagerComponent 子对象。
ATenkaichiGameState::ATenkaichiGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 允许每帧 Tick，并默认开启
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// 创建全局能力系统组件（游戏全局事物用，主要是 gameplay cue）
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UTenkaichiAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// 创建经验管理器组件（负责加载当前经验）
	ExperienceManagerComponent = CreateDefaultSubobject<UTenkaichiExperienceManagerComponent>(TEXT("ExperienceManagerComponent"));

	ServerFPS = 0.0f;
}

// 组件预初始化（先调父类）。
void ATenkaichiGameState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

// 组件后初始化（初始化全局 ASC 的 actor 信息）。
void ATenkaichiGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(/*Owner=*/ this, /*Avatar=*/ this);
}

// 返回这个 GameState 的全局能力系统组件。
UAbilitySystemComponent* ATenkaichiGameState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// Actor 生命周期结束（先调父类）。
void ATenkaichiGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// 添加玩家状态（先调父类）。
void ATenkaichiGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
}

// 移除玩家状态（先调父类）。
void ATenkaichiGameState::RemovePlayerState(APlayerState* PlayerState)
{
	//@TODO: 现在这个不会被调用（只有"富"的 AGameMode 用它，AGameModeBase 不用）
	// 至少需要注释引擎代码，可能还要挪动一些东西
	Super::RemovePlayerState(PlayerState);
}

// 无缝传送过渡检查点（移除 Bot 和非活跃玩家）。
void ATenkaichiGameState::SeamlessTravelTransitionCheckpoint(bool bToTransitionMap)
{
	// 移除非活跃玩家和 Bot
	for (int32 i = PlayerArray.Num() - 1; i >= 0; i--)
	{
		APlayerState* PlayerState = PlayerArray[i];
		if (PlayerState && (PlayerState->IsABot() || PlayerState->IsInactive()))
		{
			RemovePlayerState(PlayerState);
		}
	}
}

// 登记网络复制属性（ServerFPS + RecorderPlayerState）。
void ATenkaichiGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ServerFPS);
	DOREPLIFETIME_CONDITION(ThisClass, RecorderPlayerState, COND_ReplayOnly);
}

// 每帧更新（服务器权威时更新 ServerFPS）。
void ATenkaichiGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetLocalRole() == ROLE_Authority)
	{
		ServerFPS = GAverageFPS;
	}
}

// 发送一条所有客户端（大概率）都会收到的消息（不可靠）。
void ATenkaichiGameState::MulticastMessageToClients_Implementation(const FTenkaichiVerbMessage Message)
{
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
	}
}

// 发送一条保证所有客户端都会收到的消息（可靠）。
void ATenkaichiGameState::MulticastReliableMessageToClients_Implementation(const FTenkaichiVerbMessage Message)
{
	MulticastMessageToClients_Implementation(Message);
}

// 获取服务器的 FPS。
float ATenkaichiGameState::GetServerFPS() const
{
	return ServerFPS;
}

// 标记本地玩家状态正在录制回放。
void ATenkaichiGameState::SetRecorderPlayerState(APlayerState* NewPlayerState)
{
	if (RecorderPlayerState == nullptr)
	{
		// 设置它，并调用复制回调，让它可以做任何录制时的初始化
		RecorderPlayerState = NewPlayerState;
		OnRep_RecorderPlayerState();
	}
	else
	{
		UE_LOG(LogTenkaichi, Warning, TEXT("SetRecorderPlayerState was called on %s but should only be called once per game on the primary user"), *GetName());
	}
}

// 获取录制回放的玩家状态。
APlayerState* ATenkaichiGameState::GetRecorderPlayerState() const
{
	// TODO: 为空时也许自动选一个？

	return RecorderPlayerState;
}

// 回放玩家状态复制回调。
void ATenkaichiGameState::OnRep_RecorderPlayerState()
{
	OnRecorderPlayerStateChangedEvent.Broadcast(RecorderPlayerState);
}