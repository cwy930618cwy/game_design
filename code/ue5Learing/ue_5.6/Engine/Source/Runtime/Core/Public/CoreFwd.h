// 版权所有 Epic Games, Inc. 保留所有权利。
// 版权声明，引擎每个源文件第一行都是它，交付时译成中文。

// 头文件保护：保证同一个 .cpp 里多次 include 本文件时只展开一次。
#pragma once

// 引入基础类型层（平台宏 + 基础整数类型 + 构建开关宏）。
// 真实文件：Core\Public\CoreTypes.h（15 行）
// 为什么需要：下面的前置声明用到 uint8（第 43、44 行），必须先从 CoreTypes.h 拿到。
#include "CoreTypes.h"

// 引入容器前置声明。
// 真实文件：Core\Public\Containers\ContainersFwd.h（77 行）
// 里面声明：FString / FAnsiString / FUtf8String / FDefaultAllocator 等。
// IWYU pragma: export = 这个 include 算「本文件导出」，别被 IWYU 工具删掉。
#include "Containers/ContainersFwd.h" // IWYU pragma: export

// 引入数学前置声明。
// 真实文件：Core\Public\Math\MathFwd.h（143 行）
// 里面声明：FColor / FLinearColor / UE::Math::TVector<T> / TQuat<T> / TMatrix<T> 等。
#include "Math/MathFwd.h" // IWYU pragma: export

// 引入 UObject 层级前置声明。
// 真实文件：Core\Public\UObject\UObjectHierarchyFwd.h（56 行）
// 里面声明：UObject / UClass / UStruct / UFunction / UEnum / FProperty 等。
#include "UObject/UObjectHierarchyFwd.h" // IWYU pragma: export

// Basic types
// 分节注释：下面是「基础类型」的前置声明。

// 前置声明 FName 类（名字系统，全局唯一字符串 ID）。
// 为什么前置声明：很多头文件只需要 FName& 或 FName*，不需要完整定义。
class FName;

// 前置声明 FExec 类（控制台命令执行器）。
class FExec;

// 前置声明 FArchive 类（序列化归档，读写二进制数据的抽象基类）。
class FArchive;

// 前置声明 FOutputDevice 类（输出设备，日志输出的抽象基类）。
class FOutputDevice;

// 前置声明 FFeedbackContext 类（反馈上下文，慢任务进度条 / 取消按钮）。
class FFeedbackContext;

// 前置声明 FDateTime 结构体（日期时间）。
struct FDateTime;

// 前置声明 FGuid 结构体（全局唯一标识符，16 字节）。
struct FGuid;

// Math - See Math/MathFwd.h
// 分节注释：数学类型的前置声明在 Math/MathFwd.h 里（上面第 7 行已 include）。

// Misc
// 分节注释：下面是「杂项」的前置声明。

// 前置声明 FResourceSizeEx 结构体（资源大小统计，扩展版）。
struct FResourceSizeEx;

// 前置声明 IConsoleVariable 类（控制台变量接口）。
class IConsoleVariable;

// 前置声明 FRunnableThread 类（可运行线程）。
class FRunnableThread;

// 前置声明 FEvent 类（线程同步事件）。
class FEvent;

// 前置声明 IPlatformFile 类（平台文件系统接口）。
class IPlatformFile;

// 前置声明 FMalloc 类（内存分配器基类）。
class FMalloc;

// 前置声明 IFileHandle 类（文件句柄接口）。
class IFileHandle;

// 前置声明 FAutomationTestBase 类（自动化测试基类）。
class FAutomationTestBase;

// 前置声明 FGenericMemoryStats 结构体（通用内存统计）。
struct FGenericMemoryStats;

// 前置声明 FSHAHash 类（SHA 哈希值）。
class FSHAHash;

// 前置声明 FScriptArray 类（脚本数组，蓝图用的动态数组）。
class FScriptArray;

// 前置声明 FThreadSafeCounter 类（线程安全计数器）。
class FThreadSafeCounter;

// 前置声明 EModuleChangeReason 枚举类（模块变化原因）。
// enum class = 强类型枚举（C++11），必须写 enum class 而不是 enum。
enum class EModuleChangeReason;

// 前置声明 FManifestContext 结构体（清单上下文）。
struct FManifestContext;

// 前置声明 IConsoleObject 类（控制台对象接口，IConsoleVariable 的基类）。
class IConsoleObject;

// 前置声明 FConfigFile 类（配置文件）。
class FConfigFile;

// 前置声明 FConfigSection 结构体（配置文件的一个节）。
class FConfigSection;

// Text
// 分节注释：下面是「文本」的前置声明。

// 前置声明 FText 类（本地化文本，带文化信息的字符串）。
class FText;

// 前置声明 FTextFilterString 类（文本过滤字符串）。
class FTextFilterString;

// 前置声明 ETextFilterTextComparisonMode 枚举类，底层类型 uint8。
// 注意：这里写了「: uint8」指定底层类型，因为前置声明枚举时必须指定底层类型。
// uint8 来自 CoreTypes.h（第 5 行 include）。
enum class ETextFilterTextComparisonMode : uint8;

// 前置声明 ETextFilterComparisonOperation 枚举类，底层类型 uint8。
enum class ETextFilterComparisonOperation : uint8;