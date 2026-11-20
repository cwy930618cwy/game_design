// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractionStatics.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/OverlapResult.h"
#include "IInteractableTarget.h"
#include "UObject/ScriptInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionStatics)

UInteractionStatics::UInteractionStatics()
	: Super(FObjectInitializer::Get())
{
}

// 从「可交互目标」反推它归属的 Actor
AActor* UInteractionStatics::GetActorFromInteractableTarget(TScriptInterface<IInteractableTarget> InteractableTarget)
{
	// 拿到接口背后真正的 UObject（可能是个 Actor，也可能是个组件）
	if (UObject* Object = InteractableTarget.GetObject())
	{
		// 如果它本身就是个 Actor，直接返回
		if (AActor* Actor = Cast<AActor>(Object))
		{
			return Actor;
		}
		// 如果它是个组件，返回它所在的 Actor
		else if (UActorComponent* ActorComponent = Cast<UActorComponent>(Object))
		{
			return ActorComponent->GetOwner();
		}
		// 理论到不了这里（不该出现既不是 Actor 也不是组件的对象），兜底断言
		else
		{
			unimplemented();
		}
	}

	// 接口背后没有有效对象，返回空
	return nullptr;
}

// 取出一个 Actor 身上所有能交互的目标（Actor 自身 + 身上实现接口的组件）
void UInteractionStatics::GetInteractableTargetsFromActor(AActor* Actor, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets)
{
	// 第一步：如果这个 Actor 本身就能交互，直接收下
	TScriptInterface<IInteractableTarget> InteractableActor(Actor);
	if (InteractableActor)
	{
		OutInteractableTargets.Add(InteractableActor);
	}

	// 第二步：如果 Actor 不能交互，可能它身上有组件能交互，查出来逐个收下
	TArray<UActorComponent*> InteractableComponents = Actor ? Actor->GetComponentsByInterface(UInteractableTarget::StaticClass()) : TArray<UActorComponent*>();
	for (UActorComponent* InteractableComponent : InteractableComponents)
	{
		OutInteractableTargets.Add(TScriptInterface<IInteractableTarget>(InteractableComponent));
	}
}

// 从一组重叠结果里收集所有能交互的目标（追加进输出数组，不重复）
void UInteractionStatics::AppendInteractableTargetsFromOverlapResults(const TArray<FOverlapResult>& OverlapResults, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets)
{
	for (const FOverlapResult& Overlap : OverlapResults)
	{
		// 命中的 Actor 本身能交互就收下
		TScriptInterface<IInteractableTarget> InteractableActor(Overlap.GetActor());
		if (InteractableActor)
		{
			OutInteractableTargets.AddUnique(InteractableActor);
		}

		// 命中的组件能交互也收下
		TScriptInterface<IInteractableTarget> InteractableComponent(Overlap.GetComponent());
		if (InteractableComponent)
		{
			OutInteractableTargets.AddUnique(InteractableComponent);
		}
	}
}

// 从一次射线命中结果里收集所有能交互的目标（追加进输出数组，不重复）
void UInteractionStatics::AppendInteractableTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets)
{
	TScriptInterface<IInteractableTarget> InteractableActor(HitResult.GetActor());
	if (InteractableActor)
	{
		OutInteractableTargets.AddUnique(InteractableActor);
	}

	TScriptInterface<IInteractableTarget> InteractableComponent(HitResult.GetComponent());
	if (InteractableComponent)
	{
		OutInteractableTargets.AddUnique(InteractableComponent);
	}
}