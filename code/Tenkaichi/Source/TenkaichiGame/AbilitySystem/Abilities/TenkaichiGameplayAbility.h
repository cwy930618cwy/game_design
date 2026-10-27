// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"

#include "TenkaichiGameplayAbility.generated.h"

// 把 UE_API 宏临时替换成项目的导出宏（TENKAICHIGAME_API），文件结束再还原成引擎默认
#define UE_API TENKAICHIGAME_API

struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpec;
struct FGameplayAbilitySpecHandle;

class AActor;
class AController;
class ATenkaichiCharacter;
class ATenkaichiPlayerController;
class APlayerController;
class FText;
class ITenkaichiAbilitySourceInterface;
class UAnimMontage;
class UTenkaichiAbilityCost;
class UTenkaichiAbilitySystemComponent;
class UTenkaichiCameraMode;
class UTenkaichiHeroComponent;
class UObject;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEffectSpec;
struct FGameplayEventData;

/**
 * ELyraAbilityActivationPolicy（激活策略）
 *
 *	定义这个技能"应该怎么被触发"。
 */
UENUM(BlueprintType)
enum class ELyraAbilityActivationPolicy : uint8
{
	// 当输入被触发时尝试激活技能
	OnInputTriggered,

	// 当输入持续按下时持续尝试激活技能
	WhileInputActive,

	// 当分配了 Avatar（化身）时激活技能
	OnSpawn
};


/**
 * ELyraAbilityActivationGroup（激活组）
 *
 *	定义这个技能"相对其他技能"是如何激活的（互斥关系）。
 */
UENUM(BlueprintType)
enum class ELyraAbilityActivationGroup : uint8
{
	// 技能独立运行，不受其他技能影响
	Independent,

	// 技能会被其他"排他"技能取消并替换
	Exclusive_Replaceable,

	// 技能阻止所有其他"排他"技能激活
	Exclusive_Blocking,

	MAX	UMETA(Hidden)
};

/** 技能激活失败的原因，可用来在失败时播放一段动画蒙太奇 */
USTRUCT(BlueprintType)
struct FTenkaichiAbilityMontageFailureMessage
{
	GENERATED_BODY()

public:
	// 激活失败的那个玩家控制器（如果 AbilitySystemComponent 是玩家拥有的）
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	// 激活失败的化身（Avatar）Actor
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> AvatarActor = nullptr;

	// 这个技能失败的所有原因
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> FailureMontage = nullptr;
};

/**
 * UTenkaichiGameplayAbility
 *
 *	本项目使用的技能基类。
 */
UCLASS(MinimalAPI, Abstract, HideCategories = Input, Meta = (ShortTooltip = "The base gameplay ability class used by this project."))
class UTenkaichiGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	friend class UTenkaichiAbilitySystemComponent;

public:

	UE_API UTenkaichiGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 从 ActorInfo 里拿到本项目定制的 AbilitySystemComponent（GAS 能力组件）
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Ability")
	UE_API UTenkaichiAbilitySystemComponent* GetTenkaichiAbilitySystemComponentFromActorInfo() const;

	// 从 ActorInfo 里拿到本项目定制的玩家控制器
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Ability")
	UE_API ATenkaichiPlayerController* GetTenkaichiPlayerControllerFromActorInfo() const;

	// 从 ActorInfo 里拿到控制器（兼容非玩家控制器）
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Ability")
	UE_API AController* GetControllerFromActorInfo() const;

	// 从 ActorInfo 里拿到本项目定制的角色
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Ability")
	UE_API ATenkaichiCharacter* GetTenkaichiCharacterFromActorInfo() const;

	// 从 ActorInfo 里拿到 Hero 组件（角色核心组件）
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Ability")
	UE_API UTenkaichiHeroComponent* GetHeroComponentFromActorInfo() const;

	// 返回技能的激活策略（怎么被触发）
	ELyraAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	// 返回技能的激活组（怎么跟别的技能互斥）
	ELyraAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

	// 如果激活策略是 OnSpawn（出生即触发），尝试在出生时激活技能
	UE_API void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

	// 返回"请求的激活组"是否是一个合法的切换（即能否切到新组）
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Tenkaichi|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	UE_API bool CanChangeActivationGroup(ELyraAbilityActivationGroup NewGroup) const;

	// 尝试切换激活组。返回 true 表示切换成功。
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Tenkaichi|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	UE_API bool ChangeActivationGroup(ELyraAbilityActivationGroup NewGroup);

	// 设置技能的相机模式
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Ability")
	UE_API void SetCameraMode(TSubclassOf<UTenkaichiCameraMode> CameraMode);

	// 清除技能的相机模式。技能结束时如有需要会自动调用。
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Ability")
	UE_API void ClearCameraMode();

	// 技能激活失败时调用：同时触发原生回调与蓝图事件
	void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
	{
		NativeOnAbilityFailedToActivate(FailedReason);
		ScriptOnAbilityFailedToActivate(FailedReason);
	}

