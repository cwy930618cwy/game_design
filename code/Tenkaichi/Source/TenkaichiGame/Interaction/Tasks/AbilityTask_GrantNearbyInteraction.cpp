// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilityTask_GrantNearbyInteraction.h"

#include "AbilitySystemComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionOption.h"
#include "Interaction/InteractionQuery.h"
#include "Interaction/InteractionStatics.h"
#include "Physics/TenchiCollisionChannels.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_GrantNearbyInteraction)

UAbilityTask_GrantNearbyInteraction::UAbilityTask_GrantNearbyInteraction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAbilityTask_GrantNearbyInteraction* UAbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractors(UGameplayAbility* OwningAbility, float InteractionScanRange, float InteractionScanRate)
{
	// 用 GAS 提供的工具创建任务实例
	UAbilityTask_GrantNearbyInteraction* MyObj = NewAbilityTask<UAbilityTask_GrantNearbyInteraction>(OwningAbility);

	// 把调用者传进来的扫描半径 / 间隔存到任务上
	MyObj->InteractionScanRange = InteractionScanRange;
	MyObj->InteractionScanRate = InteractionScanRate;

	return MyObj;
}

void UAbilityTask_GrantNearbyInteraction::Activate()
{
	SetWaitingOnAvatar();

	UWorld* World = GetWorld();
	// 挂一个循环定时器，每 InteractionScanRate 秒触发一次扫描
	World->GetTimerManager().SetTimer(QueryTimerHandle, this, &ThisClass::QueryInteractables, InteractionScanRate, true);
}

void UAbilityTask_GrantNearbyInteraction::OnDestroy(bool AbilityEnded)
{
	if (UWorld* World = GetWorld())
	{
		// 任务结束，取消定时器，避免野定时器继续跑
		World->GetTimerManager().ClearTimer(QueryTimerHandle);
	}

	Super::OnDestroy(AbilityEnded);
}

void UAbilityTask_GrantNearbyInteraction::QueryInteractables()
{
	UWorld* World = GetWorld();
	AActor* ActorOwner = GetAvatarActor();
	
	if (World && ActorOwner)
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(UAbilityTask_GrantNearbyInteraction), false);

		// 以角色为中心，做一次球形扫描，撞出 InteractionScanRange 半径内所有重叠物体
		TArray<FOverlapResult> OverlapResults;
		World->OverlapMultiByChannel(OUT OverlapResults, ActorOwner->GetActorLocation(), FQuat::Identity, Tenchi_TraceChannel_Interaction, FCollisionShape::MakeSphere(InteractionScanRange), Params);

		if (OverlapResults.Num() > 0)
		{
			// 第一步：把重叠结果转成「可交互目标」列表
			TArray<TScriptInterface<IInteractableTarget>> InteractableTargets;
			UInteractionStatics::AppendInteractableTargetsFromOverlapResults(OverlapResults, OUT InteractableTargets);
			
			// 第二步：构造交互查询条件（谁在请求交互）
			FInteractionQuery InteractionQuery;
			InteractionQuery.RequestingAvatar = ActorOwner;
			InteractionQuery.RequestingController = Cast<AController>(ActorOwner->GetOwner());

			// 第三步：问每个可交互目标「你能提供什么交互」，收集成选项列表
			TArray<FInteractionOption> Options;
			for (TScriptInterface<IInteractableTarget>& InteractiveTarget : InteractableTargets)
			{
				FInteractionOptionBuilder InteractionBuilder(InteractiveTarget, Options);
				InteractiveTarget->GatherInteractionOptions(InteractionQuery, InteractionBuilder);
			}

			// 第四步：对每个需要「先授权能力」的选项，把能力授予玩家的 GAS（只授一次）
			for (FInteractionOption& Option : Options)
			{
				if (Option.InteractionAbilityToGrant)
				{
					// 用能力对象作为缓存 key，避免重复授权
					FObjectKey ObjectKey(Option.InteractionAbilityToGrant);
					if (!InteractionCache.Find(ObjectKey))
					{
						FGameplayAbilitySpec Spec(Option.InteractionAbilityToGrant, 1, INDEX_NONE, this);
						FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(Spec);
						InteractionCache.Add(ObjectKey, Handle);
					}
				}
			}
		}
	}
}