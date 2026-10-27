// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Phases/TenkaichiGamePhaseSubsystem.h"

#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "TenkaichiGamePhaseAbility.h"
#include "TenkaichiGamePhaseLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGamePhaseSubsystem)

class UTenkaichiGameplayAbility;
class UObject;

DEFINE_LOG_CATEGORY(LogTenkaichiGamePhase);

//////////////////////////////////////////////////////////////////////
// UTenkaichiGamePhaseSubsystem

// 构造函数：无额外初始化
UTenkaichiGamePhaseSubsystem::UTenkaichiGamePhaseSubsystem()
{
}

// 子系统初始化完成后调用：仅转发给父类
void UTenkaichiGamePhaseSubsystem::PostInitialize()
{
	Super::PostInitialize();
}

// 判断是否为给定 Outer 创建本子系统：始终创建
bool UTenkaichiGamePhaseSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (Super::ShouldCreateSubsystem(Outer))
	{
		//UWorld* World = Cast<UWorld>(Outer);
		//check(World);

		//return World->GetAuthGameMode() != nullptr;
		//return nullptr;
		return true;
	}

	return false;
}

// 判断支持哪些世界类型：仅游戏世界与 PIE（编辑器内运行）
bool UTenkaichiGamePhaseSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

// 启动指定阶段：给 GameState 的 ASC 授予并激活该阶段技能，记录到活跃阶段表
void UTenkaichiGamePhaseSubsystem::StartPhase(TSubclassOf<UTenkaichiGamePhaseAbility> PhaseAbility, FTenkaichiGamePhaseDelegate PhaseEndedCallback)
{
	UWorld* World = GetWorld();
	UTenkaichiAbilitySystemComponent* GameState_ASC = World->GetGameState()->FindComponentByClass<UTenkaichiAbilitySystemComponent>();
	if (ensure(GameState_ASC))
	{
		FGameplayAbilitySpec PhaseSpec(PhaseAbility, 1, 0, this);
		FGameplayAbilitySpecHandle SpecHandle = GameState_ASC->GiveAbilityAndActivateOnce(PhaseSpec);
		FGameplayAbilitySpec* FoundSpec = GameState_ASC->FindAbilitySpecFromHandle(SpecHandle);
		
		if (FoundSpec && FoundSpec->IsActive())
		{
			FTenkaichiGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(SpecHandle);
			Entry.PhaseEndedCallback = PhaseEndedCallback;
		}
		else
		{
			PhaseEndedCallback.ExecuteIfBound(nullptr);
		}
	}
}

// 蓝图版启动阶段：把动态委托转为原生委托后调用 StartPhase
void UTenkaichiGamePhaseSubsystem::K2_StartPhase(TSubclassOf<UTenkaichiGamePhaseAbility> PhaseAbility, const FTenkaichiGamePhaseDynamicDelegate& PhaseEndedDelegate)
{
	const FTenkaichiGamePhaseDelegate EndedDelegate = FTenkaichiGamePhaseDelegate::CreateWeakLambda(const_cast<UObject*>(PhaseEndedDelegate.GetUObject()), [PhaseEndedDelegate](const UTenkaichiGamePhaseAbility* PhaseAbility) {
		PhaseEndedDelegate.ExecuteIfBound(PhaseAbility);
	});

	StartPhase(PhaseAbility, EndedDelegate);
}

// 蓝图版注册阶段开始观察：把动态委托转为原生委托后调用原生版
void UTenkaichiGamePhaseSubsystem::K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FTenkaichiGamePhaseTagDynamicDelegate WhenPhaseActive)
{
	const FTenkaichiGamePhaseTagDelegate ActiveDelegate = FTenkaichiGamePhaseTagDelegate::CreateWeakLambda(WhenPhaseActive.GetUObject(), [WhenPhaseActive](const FGameplayTag& PhaseTag) {
		WhenPhaseActive.ExecuteIfBound(PhaseTag);
	});

	WhenPhaseStartsOrIsActive(PhaseTag, MatchType, ActiveDelegate);
}

// 蓝图版注册阶段结束观察：把动态委托转为原生委托后调用原生版
void UTenkaichiGamePhaseSubsystem::K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FTenkaichiGamePhaseTagDynamicDelegate WhenPhaseEnd)
{
	const FTenkaichiGamePhaseTagDelegate EndedDelegate = FTenkaichiGamePhaseTagDelegate::CreateWeakLambda(WhenPhaseEnd.GetUObject(), [WhenPhaseEnd](const FGameplayTag& PhaseTag) {
		WhenPhaseEnd.ExecuteIfBound(PhaseTag);
	});

	WhenPhaseEnds(PhaseTag, MatchType, EndedDelegate);
}

// 注册阶段开始观察：记录观察者，若该阶段已活跃则立即触发一次
void UTenkaichiGamePhaseSubsystem::WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FTenkaichiGamePhaseTagDelegate& WhenPhaseActive)
{
	FPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseActive;
	PhaseStartObservers.Add(Observer);

	if (IsPhaseActive(PhaseTag))
	{
		WhenPhaseActive.ExecuteIfBound(PhaseTag);
	}
}

// 注册阶段结束观察：记录观察者（不立即触发）
void UTenkaichiGamePhaseSubsystem::WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FTenkaichiGamePhaseTagDelegate& WhenPhaseEnd)
{
	FPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseEnd;
	PhaseEndObservers.Add(Observer);
}

