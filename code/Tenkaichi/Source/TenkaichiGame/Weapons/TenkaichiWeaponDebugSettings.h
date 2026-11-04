// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"

#include "TenkaichiWeaponDebugSettings.generated.h"

class UObject;

/**
 * 武器调试设置（开发用）
 */
UCLASS(config=EditorPerProjectUserSettings)
class UTenkaichiWeaponDebugSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UTenkaichiWeaponDebugSettings();

	//~UDevelopmentSettings 接口开始
	virtual FName GetCategoryName() const override;
	//~UDevelopmentSettings 接口结束

public:
	// 是否绘制子弹轨迹调试（如果大于 0，则设置绘制时长（秒））
	UPROPERTY(config, EditAnywhere, Category=General, meta=(ConsoleVariable="tenkaichi.Weapon.DrawBulletTraceDuration", ForceUnits=s))
	float DrawBulletTraceDuration;

	// 是否绘制子弹命中点调试（如果大于 0，则设置绘制时长（秒））
	UPROPERTY(config, EditAnywhere, Category = General, meta = (ConsoleVariable = "tenkaichi.Weapon.DrawBulletHitDuration", ForceUnits = s))
	float DrawBulletHitDuration;

	// 启用子弹命中点调试绘制时（见 DrawBulletHitDuration），命中点圆的半径大小？（单位：厘米）
	UPROPERTY(config, EditAnywhere, Category = General, meta = (ConsoleVariable = "tenkaichi.Weapon.DrawBulletHitRadius", ForceUnits = cm))
	float DrawBulletHitRadius;
};