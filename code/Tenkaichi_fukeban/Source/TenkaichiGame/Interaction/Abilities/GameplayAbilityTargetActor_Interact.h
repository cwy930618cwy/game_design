// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbilityTargetActor_Trace.h"

#include "GameplayAbilityTargetActor_Interact.generated.h"

class AActor;
class UObject;


/** 所有交互目标 Actor 的中间基类。 */
UCLASS(Blueprintable)
class AGameplayAbilityTargetActor_Interact : public AGameplayAbilityTargetActor_Trace
{
	GENERATED_BODY()

public:
	// 构造函数（带默认的 ObjectInitializer，允许无参构造）
	AGameplayAbilityTargetActor_Interact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 重写基类的射线追踪方法，把交互特有的扫描逻辑嵌进去
	virtual FHitResult PerformTrace(AActor* InSourceActor) override;

protected:
};