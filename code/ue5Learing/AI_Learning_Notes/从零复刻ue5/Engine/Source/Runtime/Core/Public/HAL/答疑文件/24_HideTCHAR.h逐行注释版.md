# 24 — `HAL/HideTCHAR.h` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\HideTCHAR.h`（**35 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\HideTCHAR.h`
> **⚠️ 本节不能独立编译**——必须与 `HAL/AllowTCHAR.h` 成对 include。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。

---

## 一、这个文件解决什么问题（为什么）

这是 `AllowTCHAR.h` 的**另一半**。上一节讲过：第三方库用 `TCHAR` 会和 UE 的 `TCHAR` 撞名。`HideTCHAR.h` 负责**「藏起来」**，`AllowTCHAR.h` 负责**「还回来」**。

**完整用法**（引擎注释第 10~13 行给了示例）：

```cpp
#include "HAL/HideTCHAR.h"                              // ① 把 TCHAR 藏起来
#include <tchar.h>                                      // ② 第三方头（看到的是 UE_SYSTEM_TCHAR）
#include <other_third_party_header_that_refers_to_tchar.h>
#include "HAL/AllowTCHAR.h"                             // ③ 把 TCHAR 还回来
```

**它做两件事**：

| 动作 | 代码 | 作用 |
|---|---|---|
| **防重复 include** | `#ifndef UE_HIDETCHAR_INCLUDED` / `#else #error` | 重复 include 直接编译报错（提示漏了 `AllowTCHAR.h`） |
| **藏 TCHAR** | `#define TCHAR UE_SYSTEM_TCHAR` | 只在 `PLATFORM_TCHAR_IS_UTF8CHAR` 模式下生效 |

**为什么只在 UTF8CHAR 模式下藏**：因为只有这个模式下 UE 的 `TCHAR` 是 `char`（UTF-8），才会和 Windows 的 `wchar_t` 撞名。其他模式下 `TCHAR` 本来就是 `wchar_t`，不冲突。

**链接为什么不受影响**（引擎注释第 15~16 行）：`UE_SYSTEM_TCHAR` 和系统 `TCHAR` 是**同一个类型**（只是名字不同），C++ 名字修饰（mangling）结果一样，所以链接器看到的是同一个符号。

> 🔑 **一句话**：`HideTCHAR.h` = **TCHAR 的「封印开关」**，把 `TCHAR` 这个名字临时改指向 `UE_SYSTEM_TCHAR`，用完必须配 `AllowTCHAR.h` 解封。

---

## 二、逐段代码（每行上方都有中文注释）

### 一、版权头 + 20 行使用说明（第 1~22 行）

```cpp
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
```

**本段新元素**

- 说明：第三方头定义/引用「系统」TCHAR 会和 `Platform.h` 的 typedef 冲突
- 说明：冲突前提是 `PLATFORM_TCHAR_IS_UTF8CHAR` 模式
- 说明：在第三方头之前 include 本文件，强制它们引用 `UE_SYSTEM_TCHAR`
- 说明：之后必须 include `HAL/AllowTCHAR.h`
- 说明：链接不受影响（`UE_SYSTEM_TCHAR` 名字修饰相同）
- 说明：预处理器 token 粘贴（`##`）会受影响，但很少见
- 说明：要故意引用系统 TCHAR 就用 `UE_SYSTEM_TCHAR`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1 行 | 版权头 |
| 第 3~4 行 | 问题描述：TCHAR 撞名 |
| 第 6~8 行 | 解法：先 include 本文件 |
| 第 10~13 行 | 完整用法示例（4 行代码） |
| 第 15~16 行 | 链接为什么不受影响 |
| 第 18~19 行 | token 粘贴的例外 |
| 第 21~22 行 | 如何故意引用系统 TCHAR |

> 类比：像说明书：先讲为什么，再讲怎么用，最后讲注意事项。

### 二、`HEADER_UNIT_SKIP` 标记（第 24~24 行）

