// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 只是转手 include 接口头文件：如果所有 include 都改成 IConsoleManager，本文件可以删掉；
// 或者把它挪进 Private，再把 FConsoleManager 类放回来。
#include "HAL/IConsoleManager.h" // IWYU pragma: export（IWYU 指令：本头文件对外导出它）
