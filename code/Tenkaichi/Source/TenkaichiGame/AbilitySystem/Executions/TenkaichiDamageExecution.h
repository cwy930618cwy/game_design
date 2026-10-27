// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "TenkaichiDamageExecution.generated.h"

class UObject;


/**
 * UTenkaichiDamageExecution
 *
 *	由游戏效果用来向生命属性施加伤害的执行计算。
 */
UCLASS()
class UTenkaichiDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	// 构造函数：往捕获列表里注册基础伤害属性（BaseDamage）
	UTenkaichiDamageExecution();

protected:

	// 执行计算的核心虚函数：算出伤害量并输出（具体算法在 .cpp）
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};