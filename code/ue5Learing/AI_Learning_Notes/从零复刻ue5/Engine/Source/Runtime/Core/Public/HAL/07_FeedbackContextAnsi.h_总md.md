# 07 — `HAL/FeedbackContextAnsi.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\FeedbackContextAnsi.h`（**34 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\FeedbackContextAnsi.h`
> **⚠️ 本节不能独立编译**——依赖 `Misc/FeedbackContext.h` 的基类定义。

---

## 一、这个文件解决什么问题（为什么）

引擎跑长任务（编译、cook、导入资源）时要告诉用户「现在在干什么」。这套机制叫**反馈上下文**（Feedback Context）：`FFeedbackContext` 是基类，`FFeedbackContextAnsi` 是**ANSI 控制台风格**的实现 —— 把进度和日志直接打到终端（stdout），不弹 GUI 窗口。

**它继承 `FFeedbackContext` 并重写 5 个虚函数**：

| 方法 | 作用 |
|---|---|
| `Serialize(V, Verbosity, Category)` | 输出一条日志 |
| `Serialize(V, Verbosity, Category, Time)` | 带时间戳的输出重载 |
| `SerializeRecord(Record)` | 输出结构化日志记录（新版 API） |
| `YesNof(Question)` | 弹「是/否」询问（终端里读 y/n） |
| `GetContext()` / `SetContext()` | 读写当前上下文供应器 |

**说个场景**：你在命令行跑 `UnrealEditor-Cmd.exe MyProject.uproject -run=cook`。cook 过程中终端不断刷 `LogCook: Display: Cooking /Game/Map...` —— 这些输出就是 `FFeedbackContextAnsi::Serialize()` 打出来的。

**`Context` 是什么**：`FContextSupplier` 是个「当前在干什么」的供应器。比如 cook 时它返回 `"Cooking package X"`，进度条就能显示这句话。

> 🔑 **一句话**：`FeedbackContextAnsi.h` = **命令行版的「进度播报员」**，把引擎的进度和日志直接打到终端。

---

## 二、内部结构全景

```
FeedbackContextAnsi.h（34 行）
├─ 第 1~13 行    版权头 + #pragma once + 9 个 include
├─ 第 15~17 行   ① class FFeedbackContextAnsi : public FFeedbackContext
├─ 第 18~19 行   ② 2 个公有成员（Context / AuxOut）
├─ 第 21 行      ③ static LocalPrint（静态打印）
├─ 第 23~25 行   ④ 3 个 Serialize 重写
├─ 第 27 行      ⑤ YesNof 重写
├─ 第 29~30 行   ⑥ GetContext / SetContext（内联）
└─ 第 32~33 行   ⑦ private: IsUsingLocalPrint
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/FeedbackContextAnsi.h` | **是**（继承 `FFeedbackContext`） | ❌ 不能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `CoreTypes.h` | `TCHAR` / `bool` |
| `Containers/StringConv.h` | TCHAR ↔ ANSI/UTF8 转换 |
| `Containers/UnrealString.h` | `FString` |
| `CoreGlobals.h` | `GLog` / `GWarn` |
| `Misc/App.h` | `FApp` |
| `Misc/OutputDeviceConsole.h` | `FOutputDeviceConsole` |
| `Misc/OutputDeviceHelper.h` | 日志格式化工具 |
| `Misc/OutputDeviceRedirector.h` | `FOutputDeviceRedirector` |
| `Misc/FeedbackContext.h` | **基类 `FFeedbackContext`** |

**注意**：`FContextSupplier` / `FOutputDevice` / `ELogVerbosity` / `FName` / `FText` / `UE::FLogRecord` 都**没有在本文件 include**，靠上面 9 个头间接引入（引擎的隐式依赖，照抄不改）。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\20_FeedbackContextAnsi.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `FileManager.h`（12810 字节）。