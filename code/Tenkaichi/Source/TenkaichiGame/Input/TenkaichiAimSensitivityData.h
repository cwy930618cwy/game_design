// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "TenkaichiAimSensitivityData.generated.h"

#define UE_API TENKAICHIGAME_API

// 手柄灵敏度档位枚举（前置声明：完整定义在 Settings/TenkaichiSettingsShared.h，P18 才教）
enum class ELyraGamepadSensitivity : uint8;

class UObject;

// 定义一组「手柄灵敏度档位 → 数值」的映射
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "Tenkaichi Aim Sensitivity Data", ShortTooltip = "Data asset used to define a map of Gamepad Sensitivty to a float value."))
class UTenkaichiAimSensitivityData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UE_API UTenkaichiAimSensitivityData(const FObjectInitializer& ObjectInitializer);

	// 查询：根据灵敏度档位枚举，返回对应的灵敏度数值
	UE_API const float SensitivtyEnumToFloat(const ELyraGamepadSensitivity InSensitivity) const;

protected:
	// 灵敏度档位 → 数值 的映射表（默认值由构造函数填充）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ELyraGamepadSensitivity, float> SensitivityMap;
};

#undef UE_API