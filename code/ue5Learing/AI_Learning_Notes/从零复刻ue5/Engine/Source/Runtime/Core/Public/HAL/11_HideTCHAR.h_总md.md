# 11 — `HAL/HideTCHAR.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\HideTCHAR.h`（**35 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\HideTCHAR.h`
> **⚠️ 本节不能独立编译**——必须与 `HAL/AllowTCHAR.h` 成对 include。

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

## 二、内部结构全景

```
HideTCHAR.h（35 行）
├─ 第 1~22 行    版权头 + 20 行使用说明（为什么、怎么用、链接影响、token 粘贴）
├─ 第 24 行      HEADER_UNIT_SKIP 标记
├─ 第 26~30 行   ① 防重复 include（重复就 #error）
└─ 第 32~35 行   ② PLATFORM_TCHAR_IS_UTF8CHAR 下 #define TCHAR UE_SYSTEM_TCHAR
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/HideTCHAR.h` | 无 | ❌ **不能**（必须配 `AllowTCHAR.h`） | 否 |

**配对关系**：

| 文件 | 作用 | 关键宏 |
|---|---|---|
| `HAL/HideTCHAR.h` | 藏 `TCHAR` | `#define TCHAR UE_SYSTEM_TCHAR` |
| `HAL/AllowTCHAR.h` | 还 `TCHAR` | `#undef TCHAR` |

**依赖的外部符号**：

| 符号 | 来源 |
|---|---|
| `PLATFORM_TCHAR_IS_UTF8CHAR` | `HAL/Platform.h` |
| `UE_SYSTEM_TCHAR` | `HAL/AllowTCHAR.h` 里 `using UE_SYSTEM_TCHAR = TCHAR;` 定义 |

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\24_HideTCHAR.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `IConsoleManager.h`（约 40KB，HAL 里最大的头之一）。