// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiGameplayAbility.h"
#include "TenkaichiLogChannels.h"
#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "AbilitySystem/TenkaichiGameplayEffectContext.h"
#include "AbilitySystemLog.h"
#include "Player/TenkaichiPlayerController.h"
#include "Character/TenkaichiCharacter.h"
#include "TenkaichiGameplayTags.h"
#include "TenkaichiAbilityCost.h"
#include "Character/TenkaichiHeroComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "TenkaichiAbilitySimpleFailureMessage.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "AbilitySystem/TenkaichiAbilitySourceInterface.h"
#include "Physics/PhysicalMaterialWithTags.h"
#include "GameFramework/PlayerState.h"
#include "Camera/TenkaichiCameraMode.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameplayAbility)

// 宏：如果不是"实例化"的技能，就报错并返回指定返回值（防止在非实例化技能上调用实例方法）
#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue)																				\
{																																						\
	if (!ensure(IsInstantiated()))																														\
	{																																					\
		ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());	\
		return ReturnValue;																																\
	}																																					\
}

// 定义"技能简单失败消息"标签（给 UI 层看的反馈消息通道）
UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_SIMPLE_FAILURE_MESSAGE, "Ability.UserFacingSimpleActivateFail.Message");
// 定义"技能失败播放蒙太奇消息"标签
UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_PLAY_MONTAGE_FAILURE_MESSAGE, "Ability.PlayMontageOnActivateFail.Message");

// 构造函数：设置技能的各种默认策略
UTenkaichiGameplayAbility::UTenkaichiGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	ActivationPolicy = ELyraAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = ELyraAbilityActivationGroup::Independent;

	bLogCancelation = false;

	ActiveCameraMode = nullptr;
}

// 从 ActorInfo 拿本项目定制的 AbilitySystemComponent
UTenkaichiAbilitySystemComponent* UTenkaichiGameplayAbility::GetTenkaichiAbilitySystemComponentFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<UTenkaichiAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr);
}

// 从 ActorInfo 拿本项目定制的玩家控制器
ATenkaichiPlayerController* UTenkaichiGameplayAbility::GetTenkaichiPlayerControllerFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<ATenkaichiPlayerController>(CurrentActorInfo->PlayerController.Get()) : nullptr);
}

