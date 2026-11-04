// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Equipment/TenkaichiEquipmentInstance.h"
#include "GameFramework/InputDevicePropertyHandle.h"

// ⚠️【注释挂起 P17】以下 include 依赖 Cosmetics 阶段（`TenkaichiCosmeticAnimationTypes.h` 未建），先注释掉
// #include "Cosmetics/TenkaichiCosmeticAnimationTypes.h"

#include "TenkaichiWeaponInstance.generated.h"

#define UE_API TENKAICHIGAME_API

class UAnimInstance;
class UObject;
struct FFrame;
struct FGameplayTagContainer;
class UInputDeviceProperty;

/**
 * UTenkaichiWeaponInstance
 *
 * 一把生成并应用到 Pawn 上的武器装备
 */
UCLASS(MinimalAPI)
class UTenkaichiWeaponInstance : public UTenkaichiEquipmentInstance
{
	GENERATED_BODY()

public:
	// 构造函数（带默认 ObjectInitializer，允许无参构造）
	UE_API UTenkaichiWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UTenkaichiEquipmentInstance 接口开始
	// 装备时：记录装备时间 + 应用手柄震动
	UE_API virtual void OnEquipped() override;
	// 卸下时：移除手柄震动
	UE_API virtual void OnUnequipped() override;
	//~UTenkaichiEquipmentInstance 接口结束

	// 开火时调用，记录开火时间
	UFUNCTION(BlueprintCallable)
	UE_API void UpdateFiringTime();

	// 返回距上次"开火或装备"过了多久（决定武器是否可切换）
	UFUNCTION(BlueprintPure)
	UE_API float GetTimeSinceLastInteractedWith() const;

protected:
	// ⚠️【注释挂起 P17】以下 2 个成员变量类型 `FTenkaichiAnimLayerSelectionSet` 依赖 Cosmetics 阶段，先注释掉
	// // 装备时用的动画层选择集（P17 再就位）
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Animation)
	// FTenkaichiAnimLayerSelectionSet EquippedAnimSet;
	//
	// // 卸下时用的动画层选择集（P17 再就位）
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Animation)
	// FTenkaichiAnimLayerSelectionSet UneuippedAnimSet;

	/**
	 * 装备期间要应用的手柄震动属性（循环播放，直到卸下武器才停止）
	 */
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Input Devices")
	TArray<TObjectPtr<UInputDeviceProperty>> ApplicableDeviceProperties;

	// ⚠️【注释挂起 P17】以下方法依赖上面两个动画层选择集成员（P17 未就位），先注释掉
	// // 按指定的 GameplayTags 从 EquippedAnimSet 或 UneuippedAnimSet 里选最佳动画层（P17 再就位）
	// UFUNCTION(BlueprintCallable, BlueprintPure=false, Category=Animation)
	// UE_API TSubclassOf<UAnimInstance> PickBestAnimLayer(bool bEquipped, const FGameplayTagContainer& CosmeticTags) const;

	// 返回拥有者 Pawn 的平台用户 ID（用于手柄震动）
	UFUNCTION(BlueprintCallable)
	UE_API const FPlatformUserId GetOwningUserId() const;

	// 拥有者死亡时的回调：移除所有已生效的手柄震动
	UFUNCTION()
	UE_API void OnDeathStarted(AActor* OwningActor);

	/**
	 * 把 ApplicableDeviceProperties 应用到拥有者 Pawn 上（循环播放，生命周期跟随武器装备期间）
	 */
	UE_API void ApplyDeviceProperties();

	// 移除 ApplyDeviceProperties 里激活的所有手柄震动属性
	UE_API void RemoveDeviceProperties();

private:

	// 本武器激活的手柄震动句柄集合（由 ApplyDeviceProperties 填充）
	UPROPERTY(Transient)
	TSet<FInputDevicePropertyHandle> DevicePropertyHandles;

	// 上次装备时间
	double TimeLastEquipped = 0.0;
	// 上次开火时间
	double TimeLastFired = 0.0;
};

#undef UE_API