// 查询指定阶段是否活跃：遍历活跃阶段表做标签匹配
bool UTenkaichiGamePhaseSubsystem::IsPhaseActive(const FGameplayTag& PhaseTag) const
{
	for (const auto& KVP : ActivePhaseMap)
	{
		const FTenkaichiGamePhaseEntry& PhaseEntry = KVP.Value;
		if (PhaseEntry.PhaseTag.MatchesTag(PhaseTag))
		{
			return true;
		}
	}

	return false;
}

// 新阶段开始：结束所有非祖先的活跃阶段，登记本阶段，并通知开始观察者
void UTenkaichiGamePhaseSubsystem::OnBeginPhase(const UTenkaichiGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const FGameplayTag IncomingPhaseTag = PhaseAbility->GetGamePhaseTag();

	UE_LOG(LogTenkaichiGamePhase, Log, TEXT("Beginning Phase '%s' (%s)"), *IncomingPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	const UWorld* World = GetWorld();
	UTenkaichiAbilitySystemComponent* GameState_ASC = World->GetGameState()->FindComponentByClass<UTenkaichiAbilitySystemComponent>();
	if (ensure(GameState_ASC))
	{
		TArray<FGameplayAbilitySpec*> ActivePhases;
		for (const auto& KVP : ActivePhaseMap)
		{
			const FGameplayAbilitySpecHandle ActiveAbilityHandle = KVP.Key;
			if (FGameplayAbilitySpec* Spec = GameState_ASC->FindAbilitySpecFromHandle(ActiveAbilityHandle))
			{
				ActivePhases.Add(Spec);
			}
		}

		for (const FGameplayAbilitySpec* ActivePhase : ActivePhases)
		{
			const UTenkaichiGamePhaseAbility* ActivePhaseAbility = CastChecked<UTenkaichiGamePhaseAbility>(ActivePhase->Ability);
			const FGameplayTag ActivePhaseTag = ActivePhaseAbility->GetGamePhaseTag();
			
			// 如果当前活跃阶段与新阶段标签匹配（即它是新阶段的祖先），就允许它继续存在。
			// 也就是说，多个技能可以关联到同一个阶段标签。
			// 举例：
			// 你可以处于 Game.Playing 阶段，然后启动一个子阶段，比如 Game.Playing.SuddenDeath。
			// Game.Playing 阶段仍然活跃，如果再启动一个，比如 Game.Playing.ActualSuddenDeath，
			// 它会结束 Game.Playing.SuddenDeath 阶段，但 Game.Playing 会继续存在。
			// 类似地，如果我们激活 Game.GameOver，所有 Game.Playing* 阶段都会结束。
			if (!IncomingPhaseTag.MatchesTag(ActivePhaseTag))
			{
				UE_LOG(LogTenkaichiGamePhase, Log, TEXT("\tEnding Phase '%s' (%s)"), *ActivePhaseTag.ToString(), *GetNameSafe(ActivePhaseAbility));

				FGameplayAbilitySpecHandle HandleToEnd = ActivePhase->Handle;
				GameState_ASC->CancelAbilitiesByFunc([HandleToEnd](const UTenkaichiGameplayAbility* TenkaichiAbility, FGameplayAbilitySpecHandle Handle) {
					return Handle == HandleToEnd;
				}, true);
			}
		}

		FTenkaichiGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(PhaseAbilityHandle);
		Entry.PhaseTag = IncomingPhaseTag;

		// 通知所有观察本阶段的观察者：阶段已开始
		for (const FPhaseObserver& Observer : PhaseStartObservers)
		{
			if (Observer.IsMatch(IncomingPhaseTag))
			{
				Observer.PhaseCallback.ExecuteIfBound(IncomingPhaseTag);
			}
		}
	}
}

// 阶段结束：触发结束回调、从活跃表移除，并通知结束观察者
void UTenkaichiGamePhaseSubsystem::OnEndPhase(const UTenkaichiGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const FGameplayTag EndedPhaseTag = PhaseAbility->GetGamePhaseTag();
	UE_LOG(LogTenkaichiGamePhase, Log, TEXT("Ended Phase '%s' (%s)"), *EndedPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	const FTenkaichiGamePhaseEntry& Entry = ActivePhaseMap.FindChecked(PhaseAbilityHandle);
	Entry.PhaseEndedCallback.ExecuteIfBound(PhaseAbility);

	ActivePhaseMap.Remove(PhaseAbilityHandle);

	// 通知所有观察本阶段的观察者：阶段已结束
	for (const FPhaseObserver& Observer : PhaseEndObservers)
	{
		if (Observer.IsMatch(EndedPhaseTag))
		{
			Observer.PhaseCallback.ExecuteIfBound(EndedPhaseTag);
		}
	}
}

// 判断给定阶段标签是否符合本观察者的匹配规则（精确匹配 / 部分匹配）
bool UTenkaichiGamePhaseSubsystem::FPhaseObserver::IsMatch(const FGameplayTag& ComparePhaseTag) const
{
	switch(MatchType)
	{
	case EPhaseTagMatchType::ExactMatch:
		return ComparePhaseTag == PhaseTag;
	case EPhaseTagMatchType::PartialMatch:
		return ComparePhaseTag.MatchesTag(PhaseTag);
	}

	return false;
}