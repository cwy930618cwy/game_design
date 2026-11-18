// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入基础类型层：TCHAR / bool 都来自这里。
#include "CoreTypes.h"
// 引入字符串转换：TCHAR 与 ANSI/UTF8 互转。
#include "Containers/StringConv.h"
// 引入 FString：字符串类型。
#include "Containers/UnrealString.h"
// 引入全局量：GLog / GWarn 等。
#include "CoreGlobals.h"
// 引入 FApp：应用级信息。
#include "Misc/App.h"
// 引入控制台输出设备：FOutputDeviceConsole。
#include "Misc/OutputDeviceConsole.h"
// 引入输出设备辅助：日志格式化工具。
#include "Misc/OutputDeviceHelper.h"
// 引入输出重定向器：FOutputDeviceRedirector。
#include "Misc/OutputDeviceRedirector.h"
// 引入反馈上下文基类：FFeedbackContext。
#include "Misc/FeedbackContext.h"

// FFeedbackContextAnsi：ANSI 控制台风格的反馈上下文，继承 FFeedbackContext。
class FFeedbackContextAnsi : public FFeedbackContext
{
// 公有区。
public:
// Context：当前上下文供应器（谁在提供「正在做什么」的描述）。
	FContextSupplier*	Context = nullptr;
// AuxOut：辅助输出设备（额外往哪写日志）。
	FOutputDevice*		AuxOut = nullptr;

// LocalPrint：直接打印到本地控制台（静态方法）。
	CORE_API static void LocalPrint(const TCHAR* Str);

// Serialize：输出一条日志（重写基类虚函数）。
	CORE_API void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;
// Serialize：带时间戳的输出重载。
	CORE_API void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category, double Time) override;
// SerializeRecord：输出一条结构化日志记录。
	CORE_API void SerializeRecord(const UE::FLogRecord& Record) override;

// YesNof：弹「是/否」询问（重写基类虚函数）。
	CORE_API bool YesNof(const FText& Question) override;

// GetContext：取当前上下文供应器（内联实现）。
	FContextSupplier* GetContext() const override { return Context; }
// SetContext：设置上下文供应器（内联实现）。
	void SetContext(FContextSupplier* InContext) override { Context = InContext; }

// 私有区。
private:
// IsUsingLocalPrint：判断是否走本地打印路径。
	bool IsUsingLocalPrint() const;
};
