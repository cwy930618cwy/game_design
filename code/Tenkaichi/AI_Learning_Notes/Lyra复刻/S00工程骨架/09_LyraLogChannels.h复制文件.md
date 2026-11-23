# 09 — `LyraLogChannels.h` 复制文件

> 依据：`E:\ue5\LyraStarterGame\Source\LyraGame\LyraLogChannels.h`（**14 行**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraLogChannels.h`
> **用法**：照着下面代码逐行敲。**注释在每行上方，敲的时候整行 `//` 注释都不要抄**。

---

## 完整代码

```cpp
// 版权声明：Epic Games 保留所有权利（源为英文版权行，按铁律译成中文，但版权行本身不翻译内容）

// 只编译一次；防止同一编译单元重复 include 导致重定义
#pragma once

// 引入日志宏头文件；DECLARE_LOG_CATEGORY_EXTERN 与 DEFINE_LOG_CATEGORY 都在这里
#include "Logging/LogMacros.h"

// 前置声明 UObject；第 14 行的函数参数只用到指针类型，不需要 include 整个 UObject 头文件
class UObject;

// 声明日志频道 LogLyra：Lyra 通用日志（游戏流程 / 初始化 / 杂项）
// 参数 1 = 频道变量名；参数 2 = 默认详细级别（Log）；参数 3 = 编译期可见级别（All）
// LYRAGAME_API 让其他模块也能用这个频道
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyra, Log, All);
// 声明日志频道 LogLyraExperience：Experience 系统专用（加载 Experience / GameFeature）
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraExperience, Log, All);
// 声明日志频道 LogLyraAbilitySystem：GAS 专用（能力授予 / GE 应用 / 属性变化）
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraAbilitySystem, Log, All);
// 声明日志频道 LogLyraTeams：队伍系统专用（队伍分配 / 切换）
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraTeams, Log, All);

// 声明工具函数：返回「服务端 / 客户端」上下文标记字符串，用于日志前缀
// 参数 ContextObject 默认 nullptr；传 Actor 取它的 LocalRole，传 Component 取它 Owner 的 Role
LYRAGAME_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
```

---

## 本段新元素（只列首次出现）

- `#pragma once` —— 头文件保护，保证同一编译单元只展开一次（比 `#ifndef` 更简洁）
- `DECLARE_LOG_CATEGORY_EXTERN` —— **声明**日志频道（定义在 `.cpp`），`LogMacros.h` 提供
- `LYRAGAME_API` —— 模块导出宏，由 UBT 按模块名 `LyraGame` 自动生成，让其他模块也能引用
- `class UObject;` —— 前置声明，只为让指针类型成立，避免拉进整个 `UObject.h`
- `GetClientServerContextString` —— Lyra 自定义工具函数，返回 `"Server"` / `"Client"` / 编辑器上下文 / `"[]"`

---

## 逐段讲解

### 段 1：头文件保护 + 依赖

```cpp
// 只编译一次；防止同一编译单元重复 include 导致重定义
#pragma once

// 引入日志宏头文件；DECLARE_LOG_CATEGORY_EXTERN 与 DEFINE_LOG_CATEGORY 都在这里
#include "Logging/LogMacros.h"
```

**`#pragma once` 的作用**：

```
A.h include 了 LyraLogChannels.h
B.h include 了 LyraLogChannels.h
C.cpp include 了 A.h 和 B.h
   ↓
没有 #pragma once → LyraLogChannels.h 被展开两次 → DECLARE_LOG_CATEGORY_EXTERN 重复声明 → 编译错
有 #pragma once → 第二次 include 直接跳过 → 正常
```

**为什么 include 写 `"Logging/LogMacros.h"` 而不是 `"CoreMinimal.h"`**：
`CoreMinimal.h` 会拉进 100+ 个头文件（FString / TArray / 容器 / 数学库全部），编译慢。这里只需要日志宏，就精确 include 那一个。

### 段 2：前置声明

```cpp
// 前置声明 UObject；第 14 行的函数参数只用到指针类型，不需要 include 整个 UObject 头文件
class UObject;
```

**前置声明 vs include 的区别**：

| 写法 | 编译器知道什么 | 编译开销 |
|---|---|---|
| `class UObject;` | 只知道「有个类叫 UObject」，**不能**访问成员、不能 `new` | 几乎为 0 |
| `#include "UObject/Object.h"` | 知道完整定义（大小、成员、基类） | 大（连带拉进一堆） |

第 14 行只需要 `UObject*` 这个**指针类型**成立，所以前置声明足够。
（`.cpp` 里真正 `Cast<AActor>(ContextObject)` 时，那边才 include 需要的头文件 —— 第 10 轮会讲。）

### 段 3：4 个频道声明

```cpp
// 声明日志频道 LogLyra：Lyra 通用日志（游戏流程 / 初始化 / 杂项）
// 参数 1 = 频道变量名；参数 2 = 默认详细级别（Log）；参数 3 = 编译期可见级别（All）
// LYRAGAME_API 让其他模块也能用这个频道
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyra, Log, All);
```

