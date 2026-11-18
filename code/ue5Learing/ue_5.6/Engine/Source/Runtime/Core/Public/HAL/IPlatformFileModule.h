// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入基础类型层：TCHAR 等基础类型。
#include "CoreTypes.h"
// 引入 Core 前置声明：IPlatformFile 等类型的前置声明。
#include "CoreFwd.h"
// 引入模块接口：IModuleInterface（本类的基类）。
#include "Modules/ModuleInterface.h"

// 说明：平台文件模块接口。
/**
 * 平台文件模块接口
 */
// IPlatformFileModule：平台文件模块接口，继承 IModuleInterface。
class IPlatformFileModule
// 继承模块接口基类。
	: public IModuleInterface
{
// 公有区。
public:

// 说明：创建一个平台文件实例。
	/**
	 * 创建一个平台文件实例。
	 *
// 返回：平台文件实例。
	 * @return 平台文件实例。
	 */
// GetPlatformFile：纯虚，取平台文件实例。
	virtual IPlatformFile* GetPlatformFile() = 0;
// 类结束。
};
