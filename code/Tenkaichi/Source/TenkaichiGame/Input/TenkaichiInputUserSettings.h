// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UserSettings/EnhancedInputUserSettings.h"
#include "PlayerMappableKeySettings.h"

#include "TenkaichiInputUserSettings.generated.h"

#define UE_API TENKAICHIGAME_API

/**
 * Tenkaichi 游戏的自定义输入设置类。
 * 它会在和 Tenkaichi 共享设置相同的时间序列化出来，并且通过调用 "Serialize" 函数兼容云存档。
 */
UCLASS(MinimalAPI)
class UTenkaichiInputUserSettings : public UEnhancedInputUserSettings
{
	GENERATED_BODY()
public:
	//~ UEnhancedInputUserSettings 接口开始
	UE_API virtual void ApplySettings() override;
	//~ UEnhancedInputUserSettings 接口结束

	// 在这里添加任何额外的输入设置！
	// 一些想法可以是：
	// - "切换/按住" 来触发游戏内动作
	// - 瞄准灵敏度应该放在这里
	// - 等等

	// 记得给你的属性标记 "SaveGame" 元数据，这样它们保存时会被序列化
	//UPROPERTY(SaveGame, BlueprintReadWrite, Category="Enhanced Input|User Settings")
	// bool bSomeExampleProperty;
};

/**
 * 按键映射设置：每个"按键动作"都对应一套设置，可存放供设置界面、输入触发器或其他地方使用的附加元数据。
 */
UCLASS(MinimalAPI)
class UTenkaichiPlayerMappableKeySettings : public UPlayerMappableKeySettings
{
	GENERATED_BODY()
	
public:

	// 返回设置界面应该为这个按键显示的提示文字
	UE_API const FText& GetTooltipText() const;

protected:
	// 在设置界面展示时，这个动作要关联的提示文字
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta=(AllowPrivateAccess=true))
	FText Tooltip = FText::GetEmpty();
};

#undef UE_API