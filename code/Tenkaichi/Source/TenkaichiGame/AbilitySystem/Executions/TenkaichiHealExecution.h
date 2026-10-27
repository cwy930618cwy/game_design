// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "TenkaichiHealExecution.generated.h"

class UObject;


/**
 * UTenkaichiHealExecution
 *
 *	由游戏效果用来向生命属性施加治疗的执行计算。
 */
UCLASS()
class UTenkaichiHealExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	// 构造函数：往捕获列表里注册基础治疗属性（BaseHeal）
	UTenkaichiHealExecution();

protected:

	// 执行计算的核心虚函数：算出治疗量并输出（具体算法在 .cpp）
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};