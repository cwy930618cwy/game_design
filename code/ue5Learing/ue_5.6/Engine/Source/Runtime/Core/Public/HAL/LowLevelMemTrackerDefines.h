// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 说明：本头文件根据编译环境里的宏，配置 LLM 的编译期开关。
// 本头文件根据编译环境里的宏，配置 LLM 的编译期开关。
// 说明：本头文件可能被 C 代码包含，所以不能包含任何 C++ 代码或含 C++ 代码的头文件。
// 本头文件可能被 C 代码包含，所以不能包含任何 C++ 代码或含 C++ 代码的头文件。

// 引入构建配置：UE_BUILD_SHIPPING / UE_BUILD_TEST / WITH_ENGINE。
#include "Misc/Build.h"

// 如果没定义 ALLOW_LOW_LEVEL_MEM_TRACKER_IN_TEST（Test 构建是否允许 LLM）。
#ifndef ALLOW_LOW_LEVEL_MEM_TRACKER_IN_TEST
// 默认 1：Test 构建默认启用，方便自动化性能测试里抓 LLM 数据。
	#define ALLOW_LOW_LEVEL_MEM_TRACKER_IN_TEST 1	// Test 构建默认启用，方便自动化性能测试里抓 LLM 数据
// 条件编译结束。
#endif

// 说明：LLM_ENABLED_IN_CONFIG 只能在这里或构建环境里定义。
// LLM_ENABLED_IN_CONFIG 只能在这里或构建环境里定义。
// 说明：不能在平台头文件里定义，因为它会被 C 语言编译单元包含，而那些单元无法包含平台头。
// 不能在平台头文件里定义，因为它会被 C 语言编译单元包含，而那些单元无法包含平台头。
// 说明：本地插桩时，在构建环境或本头文件里自己定义 LLM_ENABLED_IN_CONFIG。
// 本地插桩时，在构建环境或本头文件里自己定义 LLM_ENABLED_IN_CONFIG。
// When locally instrumenting, add your own definition of LLM_ENABLED_IN_CONFIG in build environment or this header.

// 如果没定义 LLM_ENABLED_IN_CONFIG（配置里是否启用 LLM）。
#ifndef LLM_ENABLED_IN_CONFIG 
	#define LLM_ENABLED_IN_CONFIG ( \
		!UE_BUILD_SHIPPING && (!UE_BUILD_TEST || ALLOW_LOW_LEVEL_MEM_TRACKER_IN_TEST) && \
		WITH_ENGINE \
	)
// 条件编译结束。
#endif