// 从 ActorInfo 拿控制器（兼容非玩家控制器，会沿着 Owner 链找）
AController* UTenkaichiGameplayAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (AController* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}

		// 在 Owner 链上找玩家控制器或 Pawn
		AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (AController* C = Cast<AController>(TestActor))
			{
				return C;
			}

			if (APawn* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

// 从 ActorInfo 拿本项目定制的角色
ATenkaichiCharacter* UTenkaichiGameplayAbility::GetTenkaichiCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<ATenkaichiCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

// 从 ActorInfo 拿 Hero 组件
UTenkaichiHeroComponent* UTenkaichiGameplayAbility::GetHeroComponentFromActorInfo() const
{
	return (CurrentActorInfo ? UTenkaichiHeroComponent::FindHeroComponent(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

// 技能激活失败时：找失败原因对应的用户消息/蒙太奇，通过消息总线广播出去
void UTenkaichiGameplayAbility::NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
{
	bool bSimpleFailureFound = false;
	for (FGameplayTag Reason : FailedReason)
	{
		if (!bSimpleFailureFound)
		{
			if (const FText* pUserFacingMessage = FailureTagToUserFacingMessages.Find(Reason))
			{
				FTenkaichiAbilitySimpleFailureMessage Message;
				Message.PlayerController = GetActorInfo().PlayerController.Get();
				Message.FailureTags = FailedReason;
				Message.UserFacingReason = *pUserFacingMessage;

				UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
				MessageSystem.BroadcastMessage(TAG_ABILITY_SIMPLE_FAILURE_MESSAGE, Message);
				bSimpleFailureFound = true;
			}
		}
		
		if (UAnimMontage* pMontage = FailureTagToAnimMontage.FindRef(Reason))
		{
			FTenkaichiAbilityMontageFailureMessage Message;
			Message.PlayerController = GetActorInfo().PlayerController.Get();
			Message.AvatarActor = GetActorInfo().AvatarActor.Get();
			Message.FailureTags = FailedReason;
			Message.FailureMontage = pMontage;

			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(TAG_ABILITY_PLAY_MONTAGE_FAILURE_MESSAGE, Message);
		}
	}
}

// 能否激活：除了引擎检查，还加"激活组互斥"判断
bool UTenkaichiGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	//@TODO 等标签关系建立后可能移除
	UTenkaichiAbilitySystemComponent* TenkaichiASC = CastChecked<UTenkaichiAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (TenkaichiASC->IsActivationGroupBlocked(ActivationGroup))
	{
		if (OptionalRelevantTags)
		{
			OptionalRelevantTags->AddTag(TenkaichiGameplayTags::Ability_ActivateFail_ActivationGroup);
		}
		return false;
	}

	return true;
}

// 设置能否取消：可替换组不能阻止取消
void UTenkaichiGameplayAbility::SetCanBeCanceled(bool bCanBeCanceled)
{
	// 如果技能是可替换组，它不能阻止取消
	if (!bCanBeCanceled && (ActivationGroup == ELyraAbilityActivationGroup::Exclusive_Replaceable))
	{
		UE_LOG(LogTenkaichiAbilitySystem, Error, TEXT("SetCanBeCanceled: Ability [%s] can not block canceling because its activation group is replaceable."), *GetName());
		return;
	}

	Super::SetCanBeCanceled(bCanBeCanceled);
}

// 技能被授予时：触发蓝图事件 + 尝试出生激活
void UTenkaichiGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	K2_OnAbilityAdded();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

// 技能被移除时：触发蓝图事件
void UTenkaichiGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	K2_OnAbilityRemoved();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

// 正式激活技能（目前就是透传给引擎）
void UTenkaichiGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

// 结束技能（先清相机，再透传给引擎）
void UTenkaichiGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearCameraMode();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// 检查代价：引擎检查 + 所有额外代价（AbilityCost）
bool UTenkaichiGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) || !ActorInfo)
	{
		return false;
	}

	// 检查所有额外代价是否付得起
	for (const TObjectPtr<UTenkaichiAbilityCost>& AdditionalCost : AdditionalCosts)
	{
		if (AdditionalCost != nullptr)
		{
			if (!AdditionalCost->CheckCost(this, Handle, ActorInfo, /*inout*/ OptionalRelevantTags))
			{
				return false;
			}
		}
	}

	return true;
}

// 施加代价：引擎扣 + 所有额外代价，且支持"只在命中时扣"
void UTenkaichiGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

	check(ActorInfo);

	// 判断技能是否真的打中了目标（因为有些代价只在命中成功时扣）
	auto DetermineIfAbilityHitTarget = [&]()
	{
		if (ActorInfo->IsNetAuthority())
		{
			if (UTenkaichiAbilitySystemComponent* ASC = Cast<UTenkaichiAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
			{
				FGameplayAbilityTargetDataHandle TargetData;
				ASC->GetAbilityTargetData(Handle, ActivationInfo, TargetData);
				for (int32 TargetDataIdx = 0; TargetDataIdx < TargetData.Data.Num(); ++TargetDataIdx)
				{
					if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetData, TargetDataIdx))
					{
						return true;
					}
				}
			}
		}

		return false;
	};

	// 施加所有额外代价
	bool bAbilityHitTarget = false;
	bool bHasDeterminedIfAbilityHitTarget = false;
	for (const TObjectPtr<UTenkaichiAbilityCost>& AdditionalCost : AdditionalCosts)
	{
		if (AdditionalCost != nullptr)
		{
			if (AdditionalCost->ShouldOnlyApplyCostOnHit())
			{
				if (!bHasDeterminedIfAbilityHitTarget)
				{
					bAbilityHitTarget = DetermineIfAbilityHitTarget();
					bHasDeterminedIfAbilityHitTarget = true;
				}

				if (!bAbilityHitTarget)
				{
					continue;
				}
			}

			AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
		}
	}
}