protected:

	// 技能激活失败时调用（原生 C++ 实现）
	UE_API virtual void NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

	// 技能激活失败时调用（蓝图事件，可在蓝图里覆盖）
	UFUNCTION(BlueprintImplementableEvent)
	UE_API void ScriptOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

	//~UGameplayAbility 接口开始
	// 询问：这个技能能否被激活
	UE_API virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	// 设置技能能否被取消
	UE_API virtual void SetCanBeCanceled(bool bCanBeCanceled) override;
	// 技能被授予到能力组件时调用
	UE_API virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	// 技能从能力组件移除时调用
	UE_API virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	// 正式激活技能
	UE_API virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	// 结束技能
	UE_API virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	// 检查技能代价（付得起吗）
	UE_API virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	// 施加技能代价（扣钱/扣弹药）
	UE_API virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	// 构造技能产生的 GameplayEffect 上下文
	UE_API virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;
	// 把技能标签应用到生成的 GameplayEffect 上
	UE_API virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const override;
	// 检查技能是否满足标签要求（可激活）
	UE_API virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	//~UGameplayAbility 接口结束

	// 当技能被赋予 Pawn 化身时调用
	UE_API virtual void OnPawnAvatarSet();

	// 获取这个技能的"来源"（等级、来源接口、效果制造者）
	UE_API virtual void GetAbilitySource(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& OutSourceLevel, const ITenkaichiAbilitySourceInterface*& OutAbilitySource, AActor*& OutEffectCauser) const;

	/** 当这个技能被授予给能力组件时调用。 */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
	UE_API void K2_OnAbilityAdded();

	/** 当这个技能从能力组件移除时调用。 */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityRemoved")
	UE_API void K2_OnAbilityRemoved();

	/** 当能力组件用 Pawn 化身初始化时调用。 */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnPawnAvatarSet")
	UE_API void K2_OnPawnAvatarSet();

protected:

	// 定义这个技能"怎么被触发"
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Ability Activation")
	ELyraAbilityActivationPolicy ActivationPolicy;

	// 定义这个技能激活时"与其他技能的关系"
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Ability Activation")
	ELyraAbilityActivationGroup ActivationGroup;

	// 激活这个技能所需支付的额外代价
	UPROPERTY(EditDefaultsOnly, Instanced, Category = Costs)
	TArray<TObjectPtr<UTenkaichiAbilityCost>> AdditionalCosts;

	// 失败标签到简单错误消息的映射
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;

	// 失败标签到应播放动画蒙太奇的映射
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> FailureTagToAnimMontage;

	// 若为 true，技能被取消时打印额外信息（临时，用于追踪一个 bug）
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	bool bLogCancelation;

	// 当前由技能设置的相机模式
	TSubclassOf<UTenkaichiCameraMode> ActiveCameraMode;
};

// 文件末尾还原 UE_API 为引擎默认（不要影响其他文件）
#undef UE_API