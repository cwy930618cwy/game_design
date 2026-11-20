// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkComponent.h"

#include "TenkaichiHealthComponent.generated.h"

// 把 UE_API 宏临时替换成项目的导出宏（TENKAICHIGAME_API），文件结束再还原成引擎默认
#define UE_API TENKAICHIGAME_API

class UTenkaichiHealthComponent;

class UTenkaichiAbilitySystemComponent;
class UTenkaichiHealthSet;
class UObject;
struct FFrame;
struct FGameplayEffectSpec;

// 死亡事件委托：死亡开始 / 死亡完成时触发，参数是拥有者 Actor
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTenkaichiHealth_DeathEvent, AActor*, OwningActor);
// 属性变化事件委托：血量 / 最大血量变化时触发，携带组件、旧值、新值、施加者
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FTenkaichiHealth_AttributeChanged, UTenkaichiHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);

/**
 * ETenkaichiDeathState
 *
 *	定义当前的死亡状态（三态：活着 / 开始死亡 / 死亡完成）。
 */
UENUM(BlueprintType)
enum class ETenkaichiDeathState : uint8
{
	NotDead = 0,
	DeathStarted,
	DeathFinished
};


/**
 * UTenkaichiHealthComponent
 *
 *	一个 Actor 组件，用来处理与血量相关的一切。
 */
UCLASS(MinimalAPI, Blueprintable, Meta=(BlueprintSpawnableComponent))
class UTenkaichiHealthComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:

	// 构造函数：关闭 Tick、开启网络复制，并把三个字段置空
	UE_API UTenkaichiHealthComponent(const FObjectInitializer& ObjectInitializer);

	// 如果指定的 Actor 上存在血量组件，则返回它
	UFUNCTION(BlueprintPure, Category = "Tenkaichi|Health")
	static UTenkaichiHealthComponent* FindHealthComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UTenkaichiHealthComponent>() : nullptr); }

	// 使用给定的能力系统组件来初始化本组件
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Health")
	UE_API void InitializeWithAbilitySystem(UTenkaichiAbilitySystemComponent* InASC);

	// 反初始化本组件，清空对能力系统的所有引用
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Health")
	UE_API void UninitializeFromAbilitySystem();

	// 返回当前的血量值
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Health")
	UE_API float GetHealth() const;

	// 返回当前的最大血量值
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Health")
	UE_API float GetMaxHealth() const;

	// 返回当前血量归一化后的值，范围 [0.0, 1.0]
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Health")
	UE_API float GetHealthNormalized() const;

	// 返回当前的死亡状态
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Health")
	ETenkaichiDeathState GetDeathState() const { return DeathState; }

	// 判断是否已经死亡或正在死亡
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Tenkaichi|Health", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsDeadOrDying() const { return (DeathState > ETenkaichiDeathState::NotDead); }

	// 为拥有者开启死亡流程
	UE_API virtual void StartDeath();

	// 为拥有者结束死亡流程
	UE_API virtual void FinishDeath();

	// 施加足以杀死拥有者的伤害
	UE_API virtual void DamageSelfDestruct(bool bFellOutOfWorld = false);

public:

	// 血量值发生变化时触发的委托（在客户端也会触发，但施加者可能无效）
	UPROPERTY(BlueprintAssignable)
	FTenkaichiHealth_AttributeChanged OnHealthChanged;

	// 最大血量值发生变化时触发的委托（在客户端也会触发，但施加者可能无效）
	UPROPERTY(BlueprintAssignable)
	FTenkaichiHealth_AttributeChanged OnMaxHealthChanged;

	// 死亡流程开始时触发的委托
	UPROPERTY(BlueprintAssignable)
	FTenkaichiHealth_DeathEvent OnDeathStarted;

	// 死亡流程完成时触发的委托
	UPROPERTY(BlueprintAssignable)
	FTenkaichiHealth_DeathEvent OnDeathFinished;

protected:

	// 组件注销时自动反初始化，避免残留引用
	UE_API virtual void OnUnregister() override;

	// 清空本组件施加的所有游戏标签
	UE_API void ClearGameplayTags();

	// 血量属性变化时的内部处理：转发为 OnHealthChanged 广播
	UE_API virtual void HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	// 最大血量属性变化时的内部处理：转发为 OnMaxHealthChanged 广播
	UE_API virtual void HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	// 血量耗尽时的内部处理：广播死亡游戏事件与击杀消息
	UE_API virtual void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);

	// 死亡状态从服务器复制到客户端时的回调
	UFUNCTION()
	UE_API virtual void OnRep_DeathState(ETenkaichiDeathState OldDeathState);

protected:

	// 本组件所使用的能力系统组件
	UPROPERTY()
	TObjectPtr<UTenkaichiAbilitySystemComponent> AbilitySystemComponent;

	// 本组件所使用的血量属性集
	UPROPERTY()
	TObjectPtr<const UTenkaichiHealthSet> HealthSet;

	// 用于处理死亡的、带复制的状态
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	ETenkaichiDeathState DeathState;
};

// 文件末尾还原 UE_API 为引擎默认（不要影响其他文件）
#undef UE_API