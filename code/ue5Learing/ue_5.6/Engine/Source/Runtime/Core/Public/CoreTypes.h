// 版权所有 Epic Games, Inc. 保留所有权利。
// 版权声明，引擎每个源文件第一行都是它，交付时译成中文。

// 头文件保护：保证同一个 .cpp 里多次 include 本文件时只展开一次。
// 比传统的 #ifndef/#define/#endif 更简洁，所有现代编译器都支持。
#pragma once

/*----------------------------------------------------------------------------
	Low level includes.
----------------------------------------------------------------------------*/
/* 分节注释：下面是「底层 include」区。
   这种 /*---*/ 分节注释是 UE 的代码风格，用来把文件切成若干视觉区块。 */

// IWYU pragma: begin_exports
// IWYU（Include What You Use）指令：从这里开始，本文件 include 的所有头
// 都算「本文件对外导出的头」——别的文件 include 了 CoreTypes.h，
// 就等于也 include 了下面这 4 个头。
// 为什么需要：CoreTypes.h 本身不定义任何东西，它只是「转发」，
// 所以必须告诉 IWYU 工具「这些 include 是导出，别给我删掉」。

// 引入平台层总头。
// 真实文件：Core\Public\HAL\Platform.h（1345 行）
// 里面定义：PLATFORM_WINDOWS / PLATFORM_MAC / PLATFORM_ANDROID 等平台宏，
//          以及 int8 / int16 / int32 / int64 / uint8 ... 等基础整数类型。
#include "HAL/Platform.h"

// 引入内存分配器开关定义。
// 真实文件：Core\Public\ProfilingDebugging\UMemoryDefines.h（17 行）
// 里面定义：TBB_ALLOCATOR_ALLOWED / MIMALLOC_ALLOCATOR_ALLOWED
//          （这两个宏决定能不能用 IntelTBB / mimalloc 分配器）。
#include "ProfilingDebugging/UMemoryDefines.h"

// 引入 Core 杂项宏定义。
// 真实文件：Core\Public\Misc\CoreMiscDefines.h（594 行）
// 里面定义：WITH_EDITORONLY_DATA / WITH_METADATA / UE_DISABLE_OPTIMIZATION /
//          UE_ENABLE_OPTIMIZATION / PLAYWORLD_PACKAGE_PREFIX 等构建开关宏。
#include "Misc/CoreMiscDefines.h"

// 引入 Core 基础宏定义。
// 真实文件：Core\Public\Misc\CoreDefines.h（80 行）
// 里面定义：STUBBED / CPP / STRUCTCPP / DEFAULTS / STANDALONE_SEEKFREE_SUFFIX
//          等元数据宏。
#include "Misc/CoreDefines.h"

// IWYU pragma: end_exports
// IWYU 指令：导出区到此结束。