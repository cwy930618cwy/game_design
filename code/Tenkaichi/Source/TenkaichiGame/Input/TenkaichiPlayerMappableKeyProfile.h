// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UserSettings/EnhancedInputUserSettings.h"

#include "TenkaichiPlayerMappableKeyProfile.generated.h"

#define UE_API TENKAICHIGAME_API

// 自定义按键映射配置：重写装备/卸载两个钩子，可插入自定义逻辑
UCLASS(MinimalAPI)
class UTenkaichiPlayerMappableKeyProfile : public UEnhancedPlayerMappableKeyProfile
{
	GENERATED_BODY()

protected:

	//~ UEnhancedPlayerMappableKeyProfile 接口开始
	UE_API virtual void EquipProfile() override;
	UE_API virtual void UnEquipProfile() override;
	//~ UEnhancedPlayerMappableKeyProfile 接口结束
};

#undef UE_API