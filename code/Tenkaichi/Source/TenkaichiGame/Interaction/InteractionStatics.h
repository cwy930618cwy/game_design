// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "InteractionStatics.generated.h"

template <typename InterfaceType> class TScriptInterface;

class AActor;
class IInteractableTarget;
class UObject;
struct FFrame;
struct FHitResult;
struct FOverlapResult;

// 交互静态工具类：把 Actor / 命中 / 重叠结果翻译成一批 IInteractableTarget 候选
UCLASS()
class UInteractionStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UInteractionStatics();

public:
	// 从「可交互目标」反推它归属的 Actor（对象本身，或它所在的组件所属的 Actor）
	UFUNCTION(BlueprintCallable)
	static AActor* GetActorFromInteractableTarget(TScriptInterface<IInteractableTarget> InteractableTarget);

	// 取出一个 Actor 身上所有能交互的目标（Actor 自身 + 身上实现接口的组件）
	UFUNCTION(BlueprintCallable)
	static void GetInteractableTargetsFromActor(AActor* Actor, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets);

	// 从一批重叠命中结果里，把每个命中的 Actor / 组件中能交互的追加进输出数组（仅 C++ 用）
	static void AppendInteractableTargetsFromOverlapResults(const TArray<FOverlapResult>& OverlapResults, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets);

	// 从单次射线命中结果里，把命中的 Actor / 组件中能交互的追加进输出数组（仅 C++ 用）
	static void AppendInteractableTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets);
};