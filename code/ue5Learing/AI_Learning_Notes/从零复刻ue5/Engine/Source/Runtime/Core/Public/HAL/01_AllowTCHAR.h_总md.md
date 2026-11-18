# 01 — `HAL/AllowTCHAR.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\AllowTCHAR.h`（**28 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\AllowTCHAR.h`
> **⚠️ 本节不能独立编译**——必须与 `HAL/HideTCHAR.h` 成对 include，单独 include 会触发 `#error`。

---

## 一、这个文件解决什么问题（为什么）

第三方库（比如 `<tchar.h>`、某些 SDK 头）里会用到 `TCHAR` 这个名字。UE 在 `PLATFORM_TCHAR_IS_UTF8CHAR` 模式下把 `TCHAR` 定义成 `char`（UTF-8），而 Windows 系统头把 `TCHAR` 定义成 `wchar_t` —— **两者撞名**，编译直接报重定义。

UE 的解法是「**临时把 TCHAR 藏起来**」：

```
#include "HAL/HideTCHAR.h"        ← 把 TCHAR 改名成 UE_SYSTEM_TCHAR（藏起来）
#include <tchar.h>                ← 第三方头看到的是 UE_SYSTEM_TCHAR，不冲突
#include "HAL/AllowTCHAR.h"       ← 把 TCHAR 还回来（恢复成 UE 的定义）
```

`AllowTCHAR.h` 就是**「还回来」的那一半**：它撤销 `HideTCHAR.h` 的 hack，并把系统原生 TCHAR 记成 `UE_SYSTEM_TCHAR` 备用。

**说个场景**：你接入一个第三方图像库，它的头文件里有 `TCHAR szPath[MAX_PATH];`。直接 include 会和 UE 的 `TCHAR` 冲突。解法就是上面那三行包裹。

> 🔑 **一句话**：`AllowTCHAR.h` = **TCHAR 的「解封开关」**，必须和 `HideTCHAR.h` 成对使用，单独用会 `#error`。

---

## 二、内部结构全景

```
AllowTCHAR.h（28 行）
├─ 第 1~6 行    版权头 + 使用说明 + HEADER_UNIT_SKIP 标记
├─ 第 8~12 行   ① 配对校验：没 include 过 HideTCHAR.h 就 #error
├─ 第 14~24 行  ② 首次 include 时：把当前 TCHAR 记成 UE_SYSTEM_TCHAR
└─ 第 26~28 行  ③ UTF8CHAR 模式下：撤销 TCHAR 的宏定义
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/AllowTCHAR.h` | 无 | ❌ **不能**（必须跟在 `HideTCHAR.h` 后） | 否 |

**配对关系**：

| 文件 | 作用 |
|---|---|
| `HAL/HideTCHAR.h` | 把 `TCHAR` 宏定义成 `UE_SYSTEM_TCHAR`（藏起来） |
| `HAL/AllowTCHAR.h` | 撤销上面的宏，恢复 `TCHAR`（还回来） |

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\14_AllowTCHAR.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `ConsoleManager.h`（333 字节）。