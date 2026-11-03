// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "InteractionOption.h"
#include "IInteractionInstigator.generated.h"

struct FInteractionQuery;

// UINTERFACE 外壳：仅作反射注册用，实际逻辑在下面的 IInteractionInstigator
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInteractionInstigator : public UInterface
{
	GENERATED_BODY()
};

// 交互发起者接口：多候选时由实现方挑出最合适的一个
class IInteractionInstigator
{
	GENERATED_BODY()

public:
	// 当收集到的交互选项多于一个时被调用，返回挑中的那一个
	virtual FInteractionOption ChooseBestInteractionOption(const FInteractionQuery& InteractQuery, const TArray<FInteractionOption>& InteractOptions) = 0;
};