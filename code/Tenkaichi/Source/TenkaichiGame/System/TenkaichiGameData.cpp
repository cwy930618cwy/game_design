// Copyright Epic Games, Inc. All Rights Reserved.

// 先包含自己的 .h（里面声明了构造 + Get）
#include "TenkaichiGameData.h"

#if 0 // ⚠️ 挂起：依赖 P14 AssetManager，P14 就位后解开（登记注释备忘录⑮）
// 资产管理器：Get() 从这里取全局游戏数据（P14 System 收口类，暂注释挂起）
#include "TenkaichiAssetManager.h"
#endif

// UHT 生成的 .cpp 实现（对应 TenkaichiGameData.generated.h）
#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameData)

// 构造函数：默认实现（数据成员靠资产编辑器配置，这里不需要初始化逻辑）
UTenkaichiGameData::UTenkaichiGameData()
{
}

#if 0 // ⚠️ 挂起：依赖 P14 AssetManager，P14 就位后解开（登记注释备忘录⑮）
// 返回已加载的全局游戏数据（通过资产管理器取，全局只有一份）
const UTenkaichiGameData& UTenkaichiGameData::Get()
{
	return UTenkaichiAssetManager::Get().GetGameData();
}
#endif