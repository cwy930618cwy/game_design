// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/TenkaichiGameplayAbility.h"
#include "AbilitySystem/TenkaichiAbilityTagRelationshipMapping.h"
// #include "Animation/TenkaichiAnimInstance.h"    // ⚠️【注释挂起】P16 Animation 未就位，工程无此文件 → 编译不过（见注释备忘录 ㉓-b）
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TenkaichiGlobalAbilitySystem.h"
#include "TenkaichiLogChannels.h"
// #include "System/TenkaichiAssetManager.h"        // ⚠️【注释挂起】P14 AssetManager 未就位，工程无此文件 → 编译不过（见注释备忘录 ㉓-c）
#include "System/TenkaichiGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAbilitySystemComponent)

// 定义"技能输入被阻塞"这个全局游戏标签（实际值：Gameplay.AbilityInputBlocked）
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_AbilityInputBlocked, "Gameplay.AbilityInputBlocked");

// 构造函数：清空输入句柄数组，把激活组计数全部归零
UTenkaichiAbilitySystemComponent::UTenkaichiAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();

	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

// 组件销毁时：从全局技能系统注销自己（EndPlay 生命周期回调）
void UTenkaichiAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UTenkaichiGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UTenkaichiGlobalAbilitySystem>(GetWorld()))
	{
		GlobalAbilitySystem->UnregisterASC(this);
	}

	Super::EndPlay(EndPlayReason);
}

// 设置 Owner/Avatar 时调用：通知所有技能新化身到了、注册全局系统、初始化动画（其中两段挂起）
void UTenkaichiAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (bHasNewPawnAvatar)
	{
		// 通知所有技能：新的 Pawn 化身已设置
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
PRAGMA_DISABLE_DEPRECATION_WARNINGS
			ensureMsgf(AbilitySpec.Ability && AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced, TEXT("InitAbilityActorInfo: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
PRAGMA_ENABLE_DEPRECATION_WARNINGS

			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
			for (UGameplayAbility* AbilityInstance : Instances)
			{
				UTenkaichiGameplayAbility* TenkaichiAbilityInstance = Cast<UTenkaichiGameplayAbility>(AbilityInstance);
				if (TenkaichiAbilityInstance)
				{
					// 回放时技能实例可能缺失，这里判空
					TenkaichiAbilityInstance->OnPawnAvatarSet();
				}
			}
		}

		// 一旦真正有了 Pawn 化身，就注册到全局系统。等到这时才注册，是因为一些全局效果可能需要化身。
		if (UTenkaichiGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UTenkaichiGlobalAbilitySystem>(GetWorld()))
		{
			GlobalAbilitySystem->RegisterASC(this);
		}

#if 0
		// ⚠️【注释挂起】依赖 P16 Animation 的 UTenkaichiAnimInstance（工程无此文件）→ 编译不过。等 P16 就位后放开。
		if (UTenkaichiAnimInstance* TenkaichiAnimInst = Cast<UTenkaichiAnimInstance>(ActorInfo->GetAnimInstance()))
		{
			TenkaichiAnimInst->InitializeWithAbilitySystem(this);
		}
#endif

		TryActivateAbilitiesOnSpawn();
	}
}

// 遍历所有技能，触发 OnSpawn（出生即触发）策略的技能激活
void UTenkaichiAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (const UTenkaichiGameplayAbility* TenkaichiAbilityCDO = Cast<UTenkaichiGameplayAbility>(AbilitySpec.Ability))
		{
			TenkaichiAbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
		}
	}
}

// 按回调条件取消一批技能：对每个正在运行的技能，回调返回 true 就取消它
void UTenkaichiAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		UTenkaichiGameplayAbility* TenkaichiAbilityCDO = Cast<UTenkaichiGameplayAbility>(AbilitySpec.Ability);
		if (!TenkaichiAbilityCDO)
		{
			UE_LOG(LogTenkaichiAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Non-TenkaichiGameplayAbility %s was Granted to ASC. Skipping."), *AbilitySpec.Ability.GetName());
			continue;
		}

PRAGMA_DISABLE_DEPRECATION_WARNINGS
		ensureMsgf(AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced, TEXT("CancelAbilitiesByFunc: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// 取消所有生成的技能实例
		TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
		for (UGameplayAbility* AbilityInstance : Instances)
		{
			UTenkaichiGameplayAbility* TenkaichiAbilityInstance = CastChecked<UTenkaichiGameplayAbility>(AbilityInstance);

			if (ShouldCancelFunc(TenkaichiAbilityInstance, AbilitySpec.Handle))
			{
				if (TenkaichiAbilityInstance->CanBeCanceled())
				{
					TenkaichiAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), TenkaichiAbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
				}
				else
				{
					UE_LOG(LogTenkaichiAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *TenkaichiAbilityInstance->GetName());
				}
			}
		}
	}
}

