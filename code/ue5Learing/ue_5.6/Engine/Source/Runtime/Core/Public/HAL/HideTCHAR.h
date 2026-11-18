// 版权所有 Epic Games, Inc. 保留所有权利。

// 当第三方头文件定义或引用「系统」TCHAR 时，会和我们 Platform.h 里的
// typedef 冲突 —— 前提是编译在 PLATFORM_TCHAR_IS_UTF8CHAR 模式下。
//
// 在任何引用 TCHAR 的第三方头之前 include 本头文件，我们就能强制
// 那些头文件引用那个符号（UE_SYSTEM_TCHAR）。之后应该 include
// "HAL/AllowTCHAR.h"，例如：
//
// #include "HAL/HideTCHAR.h"
// #include <tchar.h>
// #include <other_third_party_header_that_refers_to_tchar.h>
// #include "HAL/AllowTCHAR.h"
//
// 链接不会受这个「覆盖」影响，因为 UE_SYSTEM_TCHAR 类型的名字修饰
// （mangling）方式相同。
//
// 预处理器的 token 粘贴（##）会受影响，但这种情况很少见，需要专门处理。
//
// 如果将来需要故意引用系统 TCHAR 而不是 Platform.h 里的 TCHAR，
// 可以直接用 UE_SYSTEM_TCHAR 类型。
// （原文续行）

// HEADER_UNIT_SKIP 标记：不直接单独 include，只能成对使用。
// HEADER_UNIT_SKIP - Not included directly

// 如果还没定义过 UE_HIDETCHAR_INCLUDED（即第一次 include）。
#ifndef UE_HIDETCHAR_INCLUDED
// 打上「已 include」标记。
	#define UE_HIDETCHAR_INCLUDED
// 否则（重复 include 了）。
#else
// 编译报错：HideTCHAR.h 被重复 include，多半是漏了 AllowTCHAR.h。
	#error "HAL/HideTCHAR.h is being multiply included - an include of HAL/AllowTCHAR.h is likely missing."
// 条件编译结束。
#endif

// 如果平台把 TCHAR 定义成 UTF8CHAR（TCHAR 不是宽字符）。
#if PLATFORM_TCHAR_IS_UTF8CHAR
	// 覆盖 TCHAR 这个名字。
// 把 TCHAR 这个名字重定向到 UE_SYSTEM_TCHAR（藏起来）。
	#define TCHAR UE_SYSTEM_TCHAR
// 条件编译结束。
#endif
