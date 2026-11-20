// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayAbilityTargetActor_Interact.h"
#include "Async/TaskGraphInterfaces.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "GameFramework/LightWeightInstanceSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayAbilityTargetActor_Interact)

AGameplayAbilityTargetActor_Interact::AGameplayAbilityTargetActor_Interact(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 核心：从能力起点沿玩家相机视线打一条射线，取命中结果并更新瞄准准星
FHitResult AGameplayAbilityTargetActor_Interact::PerformTrace(AActor* InSourceActor)
{
	bool bTraceComplex = false;
	TArray<AActor*> ActorsToIgnore;

	ActorsToIgnore.Add(InSourceActor);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AGameplayAbilityTargetActor_SingleLineTrace), bTraceComplex);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);

	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();// InSourceActor->GetActorLocation();
	FVector TraceEnd;
	AimWithPlayerController(InSourceActor, Params, TraceStart, TraceEnd);		// 只在服务器和发起客户端生效

	// ------------------------------------------------------

	FHitResult ReturnHitResult;
	LineTraceWithFilter(ReturnHitResult, InSourceActor->GetWorld(), Filter, TraceStart, TraceEnd, TraceProfile.Name, Params);
	// 没打中任何东西时，默认用射线终点
	if (!ReturnHitResult.bBlockingHit)
	{
		ReturnHitResult.Location = TraceEnd;
	}

	// 若存在准星 Actor，把它移动到命中位置
	if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActor.Get())
	{
		// 是否打中了「真正的目标 Actor」（而非地面/墙）
		const bool bHitActor = (ReturnHitResult.bBlockingHit && (ReturnHitResult.HitObjectHandle.IsValid()));
		// 若命中目标 Actor 且准星要吸附到目标，用目标位置；否则用命中点位置
		const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor) ? FLightWeightInstanceSubsystem::Get().GetLocation(ReturnHitResult.HitObjectHandle) : ReturnHitResult.Location;

		LocalReticleActor->SetActorLocation(ReticleLocation);
		LocalReticleActor->SetIsTargetAnActor(bHitActor);
	}

#if ENABLE_DRAW_DEBUG
	if (bDebug)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green);
		DrawDebugSphere(GetWorld(), TraceEnd, 100.0f, 16, FColor::Green);
	}
#endif // ENABLE_DRAW_DEBUG
	return ReturnHitResult;
}