// 取消所有"输入触发/按住激活"的技能（给 CancelAbilitiesByFunc 提供判断回调）
void UTenkaichiAbilitySystemComponent::CancelInputActivatedAbilities(bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this](const UTenkaichiGameplayAbility* TenkaichiAbility, FGameplayAbilitySpecHandle Handle)
	{
		const ELyraAbilityActivationPolicy ActivationPolicy = TenkaichiAbility->GetActivationPolicy();
		return ((ActivationPolicy == ELyraAbilityActivationPolicy::OnInputTriggered) || (ActivationPolicy == ELyraAbilityActivationPolicy::WhileInputActive));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

// 输入按下：把事件派发给正在运行的技能实例（走复制事件，让 WaitInputPress 任务可用）
void UTenkaichiAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// 我们不支持 UGameplayAbility::bReplicateInputDirectly。
	// 改用复制事件，这样 WaitInputPress 技能任务能正常工作。
	if (Spec.IsActive())
	{
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// 触发 InputPressed 事件。这里不复制。若有人监听，可把该事件复制到服务器。
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, OriginalPredictionKey);
	}
}

// 输入松开：把事件派发给正在运行的技能实例（走复制事件，让 WaitInputRelease 任务可用）
void UTenkaichiAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// 我们不支持 UGameplayAbility::bReplicateInputDirectly。
	// 改用复制事件，这样 WaitInputRelease 技能任务能正常工作。
	if (Spec.IsActive())
	{
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// 触发 InputReleased 事件。这里不复制。若有人监听，可把该事件复制到服务器。
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, OriginalPredictionKey);
	}
}

// 输入标签被按下：把匹配该标签的技能句柄加入"按下/按住"缓存
void UTenkaichiAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

// 输入标签被松开：把匹配该标签的技能句柄加入"松开"缓存，并从"按住"缓存移除
void UTenkaichiAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

// 每帧处理输入：按住/按下/松开，批量激活技能（这是"输入→技能"的核心驱动）
void UTenkaichiAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (HasMatchingGameplayTag(TAG_Gameplay_AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	//@TODO: 看看能不能在这些循环里用 FScopedServerAbilityRPCBatcher ScopedRPCBatcher

	//
	// 处理所有"按住输入时激活"的技能。
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UTenkaichiGameplayAbility* TenkaichiAbilityCDO = Cast<UTenkaichiGameplayAbility>(AbilitySpec->Ability);
				if (TenkaichiAbilityCDO && TenkaichiAbilityCDO->GetActivationPolicy() == ELyraAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	//
	// 处理所有"本帧输入被按下"的技能。
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// 技能正在运行，把输入事件传递给它
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const UTenkaichiGameplayAbility* TenkaichiAbilityCDO = Cast<UTenkaichiGameplayAbility>(AbilitySpec->Ability);

					if (TenkaichiAbilityCDO && TenkaichiAbilityCDO->GetActivationPolicy() == ELyraAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	//
	// 一次性尝试激活所有来自"按下+按住"的技能。
	// 这样按住输入不会既激活技能、又因为按下再给技能发一次输入事件。
	//
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	//
	// 处理所有"本帧输入被松开"的技能。
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// 技能正在运行，把输入事件传递给它
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	//
	// 清空缓存的技能句柄。
	//
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

// 清空所有输入句柄缓存（输入被阻塞时也会调用）
void UTenkaichiAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

// 技能激活时：把技能加入它的激活组（计数+1，排他时取消同组其他）
void UTenkaichiAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (UTenkaichiGameplayAbility* TenkaichiAbility = Cast<UTenkaichiGameplayAbility>(Ability))
	{
		AddAbilityToActivationGroup(TenkaichiAbility->GetActivationGroup(), TenkaichiAbility);
	}
}

// 技能失败时：远程化身→客户端通知；本地→直接处理
void UTenkaichiAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	if (APawn* Avatar = Cast<APawn>(GetAvatarActor()))
	{
		if (!Avatar->IsLocallyControlled() && Ability->IsSupportedForNetworking())
		{
			ClientNotifyAbilityFailed(Ability, FailureReason);
			return;
		}
	}

	HandleAbilityFailed(Ability, FailureReason);
}

// 技能结束时：把技能从它的激活组移除（计数-1）
void UTenkaichiAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (UTenkaichiGameplayAbility* TenkaichiAbility = Cast<UTenkaichiGameplayAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(TenkaichiAbility->GetActivationGroup(), TenkaichiAbility);
	}
}

// 应用技能的 block/cancel 标签：先用标签关系映射表展开，再交给引擎
void UTenkaichiAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer ModifiedBlockTags = BlockTags;
	FGameplayTagContainer ModifiedCancelTags = CancelTags;

	if (TagRelationshipMapping)
	{
		// 用映射表把技能标签展开成 block 和 cancel 标签
		TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
	}

	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, ModifiedBlockTags, bExecuteCancelTags, ModifiedCancelTags);

	//@TODO: 在这里应用任何特殊逻辑，比如阻塞输入或移动
}

