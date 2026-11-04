// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiWeaponDebugSettings.h"
#include "Misc/App.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiWeaponDebugSettings)

UTenkaichiWeaponDebugSettings::UTenkaichiWeaponDebugSettings()
{
}

FName UTenkaichiWeaponDebugSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}