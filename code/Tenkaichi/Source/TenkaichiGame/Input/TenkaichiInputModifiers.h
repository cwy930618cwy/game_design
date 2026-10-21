// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "InputModifiers.h"

#include "UObject/UnrealType.h"
#include "TenkaichiInputModifiers.generated.h"

struct FInputActionValue;

class FProperty;
class UEnhancedPlayerInput;
class UTenkaichiAimSensitivityData;
class UObject;

// 按设置属性缩放输入值（XYZ 三轴分别用一个设置属性名控制）
UCLASS(NotBlueprintable, MinimalAPI, meta = (DisplayName = "Setting Based Scalar"))
class UTenkaichiSettingBasedScalar : public UInputModifier
{
	GENERATED_BODY()

public:

	// 用来缩放 X 轴的设置属性名
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings)
	FName XAxisScalarSettingName = NAME_None;

	// 用来缩放 Y 轴的设置属性名
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings)
	FName YAxisScalarSettingName = NAME_None;

	// 用来缩放 Z 轴的设置属性名
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings)
	FName ZAxisScalarSettingName = NAME_None;

	// 设置值的缩放上限（XYZ 三轴）
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings)
	FVector MaxValueClamp = FVector(10.0, 10.0, 10.0);

	// 设置值的缩放下限（XYZ 三轴）
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings)
	FVector MinValueClamp = FVector::ZeroVector;

protected:
	// 加工输入值：读设置属性，缩放各轴，再限制在最大/最小之间
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

	// 找到的设置属性缓存（首次查到后存起来，避免每帧重复查）
	TArray<const FProperty*> PropertyCache;
};

// 死区属于哪个摇杆（移动杆 or 视角杆）
UENUM()
enum class EDeadzoneStick : uint8
{
	/** 移动摇杆的死区 */
	MoveStick = 0,

	/** 视角摇杆的死区 */
	LookStick = 1,
};

// 死区修饰器：阈值由共享设置里的摇杆死区驱动
UCLASS(NotBlueprintable, MinimalAPI, meta = (DisplayName = "Lyra Settings Driven Dead Zone"))
class UTenkaichiInputModifierDeadZone : public UInputModifier
{
	GENERATED_BODY()

public:

	// 死区类型（径向/轴向，引擎定义的枚举）
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings, Config)
	EDeadZoneType Type = EDeadZoneType::Radial;

	// 输入超过该阈值就钳制为 1
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings, Config)
	float UpperThreshold = 1.0f;

	// 这个死区属于哪个摇杆（决定用哪个设置里的死区值）
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings, Config)
	EDeadzoneStick DeadzoneStick = EDeadzoneStick::MoveStick;

protected:
	// 加工输入值：把死区以内的输入抹成 0，超过的部分平移缩放到 ±1
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

	// 可视化颜色（编辑器里预览死区效果：未修改显示黑色，被拦截显示红色）
	virtual FLinearColor GetVisualizationColor_Implementation(FInputActionValue SampleValue, FInputActionValue FinalValue) const override;
};

// 瞄准类型（平时环视 / 开镜瞄准）
UENUM()
enum class ELyraTargetingType : uint8
{
	/** 平时环视时用的灵敏度 */
	Normal = 0,

	/** 开镜瞄准（ADS）时用的灵敏度 */
	ADS = 1,
};

// 手柄灵敏度修饰器：根据当前手柄灵敏度档位，查灵敏度表拿到数值去缩放输入
UCLASS(NotBlueprintable, MinimalAPI, meta = (DisplayName = "Lyra Gamepad Sensitivity"))
class UTenkaichiInputModifierGamepadSensitivity : public UInputModifier
{
	GENERATED_BODY()
public:

	// 用于哪种瞄准类型（普通/ADS），决定用哪个灵敏度档位
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings, Config)
	ELyraTargetingType TargetingType = ELyraTargetingType::Normal;

	// 灵敏度表资产（P03.2-1 的 AimSensitivityData）：提供「档位 → 数值」的查询
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AssetBundles="Client,Server"))
	TObjectPtr<const UTenkaichiAimSensitivityData> SensitivityLevelTable;

protected:
	// 加工输入值：查灵敏度表拿到数值，缩放输入
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;
};

// 瞄准反转修饰器：根据设置里的反转开关，翻转输入轴的正负
UCLASS(NotBlueprintable, MinimalAPI, meta = (DisplayName = "Lyra Aim Inversion Setting"))
class UTenkaichiInputModifierAimInversion : public UInputModifier
{
	GENERATED_BODY()

protected:
	// 加工输入值：按设置里的反转开关翻转 X/Y 轴
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;
};