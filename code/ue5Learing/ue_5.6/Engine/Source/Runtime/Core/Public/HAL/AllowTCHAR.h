// 版权所有 Epic Games, Inc. 保留所有权利。

// 本头文件预期在 HAL/HideTCHAR.h 之后被 include，详见那个头文件里的注释。
// 原文第二行：for more details.

// HEADER_UNIT_SKIP 标记：不直接单独 include，只能成对使用。

// 如果 UE_HIDETCHAR_INCLUDED 已定义，说明前面确实 include 过 HideTCHAR.h。
#ifdef UE_HIDETCHAR_INCLUDED
// 取消该标记，表示 TCHAR 的隐藏期结束。
	#undef UE_HIDETCHAR_INCLUDED
// 否则，没 include 过 HideTCHAR.h 就来了。
#else
// 直接编译报错：AllowTCHAR.h 必须与 HideTCHAR.h 成对使用。
	#error "HAL/AllowTCHAR.h included without a corresponding include of HAL/AllowTCHAR.h - check your includes."
// 条件编译结束。
#endif

// 如果还没定义过 UE_HIDETCHAR_INCLUDED_FIRST，即这是第一次成对 include。
#if !defined(UE_HIDETCHAR_INCLUDED_FIRST)
// 打上已首次 include 标记，保证下面这段只执行一次。
	#define UE_HIDETCHAR_INCLUDED_FIRST

	// 我们希望 UE_SYSTEM_TCHAR 在 TCHAR 第一次被定义之后就存在，即使在
	// !PLATFORM_TCHAR_IS_UTF8CHAR 模式下也要有。
	//
	// TCHAR 预期会在 HideTCHAR/AllowTCHAR 这对包裹的第一批 include 里被定义，
	// 所以我们在撤销 hack 之前，先用它来定义我们自己的类型，如果它存在的话。
	//
// 把当前的 TCHAR 类型记成 UE_SYSTEM_TCHAR，即系统原生 TCHAR。
	using UE_SYSTEM_TCHAR = TCHAR;
// 条件编译结束。
#endif

// 如果平台把 TCHAR 定义成 UTF8CHAR，即 TCHAR 不是宽字符。
#if PLATFORM_TCHAR_IS_UTF8CHAR
// 取消 TCHAR 的宏定义，恢复成 Platform.h 里的定义。
	#undef TCHAR
// 条件编译结束。
#endif
