// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiWeaponInstance.h"

#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/AssertionMacros.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "GameFramework/InputDeviceProperties.h"
#include "Character/TenkaichiHealthComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiWeaponInstance)

class UAnimInstance;
struct FGameplayTagContainer;

// 构造函数：监听宿主 Pawn 的死亡，死亡时也要移除已生效的手柄震动
UTenkaichiWeaponInstance::UTenkaichiWeaponInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 监听宿主 Pawn 的死亡，这样即使死亡无法正常卸下武器，也能移除已生效的手柄震动
	if (APawn* Pawn = GetPawn())
	{
		// 只有玩家控制的 Pawn 才需要（AI 等没有客户端输入设备）
		if (Pawn->IsPlayerControlled())
		{
			if (UTenkaichiHealthComponent* HealthComponent = UTenkaichiHealthComponent::FindHealthComponent(GetPawn()))
			{
				HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
			}
		}
	}
}

// 装备时：记录装备时间 + 应用手柄震动
void UTenkaichiWeaponInstance::OnEquipped()
{
	Super::OnEquipped();

	UWorld* World = GetWorld();
	check(World);
	TimeLastEquipped = World->GetTimeSeconds();

	ApplyDeviceProperties();
}

// 卸下时：移除已生效的手柄震动
void UTenkaichiWeaponInstance::OnUnequipped()
{
	Super::OnUnequipped();

	RemoveDeviceProperties();
}

// 开火时调用：记录本次开火时间
void UTenkaichiWeaponInstance::UpdateFiringTime()
{
	UWorld* World = GetWorld();
	check(World);
	TimeLastFired = World->GetTimeSeconds();
}

// 返回距上次"开火或装备"过了多久（决定武器是否可切换）
float UTenkaichiWeaponInstance::GetTimeSinceLastInteractedWith() const
{
	UWorld* World = GetWorld();
	check(World);
	const double WorldTime = World->GetTimeSeconds();

	double Result = WorldTime - TimeLastEquipped;

	if (TimeLastFired > 0.0)
	{
		const double TimeSinceFired = WorldTime - TimeLastFired;
		Result = FMath::Min(Result, TimeSinceFired);
	}

	return Result;
}

// ⚠️【注释挂起 P17】按外观标签从对应动画层集合里选出最佳动画层（Cosmetics 阶段未建，先 #if 0 挂起）
#if 0
TSubclassOf<UAnimInstance> UTenkaichiWeaponInstance::PickBestAnimLayer(bool bEquipped, const FGameplayTagContainer& CosmeticTags) const
{
	const FTenkaichiAnimLayerSelectionSet& SetToQuery = (bEquipped ? EquippedAnimSet : UneuippedAnimSet);
	return SetToQuery.SelectBestLayer(CosmeticTags);
}
#endif

// 返回拥有者 Pawn 的平台用户 ID（用于手柄震动）
const FPlatformUserId UTenkaichiWeaponInstance::GetOwningUserId() const
{
	if (const APawn* Pawn = GetPawn())
	{
		return Pawn->GetPlatformUserId();
	}
	return PLATFORMUSERID_NONE;
}

// 把 ApplicableProperty 应用到拥有者 Pawn 上（循环播放，生命周期跟随武器装备期间）
void UTenkaichiWeaponInstance::ApplyDeviceProperties()
{
	const FPlatformUserId UserId = GetOwningUserId();

	if (UserId.IsValid())
	{
		if (UInputDeviceSubsystem* InputDeviceSubsystem = UInputDeviceSubsystem::Get())
		{
			for (TObjectPtr<UInputDeviceProperty>& DeviceProp : ApplicableDeviceProperties)
			{
				FActivateDevicePropertyParams Params = {};
				Params.UserId = UserId;

				// 默认会在平台用户的主输入设备上播放震动
				// 想指定特定设备时，可设置 DeviceId 参数
				//Params.DeviceId = <some specific device id>;

				// 这里不自动移除：希望震动持续到武器被卸下，由 OnUnequipped 手动移除
				Params.bLooping = true;

				DevicePropertyHandles.Emplace(InputDeviceSubsystem->ActivateDeviceProperty(DeviceProp, Params));
			}
		}
	}
}

// 移除 ApplyDeviceProperties 里激活的所有手柄震动属性
void UTenkaichiWeaponInstance::RemoveDeviceProperties()
{
	const FPlatformUserId UserId = GetOwningUserId();

	if (UserId.IsValid() && !DevicePropertyHandles.IsEmpty())
	{
		// 移除所有已生效的手柄震动属性
		if (UInputDeviceSubsystem* InputDeviceSubsystem = UInputDeviceSubsystem::Get())
		{
			InputDeviceSubsystem->RemoveDevicePropertyHandles(DevicePropertyHandles);
			DevicePropertyHandles.Empty();
		}
	}
}

// 宿主死亡回调：移除可能仍生效的手柄震动，避免残留
void UTenkaichiWeaponInstance::OnDeathStarted(AActor* OwningActor)
{
	// 死亡时移除可能仍生效的手柄震动，避免残留
	RemoveDeviceProperties();
}