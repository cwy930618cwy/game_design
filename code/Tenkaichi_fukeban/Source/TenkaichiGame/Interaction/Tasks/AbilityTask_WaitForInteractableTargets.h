// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "Engine/CollisionProfile.h"
#include "Interaction/InteractionOption.h"

#include "AbilityTask_WaitForInteractableTargets.generated.h"

class AActor;
class IInteractableTarget;
class UObject;
class UWorld;
struct FCollisionQueryParams;
struct FHitResult;
struct FInteractionQuery;
template <typename InterfaceType> class TScriptInterface;

// 委托：当「可交互选项列表」变化时广播（参数是新列表）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableObjectsChangedEvent, const TArray<FInteractionOption>&, InteractableOptions);

// 抽象基类：等待玩家当前可交互目标，变化时广播给上层（UI 等）
UCLASS(Abstract)
class UAbilityTask_WaitForInteractableTargets : public UAbilityTask
{
	GENERATED_UCLASS_BODY()

public:
	// 蓝图可绑定的委托：可交互选项列表变化时触发
	UPROPERTY(BlueprintAssignable)
	FInteractableObjectsChangedEvent InteractableObjectsChanged;

protected:

	// 做一次射线检测，取最近命中点（供子类调用）
	static void LineTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params);

	// 从玩家视角计算射线终点（考虑瞄准方向、最大射程，可选忽略俯仰角）
	void AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd, bool bIgnorePitch = false) const;

	// 把相机射线裁剪到技能作用半径范围内（相机朝外射时保证终点落在能力范围内）
	static bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition);

	// 核心：把「可交互目标列表」变成「选项列表」、过滤不可用项、并广播变化
	void UpdateInteractableOptions(const FInteractionQuery& InteractQuery, const TArray<TScriptInterface<IInteractableTarget>>& InteractableTargets);

	// 射线检测用的碰撞配置名（子类传入）
	FCollisionProfileName TraceProfile;

	// 射线是否影响瞄准俯仰角（默认 true）
	bool bTraceAffectsAimPitch = true;

	// 当前生效的选项列表（用于对比是否有变化）
	TArray<FInteractionOption> CurrentOptions;
};