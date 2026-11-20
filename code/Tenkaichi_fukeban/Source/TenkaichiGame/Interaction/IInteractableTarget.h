// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "InteractionOption.h"
#include "IInteractableTarget.generated.h"

struct FInteractionQuery;

// 交互选项收集器：帮候选者把「能提供哪些交互」写进系统给的收集表里
class FInteractionOptionBuilder
{
public:
	FInteractionOptionBuilder(TScriptInterface<IInteractableTarget> InterfaceTargetScope, TArray<FInteractionOption>& InteractOptions)
		: Scope(InterfaceTargetScope)
		, Options(InteractOptions)
	{
	}

	void AddInteractionOption(const FInteractionOption& Option)
	{
		FInteractionOption& OptionEntry = Options.Add_GetRef(Option);
		OptionEntry.InteractableTarget = Scope;
	}

private:
	TScriptInterface<IInteractableTarget> Scope;
	TArray<FInteractionOption>& Options;
};

// UINTERFACE 外壳：仅作反射注册用，实际逻辑在下面的 IInteractableTarget
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInteractableTarget : public UInterface
{
	GENERATED_BODY()
};

// 可交互目标接口：规定候选者要实现的两个虚方法
class IInteractableTarget
{
	GENERATED_BODY()

public:
	// 收集本候选者能提供的所有交互选项（纯虚，必须实现）
	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder) = 0;

	// 真正触发交互前，往事件数据里塞额外信息（可选覆盖，默认空实现）
	virtual void CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData) { }
};