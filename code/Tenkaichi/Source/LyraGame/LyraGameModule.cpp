// 版权声明：Epic Games 保留所有权利

// 引入模块管理头文件；FDefaultGameModuleImpl 和主模块宏都在这里
#include "Modules/ModuleManager.h"


// 游戏主模块类；空实现即可，生命周期函数按需重写
class FLyraGameModule : public FDefaultGameModuleImpl
// 类体开始
{
	// 模块启动时由引擎调用；可在这里注册委托 / 初始化子系统
	virtual void StartupModule() override
	// 启动函数体开始
	{
		// 启动函数体结束（当前无需初始化）
	}

	// 模块关闭时由引擎调用；与启动成对，做清理
	virtual void ShutdownModule() override
	// 关闭函数体开始
	{
		// 关闭函数体结束（当前无需清理）
	}
	// 类体结束
};

// 注册主游戏模块；把实现类 FLyraGameModule 与模块名 LyraGame 绑定
IMPLEMENT_PRIMARY_GAME_MODULE(FLyraGameModule, LyraGame, "LyraGame");