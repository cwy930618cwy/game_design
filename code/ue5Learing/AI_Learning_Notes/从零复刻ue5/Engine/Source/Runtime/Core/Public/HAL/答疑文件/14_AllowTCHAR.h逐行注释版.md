# 14 — `HAL/AllowTCHAR.h` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\AllowTCHAR.h`（**28 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\AllowTCHAR.h`
> **⚠️ 本节不能独立编译**——必须与 `HAL/HideTCHAR.h` 成对 include。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。

---

## 一、这个文件解决什么问题（为什么）

第三方库（比如 `<tchar.h>`、某些 SDK 头）里会用到 `TCHAR` 这个名字。UE 在 `PLATFORM_TCHAR_IS_UTF8CHAR` 模式下把 `TCHAR` 定义成 `char`（UTF-8），而 Windows 系统头把 `TCHAR` 定义成 `wchar_t` —— **两者撞名**，编译直接报重定义。

UE 的解法是「**临时把 TCHAR 藏起来**」：

```cpp
#include "HAL/HideTCHAR.h"        // 把 TCHAR 改名成 UE_SYSTEM_TCHAR（藏起来）
#include <tchar.h>                // 第三方头看到的是 UE_SYSTEM_TCHAR，不冲突
#include "HAL/AllowTCHAR.h"       // 把 TCHAR 还回来（恢复成 UE 的定义）
```

`AllowTCHAR.h` 就是**「还回来」的那一半**：它撤销 `HideTCHAR.h` 的 hack，并把系统原生 TCHAR 记成 `UE_SYSTEM_TCHAR` 备用。

**说个场景**：你接入一个第三方图像库，它的头文件里有 `TCHAR szPath[MAX_PATH];`。直接 include 会和 UE 的 `TCHAR` 冲突。解法就是上面那三行包裹。

> 🔑 **一句话**：`AllowTCHAR.h` = **TCHAR 的「解封开关」**，必须和 `HideTCHAR.h` 成对使用，单独用会 `#error`。

---

## 二、逐段代码（每行上方都有中文注释）

### 一、版权头 + 使用说明（第 1~6 行）

```cpp
// 版权所有 Epic Games, Inc. 保留所有权利。

// 本头文件预期在 HAL/HideTCHAR.h 之后被 include，详见那个头文件里的注释。
// 原文第二行：for more details.

// HEADER_UNIT_SKIP 标记：不直接单独 include，只能成对使用。
```

**本段新元素**

- `HEADER_UNIT_SKIP` —— 标记本文件不单独作为编译单元
- 说明：必须在 `HAL/HideTCHAR.h` 之后 include

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1 行 | 版权头 |
| 第 3~4 行 | 使用说明：必须跟在 HideTCHAR.h 后 |
| 第 6 行 | HEADER_UNIT_SKIP 标记 |

> 类比：像药盒上的「请与另一半配合服用」：单独吃没用。

### 二、配对校验（第 8~12 行）

```cpp
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
```

**本段新元素**

- `UE_HIDETCHAR_INCLUDED` —— `HideTCHAR.h` 打下的标记
- `#undef` —— 取消宏定义
- `#error "..."` —— 编译期直接报错并输出消息

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 8 行 | 检查标记是否存在 |
| 第 9 行 | 存在 → 取消标记（解封） |
| 第 11 行 | 不存在 → 编译报错 |

> 类比：像验票：没买 HideTCHAR 的票，就不让进 AllowTCHAR 的门。

### 三、首次 include：记下系统 TCHAR（第 14~24 行）

```cpp
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
```

**本段新元素**

- `UE_HIDETCHAR_INCLUDED_FIRST` —— 「已首次 include」标记
- `using UE_SYSTEM_TCHAR = TCHAR;` —— 把当前 TCHAR 类型记成 UE_SYSTEM_TCHAR
- `!PLATFORM_TCHAR_IS_UTF8CHAR` —— 平台 TCHAR 不是 UTF8CHAR 的模式

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 14~15 行 | 首次 include 才执行 |
| 第 17~22 行 | 说明注释 |
| 第 23 行 | 记下系统 TCHAR |

> 类比：像拍照存档：在把 TCHAR 还回去之前，先拍一张「系统原生 TCHAR 长什么样」。

### 四、UTF8CHAR 模式下撤销宏（第 26~28 行）

```cpp
// 如果平台把 TCHAR 定义成 UTF8CHAR，即 TCHAR 不是宽字符。
#if PLATFORM_TCHAR_IS_UTF8CHAR
// 取消 TCHAR 的宏定义，恢复成 Platform.h 里的定义。
	#undef TCHAR
// 条件编译结束。
#endif
```

**本段新元素**

- `PLATFORM_TCHAR_IS_UTF8CHAR` —— 平台把 TCHAR 定义成 UTF8CHAR
- `#undef TCHAR` —— 撤销 `HideTCHAR.h` 里的 `#define TCHAR UE_SYSTEM_TCHAR`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 26 行 | 判断是否 UTF8CHAR 模式 |
| 第 27 行 | 撤销 TCHAR 宏 |
| 第 28 行 | 条件编译结束 |

> 类比：像把借出去的名字收回来：第三方头用完了，TCHAR 这个名字还给 UE。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 版权头 + 使用说明 | 1~6 | 版权头 + 使用说明 |
| 配对校验 | 8~12 | 配对校验 |
| 首次 include：记下系统 TCHAR | 14~24 | 首次 include：记下系统 TCHAR |
| UTF8CHAR 模式下撤销宏 | 26~28 | UTF8CHAR 模式下撤销宏 |

---

## 四、C++ / 预处理器语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `#ifdef X` | 如果宏 X 已定义 | `#ifdef UE_HIDETCHAR_INCLUDED` |
| `#if !defined(X)` | 如果宏 X 未定义 | `#if !defined(UE_HIDETCHAR_INCLUDED_FIRST)` |
| `#undef X` | 取消宏 X 的定义 | `#undef TCHAR` |
| `#define X` | 定义宏 X（无值） | `#define UE_HIDETCHAR_INCLUDED_FIRST` |
| `#error "msg"` | 编译期报错并输出 msg | `#error "HAL/AllowTCHAR.h included without..."` |
| `using A = B;` | 类型别名（C++11） | `using UE_SYSTEM_TCHAR = TCHAR;` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 28 行 | 49 行 | 多出的是每行上方的中文注释 |
| 非注释代码行 | 12 行 | 12 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| `#error` 消息 | 英文原文 | 保留英文 | 编译期输出，属运行时字符串，不翻译 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\AllowTCHAR.h`（49 行）
2. 脚本验收：`codeOurs=12 / codeEngine=12 / diff=0`
3. 确认后下一个：`HAL\ConsoleManager.h`（333 字节）

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\AllowTCHAR.h`（28 行）
- 配套：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\HideTCHAR.h`（35 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\AllowTCHAR.h`（49 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\01_AllowTCHAR.h_总md.md`