// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiAimSensitivityData.h"

// #include "Settings/TenkaichiSettingsShared.h"   // ⚠️【注释挂起】P18 Settings 未就位，工程无此文件 → 编译不过（见注释备忘录 ⑰）

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAimSensitivityData)

// ⚠️【注释挂起】构造函数体 + 查询函数体 + include 依赖 P18 Settings（完整枚举定义），当前未就位，用 #if 0 挂起
//（代码已一比一还原 Lyra，P18 Settings 就位后放开即可；已登记注释备忘录 ⑰）
#if 0

UTenkaichiAimSensitivityData::UTenkaichiAimSensitivityData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 灵敏度档位 → 数值 的默认映射表（档位名来自 P18 的 ETenkaichiGamepadSensitivity 枚举）
	SensitivityMap =
	{
		{ ELyraGamepadSensitivity::Slow,				0.5f },
		{ ELyraGamepadSensitivity::SlowPlus,			0.75f },
		{ ELyraGamepadSensitivity::SlowPlusPlus,	0.9f },
		{ ELyraGamepadSensitivity::Normal,			1.0f },
		{ ELyraGamepadSensitivity::NormalPlus,		1.1f },
		{ ELyraGamepadSensitivity::NormalPlusPlus,	1.25f },
		{ ELyraGamepadSensitivity::Fast,				1.5f },
		{ ELyraGamepadSensitivity::FastPlus,			1.75f },
		{ ELyraGamepadSensitivity::FastPlusPlus,	2.0f },
		{ ELyraGamepadSensitivity::Insane,			2.5f },
	};
}

// 查询：根据灵敏度档位枚举，返回对应的灵敏度数值；找不到返回默认 1.0f
const float UTenkaichiAimSensitivityData::SensitivtyEnumToFloat(const ELyraGamepadSensitivity InSensitivity) const
{
	if (const float* Sens = SensitivityMap.Find(InSensitivity))
	{
		return *Sens;
	}

	return 1.0f;
}

#endif // #if 0 挂起结束（P18 Settings 就位后解开）