// 构造技能产生的效果上下文：注入能力来源
FGameplayEffectContextHandle UTenkaichiGameplayAbility::MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	FGameplayEffectContextHandle ContextHandle = Super::MakeEffectContext(Handle, ActorInfo);

	FTenkaichiGameplayEffectContext* EffectContext = FTenkaichiGameplayEffectContext::ExtractEffectContext(ContextHandle);
	check(EffectContext);

	check(ActorInfo);

	AActor* EffectCauser = nullptr;
	const ITenkaichiAbilitySourceInterface* AbilitySource = nullptr;
	float SourceLevel = 0.0f;
	GetAbilitySource(Handle, ActorInfo, /*out*/ SourceLevel, /*out*/ AbilitySource, /*out*/ EffectCauser);

	UObject* SourceObject = GetSourceObject(Handle, ActorInfo);

	AActor* Instigator = ActorInfo ? ActorInfo->OwnerActor.Get() : nullptr;

	EffectContext->SetAbilitySource(AbilitySource, SourceLevel);
	EffectContext->AddInstigator(Instigator, EffectCauser);
	EffectContext->AddSourceObject(SourceObject);

	return ContextHandle;
}

// 把技能标签应用到效果上（这里额外把命中物理材质的标签并入）
void UTenkaichiGameplayAbility::ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const
{
	Super::ApplyAbilityTagsToGameplayEffectSpec(Spec, AbilitySpec);

	if (const FHitResult* HitResult = Spec.GetContext().GetHitResult())
	{
		if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(HitResult->PhysMaterial.Get()))
		{
			Spec.CapturedTargetTags.GetSpecTags().AppendTags(PhysMatWithTags->Tags);
		}
	}
}