// 处理技能可取消状态变化（本类暂无额外逻辑，透传给引擎）
void UTenkaichiAbilitySystemComponent::HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled)
{
	Super::HandleChangeAbilityCanBeCanceled(AbilityTags, RequestingAbility, bCanBeCanceled);

	//@TODO: 在这里应用任何特殊逻辑，比如阻塞输入或移动
}

// 查看技能标签，收集额外的"必需标签"和"阻塞标签"（用映射表展开）
void UTenkaichiAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
{
	if (TagRelationshipMapping)
	{
		TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
	}
}

// 设置当前的标签关系映射表（P05.1 的表）
void UTenkaichiAbilitySystemComponent::SetTagRelationshipMapping(UTenkaichiAbilityTagRelationshipMapping* NewMapping)
{
	TagRelationshipMapping = NewMapping;
}

// 客户端收到技能失败通知：直接走本地失败处理
void UTenkaichiAbilitySystemComponent::ClientNotifyAbilityFailed_Implementation(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	HandleAbilityFailed(Ability, FailureReason);
}

// 技能失败处理：调用技能本体的失败回调（触发 P05.3-B 的失败消息/动画）
void UTenkaichiAbilitySystemComponent::HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	//UE_LOG(LogTenkaichiAbilitySystem, Warning, TEXT("Ability %s failed to activate (tags: %s)"), *GetPathNameSafe(Ability), *FailureReason.ToString());

	if (const UTenkaichiGameplayAbility* TenkaichiAbility = Cast<const UTenkaichiGameplayAbility>(Ability))
	{
		TenkaichiAbility->OnAbilityFailedToActivate(FailureReason);
	}
}