**三个参数分别是什么**：

| 参数 | 值 | 含义 |
|---|---|---|
| 第 1 个 | `LogLyra` | 频道变量名；`UE_LOG(LogLyra, Warning, TEXT("..."))` 里用的就是它 |
| 第 2 个 | `Log` | **默认详细级别** —— 运行时不写 `-LogCmds` 参数时，这个频道显示到 `Log` 级（不显示 `Verbose` / `VeryVerbose`） |
| 第 3 个 | `All` | **编译期可见级别** —— 编译时把 `All` 及以上的日志代码都编进去（`NoLogging` 会全部剔除） |

**日志详细级别从高到低**（越往下越啰嗦）：

```
Fatal       → 致命错误，会崩
Error       → 错误
Warning     → 警告
Display     → 显式提示（默认 Output Log 显示到这一级）
Log         → 普通日志  ← 我们的默认级别
Verbose     → 详细
VeryVerbose → 极其详细
All         → 全部（含断言用）
```

**`LYRAGAME_API` 的作用**：

```
没有前缀：
  LogLyra 只在 LyraGame.dll 内部可见
  → LyraEditor.dll 里写 UE_LOG(LogLyra, ...) → 链接报错「未解析的外部符号」

有 LYRAGAME_API：
  符号导出到 DLL 边界
  → LyraEditor.dll / 各插件都能直接用
```

> 📌 `LYRAGAME_API` 这个宏是 UBT 自动生成的 —— 因为模块名叫 `LyraGame`，UBT 就生成 `LYRAGAME_API`（大写 + `_API`）。**你没在任何地方定义它，它就能用**，这就是为什么 `.uproject` 里模块名必须是 `LyraGame`。

### 段 4：工具函数声明

```cpp
// 声明工具函数：返回「服务端 / 客户端」上下文标记字符串，用于日志前缀
// 参数 ContextObject 默认 nullptr；传 Actor 取它的 LocalRole，传 Component 取它 Owner 的 Role
LYRAGAME_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
```

**典型用法**（Lyra 源码里到处都是）：

```cpp
UE_LOG(LogLyraTeams, Warning, TEXT("%s Team %d assigned"),
    *GetClientServerContextString(this),   // ← 这里
    TeamId);
```

输出：

```
LogLyraTeams: Warning: [Server] Team 1 assigned
```

**默认参数 `= nullptr` 的意义**：

```cpp
// 调用方可以省参数
GetClientServerContextString()        // ContextObject 自动为 nullptr
GetClientServerContextString(this)    // 传 Actor / Component
```

不传时函数返回什么？看 `.cpp`（第 10 轮）：`Role == ROLE_None` 且不在编辑器 → 返回 `TEXT("[]")`。

---

## 敲完自查

| 检查项 | 通过标准 |
|---|---|
| 文件路径 | `Source\LyraGame\LyraLogChannels.h` |
| `#pragma once` | 有，在版权行之后 |
| include | **只有 1 个**：`#include "Logging/LogMacros.h"` |
| 前置声明 | `class UObject;` |
| 频道数 | **4 个**，顺序：`LogLyra` / `LogLyraExperience` / `LogLyraAbilitySystem` / `LogLyraTeams` |
| 每个频道 | 都有 `LYRAGAME_API` 前缀，参数是 `(名字, Log, All)` |
| 函数声明 | `LYRAGAME_API FString GetClientServerContextString(UObject* ContextObject = nullptr);` |
| 注释 | 所有 `//` 开头的行**全部删掉** |
| 代码行 | 剔注释后 **8 行**（`#pragma once` / `#include` / `class UObject;` / 4 个频道 / 函数声明） |

> ⚠️ **`LYRAGAME_API` 不是拼错的 `LYRA_API`** —— 模块名是 `LyraGame`，所以宏叫 `LYRAGAME_API`（全大写）。写错成 `LYRA_API` 会报「未定义标识符」。

---

## 源码依据

- `E:\ue5\LyraStarterGame\Source\LyraGame\LyraLogChannels.h:1-14`（源全文 14 行）
- `E:\ue5\LyraStarterGame\Source\LyraGame\LyraLogChannels.h:9-12`（4 个频道声明）
- `E:\ue5\LyraStarterGame\Source\LyraGame\LyraLogChannels.h:14`（`GetClientServerContextString` 声明）
- `E:\ue5\LyraStarterGame\Source\LyraGame\LyraLogChannels.cpp:1-40`（对应的定义，第 10 轮教）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Core\Public\Logging\LogMacros.h`（`DECLARE_LOG_CATEGORY_EXTERN` / `DEFINE_LOG_CATEGORY` 宏定义）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Core\Public\Logging\LogVerbosity.h`（`ELogVerbosity` 枚举：Fatal / Error / Warning / Display / Log / Verbose / VeryVerbose / All）