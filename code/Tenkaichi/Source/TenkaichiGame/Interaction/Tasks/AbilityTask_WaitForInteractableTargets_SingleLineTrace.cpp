// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilityTask_WaitForInteractableTargets_SingleLineTrace.h"
#include "Interaction/InteractionStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_WaitForInteractableTargets_SingleLineTrace)

UAbilityTask_WaitForInteractableTargets_SingleLineTrace::UAbilityTask_WaitForInteractableTargets_SingleLineTrace(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 静态工厂：创建任务实例并保存所有参数
UAbilityTask_WaitForInteractableTargets_SingleLineTrace* UAbilityTask_WaitForInteractableTargets_SingleLineTrace::WaitForInteractableTargets_SingleLineTrace(UGameplayAbility* OwningAbility, FInteractionQuery InteractionQuery, FCollisionProfileName TraceProfile, FGameplayAbilityTargetingLocationInfo StartLocation, float InteractionScanRange, float InteractionScanRate, bool bShowDebug)
{
	UAbilityTask_WaitForInteractableTargets_SingleLineTrace* MyObj = NewAbilityTask<UAbilityTask_WaitForInteractableTargets_SingleLineTrace>(OwningAbility);
	MyObj->InteractionScanRange = InteractionScanRange;
	MyObj->InteractionScanRate = InteractionScanRate;
	MyObj->StartLocation = StartLocation;
	MyObj->InteractionQuery = InteractionQuery;
	MyObj->TraceProfile = TraceProfile;
	MyObj->bShowDebug = bShowDebug;

	return MyObj;
}

// 任务激活：启动定时器，每隔 InteractionScanRate 秒执行一次 PerformTrace
void UAbilityTask_WaitForInteractableTargets_SingleLineTrace::Activate()
{
	SetWaitingOnAvatar();

	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::PerformTrace, InteractionScanRate, true);
}

// 任务销毁：停止定时器
void UAbilityTask_WaitForInteractableTargets_SingleLineTrace::OnDestroy(bool AbilityEnded)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}

	Super::OnDestroy(AbilityEnded);
}

// 核心：每次执行一次射线检测，找出可交互目标并交给基类处理
void UAbilityTask_WaitForInteractableTargets_SingleLineTrace::PerformTrace()
{
	AActor* AvatarActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (!AvatarActor)
	{
		return;
	}

	UWorld* World = GetWorld();

	// 射线要忽略的 Actor 列表（先把自己排除，避免扫到自己）
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AvatarActor);

	const bool bTraceComplex = false;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(UAbilityTask_WaitForInteractableTargets_SingleLineTrace), bTraceComplex);
	Params.AddIgnoredActors(ActorsToIgnore);

	// 起点：能力目标定位信息里的位置
	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FVector TraceEnd;

	// 终点：沿玩家相机视线方向、在 InteractionScanRange 范围内的瞄准点
	AimWithPlayerController(AvatarActor, Params, TraceStart, InteractionScanRange, OUT TraceEnd);

	// 打一条射线，拿到最近命中点
	FHitResult OutHitResult;
	LineTrace(OutHitResult, World, TraceStart, TraceEnd, TraceProfile.Name, Params);

	// 从命中点解析出可交互目标集合
	TArray<TScriptInterface<IInteractableTarget>> InteractableTargets;
	UInteractionStatics::AppendInteractableTargetsFromHitResult(OutHitResult, InteractableTargets);

	// 交给基类：收集、过滤、广播新的可交互目标
	UpdateInteractableOptions(InteractionQuery, InteractableTargets);

#if ENABLE_DRAW_DEBUG
	// 调试模式：画出射线和命中点
	if (bShowDebug)
	{
		FColor DebugColor = OutHitResult.bBlockingHit ? FColor::Red : FColor::Green;
		if (OutHitResult.bBlockingHit)
		{
			DrawDebugLine(World, TraceStart, OutHitResult.Location, DebugColor, false, InteractionScanRate);
			DrawDebugSphere(World, OutHitResult.Location, 5, 16, DebugColor, false, InteractionScanRate);
		}
		else
		{
			DrawDebugLine(World, TraceStart, TraceEnd, DebugColor, false, InteractionScanRate);
		}
	}
#endif // ENABLE_DRAW_DEBUG
}