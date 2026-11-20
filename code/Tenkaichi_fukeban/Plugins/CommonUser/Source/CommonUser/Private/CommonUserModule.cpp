// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonUserModule.h"

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FCommonUserModule"

void FCommonUserModule::StartupModule()
{
	// 这段代码会在你的模块加载到内存后执行；具体时机在 .uplugin 文件里按模块指定
}

void FCommonUserModule::ShutdownModule()
{
	// 此函数可能在关闭过程中被调用以清理你的模块。对于支持动态重载的模块，
	// 我们会在卸载模块之前调用此函数。
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCommonUserModule, CommonUser)