// 检查技能是否满足标签要求（处理死亡禁止 + 用 ASC 展开额外要求）
bool UTenkaichiGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	// 专门处理死亡排除和通过 ASC 展开的 AbilityTags

	bool bBlocked = false;
	bool bMissing = false;

	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	// 检查这个技能的任何标签当前是否被阻塞
	if (AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags()))
	{
		bBlocked = true;
	}

	const UTenkaichiAbilitySystemComponent* TenkaichiASC = Cast<UTenkaichiAbilitySystemComponent>(&AbilitySystemComponent);
	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;

	// 展开我们的技能标签，加上额外的 required/blocked 标签
	if (TenkaichiASC)
	{
		TenkaichiASC->GetAdditionalActivationTagRequirements(GetAssetTags(), AllRequiredTags, AllBlockedTags);
	}

	// 检查这个技能的 required/blocked 标签
	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;
		
		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(AllBlockedTags))
		{
			if (OptionalRelevantTags && AbilitySystemComponentTags.HasTag(TenkaichiGameplayTags::Status_Death))
			{
				// 如果玩家死亡且因阻塞标签被拒绝，给出那个反馈
				OptionalRelevantTags->AddTag(TenkaichiGameplayTags::Ability_ActivateFail_IsDead);
			}

			bBlocked = true;
		}

		if (!AbilitySystemComponentTags.HasAll(AllRequiredTags))
		{
			bMissing = true;
		}
	}

	if (SourceTags != nullptr)
	{
		if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
		{
			if (SourceTags->HasAny(SourceBlockedTags))
			{
				bBlocked = true;
			}

			if (!SourceTags->HasAll(SourceRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (TargetTags != nullptr)
	{
		if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
		{
			if (TargetTags->HasAny(TargetBlockedTags))
			{
				bBlocked = true;
			}

			if (!TargetTags->HasAll(TargetRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (bBlocked)
	{
		if (OptionalRelevantTags && BlockedTag.IsValid())
		{
			OptionalRelevantTags->AddTag(BlockedTag);
		}
		return false;
	}
	if (bMissing)
	{
		if (OptionalRelevantTags && MissingTag.IsValid())
		{
			OptionalRelevantTags->AddTag(MissingTag);
		}
		return false;
	}

	return true;
}

// 技能被赋予 Pawn 化身时：触发蓝图事件
void UTenkaichiGameplayAbility::OnPawnAvatarSet()
{
	K2_OnPawnAvatarSet();
}

// 获取技能的来源（等级、来源接口、效果制造者）
void UTenkaichiGameplayAbility::GetAbilitySource(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& OutSourceLevel, const ITenkaichiAbilitySourceInterface*& OutAbilitySource, AActor*& OutEffectCauser) const
{
	OutSourceLevel = 0.0f;
	OutAbilitySource = nullptr;
	OutEffectCauser = nullptr;

	OutEffectCauser = ActorInfo->AvatarActor.Get();

	// 如果这个技能是被某个"能力来源"添加的，就使用它
	UObject* SourceObject = GetSourceObject(Handle, ActorInfo);

	OutAbilitySource = Cast<ITenkaichiAbilitySourceInterface>(SourceObject);
}

// 如果激活策略是 OnSpawn，尝试在出生时激活技能
void UTenkaichiGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{
	// 如果激活策略是出生即触发，就尝试激活
	if (ActorInfo && !Spec.IsActive() && (ActivationPolicy == ELyraAbilityActivationPolicy::OnSpawn))
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		// 如果化身已撕裂或即将死亡，等新化身来了再尝试激活
		if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
			const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

			const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
			const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
		}
	}
}

// 返回"能否切到新激活组"
bool UTenkaichiGameplayAbility::CanChangeActivationGroup(ELyraAbilityActivationGroup NewGroup) const
{
	if (!IsInstantiated() || !IsActive())
	{
		return false;
	}

	if (ActivationGroup == NewGroup)
	{
		return true;
	}

	UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponentFromActorInfo();
	check(TenkaichiASC);

	if ((ActivationGroup != ELyraAbilityActivationGroup::Exclusive_Blocking) && TenkaichiASC->IsActivationGroupBlocked(NewGroup))
	{
		// 这个技能不能切组，如果它被阻塞了（除非它是阻塞别人的那个）
		return false;
	}

	if ((NewGroup == ELyraAbilityActivationGroup::Exclusive_Replaceable) && !CanBeCanceled())
	{
		// 这个技能不能变成"可替换"如果它不能被取消
		return false;
	}

	return true;
}

// 尝试切换激活组（成功返回 true）
bool UTenkaichiGameplayAbility::ChangeActivationGroup(ELyraAbilityActivationGroup NewGroup)
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ChangeActivationGroup, false);

	if (!CanChangeActivationGroup(NewGroup))
	{
		return false;
	}

	if (ActivationGroup != NewGroup)
	{
		UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponentFromActorInfo();
		check(TenkaichiASC);

		TenkaichiASC->RemoveAbilityFromActivationGroup(ActivationGroup, this);
		TenkaichiASC->AddAbilityToActivationGroup(NewGroup, this);

		ActivationGroup = NewGroup;
	}

	return true;
}

// 设置技能的相机模式
void UTenkaichiGameplayAbility::SetCameraMode(TSubclassOf<UTenkaichiCameraMode> CameraMode)
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(SetCameraMode, );

	if (UTenkaichiHeroComponent* HeroComponent = GetHeroComponentFromActorInfo())
	{
		HeroComponent->SetAbilityCameraMode(CameraMode, CurrentSpecHandle);
		ActiveCameraMode = CameraMode;
	}
}

// 清除技能的相机模式
void UTenkaichiGameplayAbility::ClearCameraMode()
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ClearCameraMode, );

	if (ActiveCameraMode)
	{
		if (UTenkaichiHeroComponent* HeroComponent = GetHeroComponentFromActorInfo())
		{
			HeroComponent->ClearAbilityCameraMode(CurrentSpecHandle);
		}

		ActiveCameraMode = nullptr;
	}
}
