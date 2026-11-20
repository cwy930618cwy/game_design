// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/TenkaichiGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"

#include "TenkaichiAbilitySystemComponent.generated.h"

// 把 UE_API 宏临时替换成项目的导出宏（TENKAICHIGAME_API），文件结束再还原成引擎默认
#define UE_API TENKAICHIGAME_API

class AActor;
class UGameplayAbility;
class UTenkaichiAbilityTagRelationshipMapping;
class UObject;
struct FFrame;
struct FGameplayAbilityTargetDataHandle;

// 声明"技能输入被阻塞"这个全局游戏标签（实际定义在 .cpp）
TENKAICHIGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);

/**
 * UTenkaichiAbilitySystemComponent
 *
 *	本项目使用的能力系统组件基类。
 */
UCLASS(MinimalAPI)
class UTenkaichiAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	UE_API UTenkaichiAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent 接口开始
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~UActorComponent 接口结束

	UE_API virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	// 取消技能用的函数类型：传入"是否取消该技能"的判断回调
	typedef TFunctionRef<bool(const UTenkaichiGameplayAbility* TenkaichiAbility, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	UE_API void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);

	// 取消所有"输入触发/按住激活"的技能
	UE_API void CancelInputActivatedAbilities(bool bReplicateCancelAbility);

	// 输入标签被按下：把输入标签翻译成"待激活"的技能句柄
	UE_API void AbilityInputTagPressed(const FGameplayTag& InputTag);
	// 输入标签被松开：把输入标签翻译成"待释放"的技能句柄
	UE_API void AbilityInputTagReleased(const FGameplayTag& InputTag);

	// 每帧处理输入：按住/按下/松开，批量激活技能
	UE_API void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	// 清空输入句柄缓存
	UE_API void ClearAbilityInput();

	// 激活组是否被阻塞（排他组被占用）
	UE_API bool IsActivationGroupBlocked(ELyraAbilityActivationGroup Group) const;
	// 技能加入激活组（计数+1，排他时取消同组其他）
	UE_API void AddAbilityToActivationGroup(ELyraAbilityActivationGroup Group, UTenkaichiGameplayAbility* TenkaichiAbility);
	// 技能离开激活组（计数-1）
	UE_API void RemoveAbilityFromActivationGroup(ELyraAbilityActivationGroup Group, UTenkaichiGameplayAbility* TenkaichiAbility);
	// 取消指定激活组的所有技能
	UE_API void CancelActivationGroupAbilities(ELyraAbilityActivationGroup Group, UTenkaichiGameplayAbility* IgnoreTenkaichiAbility, bool bReplicateCancelAbility);

	// 用一个 GameplayEffect 添加指定的动态授予标签
	UE_API void AddDynamicTagGameplayEffect(const FGameplayTag& Tag);

	// 移除所有用来添加指定动态授予标签的 GameplayEffect 实例
	UE_API void RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag);

	// 取与给定技能句柄和激活信息关联的技能目标数据
	UE_API void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

	// 设置当前的标签关系映射表，如果传 null 则清空它
	UE_API void SetTagRelationshipMapping(UTenkaichiAbilityTagRelationshipMapping* NewMapping);

	// 查看技能标签，收集额外的"必需标签"和"阻塞标签"
	UE_API void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const;

	// 遍历技能，触发 OnSpawn（出生即触发）策略的技能激活
	UE_API void TryActivateAbilitiesOnSpawn();

protected:

	UE_API virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	UE_API virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	UE_API virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	UE_API virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;
	UE_API virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
	UE_API virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;
	UE_API virtual void HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled) override;

	// 通知客户端：某个技能激活失败
	UFUNCTION(Client, Unreliable)
	UE_API void ClientNotifyAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

	UE_API void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);
protected:

	// 若设置了这个表，就用它查找激活/取消的标签关系
	UPROPERTY()
	TObjectPtr<UTenkaichiAbilityTagRelationshipMapping> TagRelationshipMapping;

	// 本帧输入被按下的技能句柄
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// 本帧输入被松开的技能句柄
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// 输入一直被按住的技能句柄
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	// 每个激活组正在运行的技能数量
	int32 ActivationGroupCounts[(uint8)ELyraAbilityActivationGroup::MAX];
};

// 文件末尾还原 UE_API 为引擎默认（不要影响其他文件）
#undef UE_API