// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "ModularAIController.h"
#include "Teams/TenkaichiTeamAgentInterface.h"

#include "TenkaichiPlayerBotController.generated.h"

namespace ETeamAttitude { enum Type : int; }
struct FGenericTeamId;

class APlayerState;
class UAIPerceptionComponent;
class UObject;
struct FFrame;

/**
 * ATenkaichiPlayerBotController
 *
 *	本工程机器人玩家使用的控制器类。
 */
UCLASS(Blueprintable)
class ATenkaichiPlayerBotController : public AModularAIController, public ITenkaichiTeamAgentInterface
{
	GENERATED_BODY()

public:
	ATenkaichiPlayerBotController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ~ITenkaichiTeamAgentInterface 队伍接口
	// 给机器人设置队伍（机器人禁止手动设，这里直接报错）。
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	// 查询机器人当前队伍 ID（从关联的 PlayerState 拿）。
	virtual FGenericTeamId GetGenericTeamId() const override;
	// 获取"队伍变更回调"的委托对象。
	virtual FOnTenkaichiTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	// 判定对方是敌/友/中立（AI 感知系统用）。
	ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	// ~ITenkaichiTeamAgentInterface 队伍接口结束

	// 尝试重启本控制器（例如用于重生机器人）。
	void ServerRestartController();

	// 更新 AI 感知系统对队伍的敌我判定。
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi AI Player Controller")
	void UpdateTeamAttitude(UAIPerceptionComponent* AIPerception);

	virtual void OnUnPossess() override;


private:
	// 当关联的 PlayerState 队伍改变时回调。
	UFUNCTION()
	void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

protected:
	// 当 PlayerState 被设置或清除时调用（留给子类扩展）。
	virtual void OnPlayerStateChanged();

private:
	// 广播 PlayerState 变更（解绑旧、绑定新、转发队伍变更）。
	void BroadcastOnPlayerStateChanged();

protected:
	// ~AController 接口
	virtual void InitPlayerState() override;
	virtual void CleanupPlayerState() override;
	virtual void OnRep_PlayerState() override;
	// ~AController 接口结束

private:
	// 本控制器的队伍变更委托（存储用）。
	UPROPERTY()
	FOnTenkaichiTeamIndexChangedDelegate OnTeamChangedDelegate;

	// 上一次见过的 PlayerState（用来解绑旧的状态）。
	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;
};