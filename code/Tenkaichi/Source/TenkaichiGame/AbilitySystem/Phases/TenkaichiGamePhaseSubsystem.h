// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"

#include "TenkaichiGamePhaseSubsystem.generated.h"

template <typename T> class TSubclassOf;

class UTenkaichiGamePhaseAbility;
class UObject;
struct FFrame;
struct FGameplayAbilitySpecHandle;

DECLARE_DYNAMIC_DELEGATE_OneParam(FTenkaichiGamePhaseDynamicDelegate, const UTenkaichiGamePhaseAbility*, Phase);
DECLARE_DELEGATE_OneParam(FTenkaichiGamePhaseDelegate, const UTenkaichiGamePhaseAbility* Phase);

DECLARE_DYNAMIC_DELEGATE_OneParam(FTenkaichiGamePhaseTagDynamicDelegate, const FGameplayTag&, PhaseTag);
DECLARE_DELEGATE_OneParam(FTenkaichiGamePhaseTagDelegate, const FGameplayTag& PhaseTag);

// 消息接收者的匹配规则
UENUM(BlueprintType)
enum class EPhaseTagMatchType : uint8
{
	// 精确匹配只接收通道完全相同的消息
	// （例如：注册 "A.B" 会匹配广播 A.B，但不匹配 A.B.C）
	ExactMatch,

	// 部分匹配会接收以同一通道为根的所有消息
	// （例如：注册 "A.B" 会匹配广播 A.B 以及 A.B.C）
	PartialMatch
};


/** 用 GameplayTag 以嵌套方式管理 Tenkaichi 游戏阶段的子系统，父子阶段可以同时活跃，但同级（sibling）阶段不行。
 * 例如：Game.Playing 和 Game.Playing.WarmUp 可以共存，但 Game.Playing 和 Game.ShowingScore 不行。
 * 当新阶段启动时，任何不是它祖先的活跃阶段都会被结束。
 * 例如：如果 Game.Playing 和 Game.Playing.CaptureTheFlag 处于活跃状态时启动了 Game.Playing.PostGame，
 *      那么 Game.Playing 会保持活跃，而 Game.Playing.CaptureTheFlag 会结束。
 */
UCLASS()
class UTenkaichiGamePhaseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 构造函数：无额外初始化
	UTenkaichiGamePhaseSubsystem();

	// 子系统初始化完成后调用
	virtual void PostInitialize() override;

	// 判断是否应该为给定的 Outer 创建本子系统
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// 启动指定阶段技能所代表的阶段，可传入阶段结束时的回调
	void StartPhase(TSubclassOf<UTenkaichiGamePhaseAbility> PhaseAbility, FTenkaichiGamePhaseDelegate PhaseEndedCallback = FTenkaichiGamePhaseDelegate());

	//TODO 应该返回一个句柄，方便使用者删除这些观察者。它们只会一直增长直到世界重置。
	//TODO 我们是否应该偶尔清理这些观察者？毕竟即使提供了句柄，也不是每个人都会正确地解绑。
	// 注册一个回调：当指定阶段开始（或已处于活跃）时触发
	void WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FTenkaichiGamePhaseTagDelegate& WhenPhaseActive);
	// 注册一个回调：当指定阶段结束时触发
	void WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FTenkaichiGamePhaseTagDelegate& WhenPhaseEnd);

	// 查询指定阶段当前是否处于活跃状态
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintPure = false, meta = (AutoCreateRefTerm = "PhaseTag"))
	bool IsPhaseActive(const FGameplayTag& PhaseTag) const;

protected:
	// 判断本子系统支持哪些世界类型
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	// 蓝图版启动阶段（把动态委托转换为原生委托后调用 StartPhase）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName="Start Phase", AutoCreateRefTerm = "PhaseEnded"))
	void K2_StartPhase(TSubclassOf<UTenkaichiGamePhaseAbility> Phase, const FTenkaichiGamePhaseDynamicDelegate& PhaseEnded);

	// 蓝图版注册阶段开始观察（DisplayName 用于在蓝图里显示为 "When Phase Starts or Is Active"）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName = "When Phase Starts or Is Active", AutoCreateRefTerm = "WhenPhaseActive"))
	void K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FTenkaichiGamePhaseTagDynamicDelegate WhenPhaseActive);

	// 蓝图版注册阶段结束观察
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName = "When Phase Ends", AutoCreateRefTerm = "WhenPhaseEnd"))
	void K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FTenkaichiGamePhaseTagDynamicDelegate WhenPhaseEnd);

	// 由阶段技能在激活时回调：处理新阶段开始与兄弟阶段的结束
	void OnBeginPhase(const UTenkaichiGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);
	// 由阶段技能在结束时回调：处理阶段结束与观察者通知
	void OnEndPhase(const UTenkaichiGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);

private:
	struct FTenkaichiGamePhaseEntry
	{
	public:
		FGameplayTag PhaseTag;
		FTenkaichiGamePhaseDelegate PhaseEndedCallback;
	};

	TMap<FGameplayAbilitySpecHandle, FTenkaichiGamePhaseEntry> ActivePhaseMap;

	struct FPhaseObserver
	{
	public:
		// 判断给定的阶段标签是否符合本观察者的匹配规则
		bool IsMatch(const FGameplayTag& ComparePhaseTag) const;
	
		FGameplayTag PhaseTag;
		EPhaseTagMatchType MatchType = EPhaseTagMatchType::ExactMatch;
		FTenkaichiGamePhaseTagDelegate PhaseCallback;
	};

	TArray<FPhaseObserver> PhaseStartObservers;
	TArray<FPhaseObserver> PhaseEndObservers;

	friend class UTenkaichiGamePhaseAbility;
};