```cpp
// HEADER_UNIT_SKIP 标记：不直接单独 include，只能成对使用。
// HEADER_UNIT_SKIP - Not included directly
```

**本段新元素**

- `HEADER_UNIT_SKIP` —— 标记本文件不单独作为编译单元

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 24 行 | HEADER_UNIT_SKIP 标记 |

> 类比：像药盒上的「请与另一半配合服用」。

### 三、防重复 include（第 26~30 行）

```cpp
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
```

**本段新元素**

- `UE_HIDETCHAR_INCLUDED` —— 「已 include」标记
- `#ifndef X` / `#define X` / `#else` / `#error` / `#endif` —— 标准防重复模式
- `#error "..."` —— 编译期报错并输出消息
- **为什么不用 `#pragma once`** —— 因为要检测「重复 include」并报错，`#pragma once` 会静默跳过

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 26 行 | 检查标记是否存在 |
| 第 27 行 | 不存在 → 打上标记 |
| 第 28 行 | 存在 → 走 else |
| 第 29 行 | 编译报错（提示漏了 AllowTCHAR.h） |
| 第 30 行 | 条件编译结束 |

> 类比：像验票：第一次进打孔，第二次进直接拦下并告诉你「你忘了出门」。

### 四、藏 TCHAR（第 32~35 行）

```cpp
// 如果平台把 TCHAR 定义成 UTF8CHAR（TCHAR 不是宽字符）。
#if PLATFORM_TCHAR_IS_UTF8CHAR
	// 覆盖 TCHAR 这个名字。
// 把 TCHAR 这个名字重定向到 UE_SYSTEM_TCHAR（藏起来）。
	#define TCHAR UE_SYSTEM_TCHAR
// 条件编译结束。
#endif
```

**本段新元素**

- `PLATFORM_TCHAR_IS_UTF8CHAR` —— 平台把 TCHAR 定义成 UTF8CHAR
- `#define TCHAR UE_SYSTEM_TCHAR` —— 把 `TCHAR` 这个名字重定向
- **只在 UTF8CHAR 模式下生效** —— 其他模式不冲突，不需要藏

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 32 行 | 平台模式判断 |
| 第 33 行 | 说明注释 |
| 第 34 行 | 重定向 TCHAR |
| 第 35 行 | 条件编译结束 |

> 类比：像给同名人起外号：暂时叫他「系统 TCHAR」，避免和我们的 TCHAR 混淆。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 版权头 + 20 行使用说明 | 1~22 | 版权头 + 20 行使用说明 |
| `HEADER_UNIT_SKIP` 标记 | 24~24 | `HEADER_UNIT_SKIP` 标记 |
| 防重复 include | 26~30 | 防重复 include |
| 藏 TCHAR | 32~35 | 藏 TCHAR |

---

## 四、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `#ifndef X` / `#define X` / `#else` / `#endif` | 防重复 include（可报错版） | `#ifndef UE_HIDETCHAR_INCLUDED` |
| `#error "msg"` | 编译期报错 | `#error "HAL/HideTCHAR.h is being multiply included..."` |
| `#define A B` | 宏替换（把 A 换成 B） | `#define TCHAR UE_SYSTEM_TCHAR` |
| `#if PLATFORM_X` | 平台条件编译 | `#if PLATFORM_TCHAR_IS_UTF8CHAR` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 35 行 | 45 行 | 多出的是每行上方的中文注释 |
| 非注释代码行 | 8 行 | 8 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| `#error` 消息 | 英文原文 | 保留英文 | 编译期输出，属运行时字符串，不翻译 |
| 示例代码里的 `#include` | 保留原文 | 保留原文 | 代码示例，不翻译 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\HideTCHAR.h`（45 行）
2. 脚本验收：`codeOurs=8 / codeEngine=8 / diff=0`
3. 确认后下一个：`HAL\IConsoleManager.h`（HAL 里最大的头之一）

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\HideTCHAR.h`（35 行）
- 配套：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\AllowTCHAR.h`（28 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\HideTCHAR.h`（45 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\11_HideTCHAR.h_总md.md`