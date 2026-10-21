// Copyright Epic Games, Inc. All Rights Reserved.

#include "Input/TenkaichiPlayerMappableKeyProfile.h"

// 按键映射配置被装备时的钩子
void UTenkaichiPlayerMappableKeyProfile::EquipProfile()
{
	Super::EquipProfile();

	// 在这里做任何你想要在"新按键配置被装备"时做的事
}

// 按键映射配置被卸载时的钩子
void UTenkaichiPlayerMappableKeyProfile::UnEquipProfile()
{
	Super::UnEquipProfile();
	
	// 在这里做任何你想要在"按键配置被卸载"时做的事
}