// 激活组是否被阻塞：排他组里有技能在跑，其它排他技能就不能激活
bool UTenkaichiAbilitySystemComponent::IsActivationGroupBlocked(ELyraAbilityActivationGroup Group) const
{
	bool bBlocked = false;

	switch (Group)
	{
	case ELyraAbilityActivationGroup::Independent:
		// 独立技能永远不会被阻塞
		bBlocked = false;
		break;

	case ELyraAbilityActivationGroup::Exclusive_Replaceable:
	case ELyraAbilityActivationGroup::Exclusive_Blocking:
		// 排他技能只有在"有东西在阻塞"时才不能激活
		bBlocked = (ActivationGroupCounts[(uint8)ELyraAbilityActivationGroup::Exclusive_Blocking] > 0);
		break;

	default:
		checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	return bBlocked;
}

// 技能加入激活组：计数+1，若是排他组则取消同组其它"可替换"技能
void UTenkaichiAbilitySystemComponent::AddAbilityToActivationGroup(ELyraAbilityActivationGroup Group, UTenkaichiGameplayAbility* TenkaichiAbility)
{
	check(TenkaichiAbility);
	check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);

	ActivationGroupCounts[(uint8)Group]++;

	const bool bReplicateCancelAbility = false;

	switch (Group)
	{
	case ELyraAbilityActivationGroup::Independent:
		// 独立技能不会取消任何其它技能
		break;

	case ELyraAbilityActivationGroup::Exclusive_Replaceable:
	case ELyraAbilityActivationGroup::Exclusive_Blocking:
		CancelActivationGroupAbilities(ELyraAbilityActivationGroup::Exclusive_Replaceable, TenkaichiAbility, bReplicateCancelAbility);
		break;

	default:
		checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	const int32 ExclusiveCount = ActivationGroupCounts[(uint8)ELyraAbilityActivationGroup::Exclusive_Replaceable] + ActivationGroupCounts[(uint8)ELyraAbilityActivationGroup::Exclusive_Blocking];
	if (!ensure(ExclusiveCount <= 1))
	{
		UE_LOG(LogTenkaichiAbilitySystem, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
	}
}

// 技能离开激活组：计数-1
void UTenkaichiAbilitySystemComponent::RemoveAbilityFromActivationGroup(ELyraAbilityActivationGroup Group, UTenkaichiGameplayAbility* TenkaichiAbility)
{
	check(TenkaichiAbility);
	check(ActivationGroupCounts[(uint8)Group] > 0);

	ActivationGroupCounts[(uint8)Group]--;
}

// 取消指定激活组的所有技能（排除一个忽略对象）
void UTenkaichiAbilitySystemComponent::CancelActivationGroupAbilities(ELyraAbilityActivationGroup Group, UTenkaichiGameplayAbility* IgnoreTenkaichiAbility, bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this, Group, IgnoreTenkaichiAbility](const UTenkaichiGameplayAbility* TenkaichiAbility, FGameplayAbilitySpecHandle Handle)
	{
		return ((TenkaichiAbility->GetActivationGroup() == Group) && (TenkaichiAbility != IgnoreTenkaichiAbility));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

#if 0
// ⚠️ 注释挂起：依赖 P14 UTenkaichiAssetManager（工程无）+ UTenkaichiGameData::Get()（P02✅，但 AssetManager 未就位编译不过）。等 P14 就位后放开。
// 用一个 GameplayEffect 添加指定的动态授予标签
void UTenkaichiAbilitySystemComponent::AddDynamicTagGameplayEffect(const FGameplayTag& Tag)
{
	const TSubclassOf<UGameplayEffect> DynamicTagGE = UTenkaichiAssetManager::GetSubclass(UTenkaichiGameData::Get().DynamicTagGameplayEffect);
	if (!DynamicTagGE)
	{
		UE_LOG(LogTenkaichiAbilitySystem, Warning, TEXT("AddDynamicTagGameplayEffect: Unable to find DynamicTagGameplayEffect [%s]."), *UTenkaichiGameData::Get().DynamicTagGameplayEffect.GetAssetName());
		return;
	}

	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(DynamicTagGE, 1.0f, MakeEffectContext());
	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

	if (!Spec)
	{
		UE_LOG(LogTenkaichiAbilitySystem, Warning, TEXT("AddDynamicTagGameplayEffect: Unable to make outgoing spec for [%s]."), *GetNameSafe(DynamicTagGE));
		return;
	}

	Spec->DynamicGrantedTags.AddTag(Tag);

	ApplyGameplayEffectSpecToSelf(*Spec);
}

// 移除所有用来添加指定动态授予标签的 GameplayEffect 实例
void UTenkaichiAbilitySystemComponent::RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag)
{
	const TSubclassOf<UGameplayEffect> DynamicTagGE = UTenkaichiAssetManager::GetSubclass(UTenkaichiGameData::Get().DynamicTagGameplayEffect);
	if (!DynamicTagGE)
	{
		UE_LOG(LogTenkaichiAbilitySystem, Warning, TEXT("RemoveDynamicTagGameplayEffect: Unable to find gameplay effect [%s]."), *UTenkaichiGameData::Get().DynamicTagGameplayEffect.GetAssetName());
		return;
	}

	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tag));
	Query.EffectDefinition = DynamicTagGE;

	RemoveActiveEffects(Query);
}
#endif

// 取与给定技能句柄和激活信息关联的技能目标数据
void UTenkaichiAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}