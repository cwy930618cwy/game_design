// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiPlayerBotController.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
// #if 0  // ⚠️ 注释挂起（依赖 P13）：本头文件由 P13 GameModes 阶段创建
// #include "GameModes/TenkaichiGameMode.h"
// #endif  // 注释挂起结束
#include "TenkaichiLogChannels.h"
#include "Perception/AIPerceptionComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiPlayerBotController)

class UObject;

ATenkaichiPlayerBotController::ATenkaichiPlayerBotController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 机器人也要挂 PlayerState（档案/队伍/小队数据）。
	bWantsPlayerState = true;
	// 控制器失配后不停止 AI 逻辑（这样机器人还能继续决策/重生）。
	bStopAILogicOnUnposses = false;
}

// 当关联的 PlayerState 队伍改变时：把新旧队伍转发成"队伍变更广播"。
void ATenkaichiPlayerBotController::OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
}

// 空实现，留给子类扩展（不用挂一堆事件）。
void ATenkaichiPlayerBotController::OnPlayerStateChanged()
{
	// 空实现：给派生类留扩展点。
}

// 核心：PlayerState 变了 → 解绑旧回调、绑定新回调、广播队伍变更。
void ATenkaichiPlayerBotController::BroadcastOnPlayerStateChanged()
{
	// 先让子类有机会处理"状态变更"这个事件。
	OnPlayerStateChanged();

	// 解绑旧 PlayerState 的队伍回调（如果有的话）。
	FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
	if (LastSeenPlayerState != nullptr)
	{
		if (ITenkaichiTeamAgentInterface* PlayerStateTeamInterface = Cast<ITenkaichiTeamAgentInterface>(LastSeenPlayerState))
		{
			// 记下旧队伍 ID，并解绑旧状态的回调。
			OldTeamID = PlayerStateTeamInterface->GetGenericTeamId();
			PlayerStateTeamInterface->GetTeamChangedDelegateChecked().RemoveAll(this);
		}
	}

	// 绑定新 PlayerState 的队伍回调（如果有的话）。
	FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	if (PlayerState != nullptr)
	{
		if (ITenkaichiTeamAgentInterface* PlayerStateTeamInterface = Cast<ITenkaichiTeamAgentInterface>(PlayerState))
		{
			// 记下新队伍 ID，并绑定新状态的回调（队伍变了会触发 OnPlayerStateChangedTeam）。
			NewTeamID = PlayerStateTeamInterface->GetGenericTeamId();
			PlayerStateTeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnPlayerStateChangedTeam);
		}
	}

	// 如果新旧队伍真的不一样，就广播出去。
	ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);

	// 记录本次的 PlayerState，下次解绑用。
	LastSeenPlayerState = PlayerState;
}

// 控制器挂上 PlayerState 时触发。
void ATenkaichiPlayerBotController::InitPlayerState()
{
	Super::InitPlayerState();
	BroadcastOnPlayerStateChanged();
}

// 控制器清除 PlayerState 时触发。
void ATenkaichiPlayerBotController::CleanupPlayerState()
{
	Super::CleanupPlayerState();
	BroadcastOnPlayerStateChanged();
}

// 客户端收到 PlayerState 复制时触发。
void ATenkaichiPlayerBotController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BroadcastOnPlayerStateChanged();
}

// 给机器人设置队伍：禁止！队伍由关联的 PlayerState 决定，这里直接报错。
void ATenkaichiPlayerBotController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	UE_LOG(LogTenkaichiTeams, Error, TEXT("You can't set the team ID on a player bot controller (%s); it's driven by the associated player state"), *GetPathNameSafe(this));
}

// 查询机器人当前队伍：从关联的 PlayerState 拿。
FGenericTeamId ATenkaichiPlayerBotController::GetGenericTeamId() const
{
	if (ITenkaichiTeamAgentInterface* PSWithTeamInterface = Cast<ITenkaichiTeamAgentInterface>(PlayerState))
	{
		return PSWithTeamInterface->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}

// 返回"队伍变更回调"委托的地址。
FOnTenkaichiTeamIndexChangedDelegate* ATenkaichiPlayerBotController::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

// 服务器端重启控制器（用于机器人重生）。依赖 P13 GameMode，暂时挂起。
// #if 0  // ⚠️ 注释挂起（依赖 P13）：GameModes/TenkaichiGameMode.h 未建
void ATenkaichiPlayerBotController::ServerRestartController()
{
	// 客户端不执行，只有服务器（权威）才能重生。
	if (GetNetMode() == NM_Client)
	{
		return;
	}

	// 断言：此时必须没有 Pawn，且处于"空闲"状态。
	ensure((GetPawn() == nullptr) && IsInState(NAME_Inactive));

	if (IsInState(NAME_Inactive) || (IsInState(NAME_Spectating)))
	{
		// 拿到本关卡的 GameMode 来执行重生。
		ATenkaichiGameMode* const GameMode = GetWorld()->GetAuthGameMode<ATenkaichiGameMode>();

		// 如果拿不到 GameMode，或它不允许本控制器重生，就返回。
		if ((GameMode == nullptr) || !GameMode->ControllerCanRestart(this))
		{
			return;
		}

		// 如果还挂着 Pawn，先释放。
		if (GetPawn() != nullptr)
		{
			UnPossess();
		}

		// 重新启用输入（和 ClientRestart 类似）。
		ResetIgnoreInputFlags();

		// 让 GameMode 帮我们重启这个玩家。
		GameMode->RestartPlayer(this);
	}
}
// #endif  // 注释挂起结束

// 判定对方是敌/友/中立（AI 感知系统用）。
ETeamAttitude::Type ATenkaichiPlayerBotController::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const APawn* OtherPawn = Cast<APawn>(&Other)) {

		if (const ITenkaichiTeamAgentInterface* TeamAgent = Cast<ITenkaichiTeamAgentInterface>(OtherPawn->GetController()))
		{
			FGenericTeamId OtherTeamID = TeamAgent->GetGenericTeamId();

			// 对比对方的队伍 ID 和我的队伍 ID。
			if (OtherTeamID.GetId() != GetGenericTeamId().GetId())
			{
				return ETeamAttitude::Hostile; // 不同队 = 敌对
			}
			else
			{
				return ETeamAttitude::Friendly; // 同队 = 友好
			}
		}
	}

	return ETeamAttitude::Neutral; // 无法判定 = 中立
}

// 更新 AI 感知系统的敌我判定（告诉感知系统"队伍关系变了，重新算"）。
void ATenkaichiPlayerBotController::UpdateTeamAttitude(UAIPerceptionComponent* AIPerception)
{
	if (AIPerception)
	{
		AIPerception->RequestStimuliListenerUpdate();
	}
}

// 失配时：确保被失配的 Pawn 不再作为 ASC 的头像（防止技能系统引用悬空）。
void ATenkaichiPlayerBotController::OnUnPossess()
{
	// 确保被失配的 Pawn 不再作为我们 ASC 的头像 Actor。
	if (APawn* PawnBeingUnpossessed = GetPawn())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState))
		{
			if (ASC->GetAvatarActor() == PawnBeingUnpossessed)
			{
				ASC->SetAvatarActor(nullptr);
			}
		}
	}

	Super::OnUnPossess();
}