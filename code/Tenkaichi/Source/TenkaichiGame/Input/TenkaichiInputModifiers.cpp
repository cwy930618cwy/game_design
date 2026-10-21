// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiInputModifiers.h"

#include "EnhancedPlayerInput.h"
#include "GameFramework/PlayerController.h"
#include "Input/TenkaichiAimSensitivityData.h"
#include "Player/TenkaichiLocalPlayer.h"
#include "Settings/TenkaichiSettingsShared.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiInputModifiers)

DEFINE_LOG_CATEGORY_STATIC(LogTenkaichiInputModifiers, Log, All);

#if 0 // 依赖 P12 TenkaichiLocalPlayer + P18 TenkaichiSettingsShared，暂挂起，就位后解开（见注释备忘录 ⑱）

//////////////////////////////////////////////////////////////////////
// TenkaichiInputModifiersHelpers

namespace TenkaichiInputModifiersHelpers
{
	// 返回一个 Enhanced Player Input 指针所属的 TenkaichiLocalPlayer
	static UTenkaichiLocalPlayer* GetLocalPlayer(const UEnhancedPlayerInput* PlayerInput)
	{
		if (PlayerInput)
		{
			if (APlayerController* PC = Cast<APlayerController>(PlayerInput->GetOuter()))
			{
				return Cast<UTenkaichiLocalPlayer>(PC->GetLocalPlayer());
			}
		}
		return nullptr;
	}
	
}

//////////////////////////////////////////////////////////////////////
// UTenkaichiSettingBasedScalar

FInputActionValue UTenkaichiSettingBasedScalar::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	// 布尔型动作没法缩放，报错并直接返回原值
	if (ensureMsgf(CurrentValue.GetValueType() != EInputActionValueType::Boolean, TEXT("Setting Based Scalar modifier doesn't support boolean values.")))
	{
		// 拿到本输入所属的本地玩家
		if (UTenkaichiLocalPlayer* LocalPlayer = TenkaichiInputModifiersHelpers::GetLocalPlayer(PlayerInput))
		{
			const UClass* SettingsClass = UTenkaichiSettingsShared::StaticClass();
			UTenkaichiSettingsShared* SharedSettings = LocalPlayer->GetSharedSettings();
			
			// 属性缓存是否已填满（X/Y/Z 三个轴各一个）
			const bool bHasCachedProperty = PropertyCache.Num() == 3;
			
			// 取 X/Y/Z 轴对应的设置属性（有缓存用缓存，没有就按名字现查）
			const FProperty* XAxisValue = bHasCachedProperty ? PropertyCache[0] : SettingsClass->FindPropertyByName(XAxisScalarSettingName);
			const FProperty* YAxisValue = bHasCachedProperty ? PropertyCache[1] : SettingsClass->FindPropertyByName(YAxisScalarSettingName);
			const FProperty* ZAxisValue = bHasCachedProperty ? PropertyCache[2] : SettingsClass->FindPropertyByName(ZAxisScalarSettingName);

			// 缓存为空就填一次，避免每帧重复查反射
			if (PropertyCache.IsEmpty())
			{
				PropertyCache.Emplace(XAxisValue);
				PropertyCache.Emplace(YAxisValue);
				PropertyCache.Emplace(ZAxisValue);
			}

			// 默认缩放全是 1（不缩放）
			FVector ScalarToUse = FVector(1.0, 1.0, 1.0);

			// 按输入是几维（1D/2D/3D），只读对应的轴，把共享设置里那个属性值读出来当缩放
			switch (CurrentValue.GetValueType())
			{
			case EInputActionValueType::Axis3D:
				ScalarToUse.Z = ZAxisValue ? *ZAxisValue->ContainerPtrToValuePtr<double>(SharedSettings) : 1.0;
				//[[fallthrough]];
			case EInputActionValueType::Axis2D:
				ScalarToUse.Y = YAxisValue ? *YAxisValue->ContainerPtrToValuePtr<double>(SharedSettings) : 1.0;
				//[[fallthrough]];
			case EInputActionValueType::Axis1D:
				ScalarToUse.X = XAxisValue ? *XAxisValue->ContainerPtrToValuePtr<double>(SharedSettings) : 1.0;
				break;
			}

			// 缩放值再钳制到配置的最大/最小之间
			ScalarToUse.X = FMath::Clamp(ScalarToUse.X, MinValueClamp.X, MaxValueClamp.X);
			ScalarToUse.Y = FMath::Clamp(ScalarToUse.Y, MinValueClamp.Y, MaxValueClamp.Y);
			ScalarToUse.Z = FMath::Clamp(ScalarToUse.Z, MinValueClamp.Z, MaxValueClamp.Z);
			
			// 返回 输入 × 缩放
			return CurrentValue.Get<FVector>() * ScalarToUse;
		}
	}
	
	return CurrentValue;	
}

//////////////////////////////////////////////////////////////////////
// UTenkaichiInputModifierDeadZone

