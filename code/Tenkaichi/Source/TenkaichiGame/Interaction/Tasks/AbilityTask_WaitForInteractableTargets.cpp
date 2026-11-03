// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/Tasks/AbilityTask_WaitForInteractableTargets.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/IInteractableTarget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_WaitForInteractableTargets)

struct FInteractionQuery;

UAbilityTask_WaitForInteractableTargets::UAbilityTask_WaitForInteractableTargets(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 做一次射线检测，取最近的命中点（静态工具，供子类复用）
void UAbilityTask_WaitForInteractableTargets::LineTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params)
{
	check(World);

	OutHitResult = FHitResult();
	TArray<FHitResult> HitResults;
	World->LineTraceMultiByProfile(HitResults, Start, End, ProfileName, Params);

	OutHitResult.TraceStart = Start;
	OutHitResult.TraceEnd = End;

	if (HitResults.Num() > 0)
	{
		OutHitResult = HitResults[0];
	}
}

// 从玩家视角计算瞄准射线的终点（考虑相机朝向、最大射程，可选忽略俯仰角）
void UAbilityTask_WaitForInteractableTargets::AimAtPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd, bool bIgnorePitch) const
{
	if (!Ability) // 只在服务器和发起客户端执行
	{
		return;
	}

	//@TODO: Bots?
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	check(PC);

	FVector ViewStart;
	FRotator ViewRot;
	PC->GetPlayerViewPoint(ViewStart, ViewRot);

	const FVector ViewDir = ViewRot.Vector();
	FVector ViewEnd = ViewStart + (ViewDir * MaxRange);

	// 把相机射线裁剪到能力作用半径内
	ClipCameraToAbilityRange(ViewStart, ViewDir, Start, MaxRange, ViewEnd);

	FHitResult HitResult;
	LineTrace(HitResult, InSourceActor->GetWorld(), ViewStart, ViewEnd, TraceProfile.Name, Params);

	// 若射线打中障碍物且在最大射程内，就用命中点作为瞄准终点
	const bool bUseResult = HitResult.bBlockingHit && (FVector::DistSquared(Start, HitResult.Location) <= (MaxRange * MaxRange));
	const FVector AdjustedEnd = bUseResult ? HitResult.Location : ViewEnd;

	FVector AdjustedDir = (AdjustedEnd - Start).GetSafeNormal();
	if (AdjustedDir.IsZero())
	{
		AdjustedDir = ViewDir;
	}

	// 忽略俯仰角时：保留原视线俯仰，只改水平方向
	if (!bIgnorePitch && !bUseResult)
	{
		FVector OriginalDir = (ViewEnd - Start).GetSafeNormal();
		if (!OriginalDir.IsZero())
		{
			const FRotator OriginalRot = OriginalDir.Rotation();
			FRotator AdjustedRot = AdjustedDir.Rotation();
			AdjustedRot.Pitch = OriginalRot.Pitch;
			AdjustedDir = AdjustedRot.Vector();
		}
	}

	OutTarget = Start + (AdjustedDir * MaxRange);
}