FInputActionValue UTenkaichiInputModifierDeadZone::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	EInputActionValueType ValueType = CurrentValue.GetValueType();
	UTenkaichiLocalPlayer* LocalPlayer = TenkaichiInputModifiersHelpers::GetLocalPlayer(PlayerInput);
	// 布尔动作或拿不到本地玩家，直接返回
	if (ValueType == EInputActionValueType::Boolean || !LocalPlayer)
	{
		return CurrentValue;
	}
	
	UTenkaichiSettingsShared* Settings = LocalPlayer->GetSharedSettings();
	ensure(Settings);

	// 死区下限：移动摇杆用移动死区设置，视角摇杆用视角死区设置
	float LowerThreshold =
		(DeadzoneStick == EDeadzoneStick::MoveStick) ? 
		Settings->GetGamepadMoveStickDeadZone() :
		Settings->GetGamepadLookStickDeadZone();
	
	// 下限钳到 [0,1]
	LowerThreshold = FMath::Clamp(LowerThreshold, 0.0f, 1.0f);
	
	// 死区加工 lambda：把死区以内抹成 0，超过的部分平移缩放到 ±1 范围
	auto DeadZoneLambda = [LowerThreshold, this](const float AxisVal)
	{
		// 去掉死区后，需要把输入平移缩放到 +/- 1 范围
		return FMath::Min(1.f, (FMath::Max(0.f, FMath::Abs(AxisVal) - LowerThreshold) / (UpperThreshold - LowerThreshold))) * FMath::Sign(AxisVal);
	};

	FVector NewValue = CurrentValue.Get<FVector>();
	// 按死区类型分两种处理
	switch (Type)
	{
	case EDeadZoneType::Axial:
		// 轴向死区：三个轴各按 lambda 独立加工
		NewValue.X = DeadZoneLambda(NewValue.X);
		NewValue.Y = DeadZoneLambda(NewValue.Y);
		NewValue.Z = DeadZoneLambda(NewValue.Z);
		break;
	case EDeadZoneType::Radial:
		// 径向死区：按向量长度加工，再乘回单位方向
		if (ValueType == EInputActionValueType::Axis3D)
		{
			NewValue = NewValue.GetSafeNormal() * DeadZoneLambda(NewValue.Size());
		}
		else if (ValueType == EInputActionValueType::Axis2D)
		{
			NewValue = NewValue.GetSafeNormal2D() * DeadZoneLambda(NewValue.Size2D());
		}
		else
		{
			NewValue.X = DeadZoneLambda(NewValue.X);
		}
		break;
	}

	return NewValue;
}

FLinearColor UTenkaichiInputModifierDeadZone::GetVisualizationColor_Implementation(FInputActionValue SampleValue, FInputActionValue FinalValue) const
{
	// 取自 UInputModifierDeadZone::GetVisualizationColor_Implementation
	if (FinalValue.GetValueType() == EInputActionValueType::Boolean || FinalValue.GetValueType() == EInputActionValueType::Axis1D)
	{
		// 1D/布尔：被死区拦截（结果为 0）显示红，否则黑
		return FLinearColor(FinalValue.Get<float>() == 0.f ? 1.f : 0.f, 0.f, 0.f);
	}
	// 2D：两个分量任一被拦显示红，否则黑
	return FLinearColor((FinalValue.Get<FVector2D>().X == 0.f ? 0.5f : 0.f) + (FinalValue.Get<FVector2D>().Y == 0.f ? 0.5f : 0.f), 0.f, 0.f);
}

//////////////////////////////////////////////////////////////////////
// UTenkaichiInputModifierGamepadSensitivity

FInputActionValue UTenkaichiInputModifierGamepadSensitivity::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	// 布尔动作没法缩放
	UTenkaichiLocalPlayer* LocalPlayer = TenkaichiInputModifiersHelpers::GetLocalPlayer(PlayerInput);
	if (CurrentValue.GetValueType() == EInputActionValueType::Boolean || !LocalPlayer || !SensitivityLevelTable)
	{
		return CurrentValue;
	}
	
	UTenkaichiSettingsShared* Settings = LocalPlayer->GetSharedSettings();
	ensure(Settings);

	// 按瞄准类型读对应灵敏度档位（普通用环视灵敏度，ADS 用瞄准灵敏度）
	const ELyraGamepadSensitivity Sensitivity = (TargetingType == ELyraTargetingType::Normal) ? Settings->GetGamepadLookSensitivityPreset() : Settings->GetGamepadTargetingSensitivityPreset();

	// 灵敏度表把档位翻译成具体数值
	const float Scalar = SensitivityLevelTable->SensitivtyEnumToFloat(Sensitivity);

	return CurrentValue.Get<FVector>() * Scalar;
}

//////////////////////////////////////////////////////////////////////
// UTenkaichiInputModifierAimInversion

FInputActionValue UTenkaichiInputModifierAimInversion::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	UTenkaichiLocalPlayer* LocalPlayer = TenkaichiInputModifiersHelpers::GetLocalPlayer(PlayerInput);
	if (!LocalPlayer)
	{
		return CurrentValue;
	}
	
	UTenkaichiSettingsShared* Settings = LocalPlayer->GetSharedSettings();
	ensure(Settings);

	FVector NewValue = CurrentValue.Get<FVector>();
	
	// 设置里开了垂直反转，Y 轴取反
	if (Settings->GetInvertVerticalAxis())
	{
		NewValue.Y *= -1.0f;
	}
	
	// 设置里开了水平反转，X 轴取反
	if (Settings->GetInvertHorizontalAxis())
	{
		NewValue.X *= -1.0f;
	}
	
	return NewValue;
}

#endif // 依赖 P12 + P18 的挂起结束