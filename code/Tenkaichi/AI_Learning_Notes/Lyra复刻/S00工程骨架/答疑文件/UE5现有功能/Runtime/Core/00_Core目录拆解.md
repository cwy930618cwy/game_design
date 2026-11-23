# Core — 目录拆解

> 位置：`Engine\Source\Runtime\Core\`（Public 下 **44** 个子目录）
> 来源：引擎源码（只读）
> 分类：Runtime · 核心地基 · 最底层工具箱
> 对应总览：`..\01_核心地基五模块.md`

---

## 一、一句话

★ Core 不依赖任何别的引擎模块，它自己就是「C++ 工具箱」：容器、字符串、数学、委托、平台抽象、日志、文件、序列化全在这。

---

## 二、内部结构全景（44 个 Public 子目录，按功能分 11 类）

**① 基础类型与容器（7）**
`Containers` `String` `Templates` `Traits` `Hash` `Algo` `Concepts`
→ `TArray` / `TMap` / `FString` / `TSharedPtr` / `MoveTemp` 都在这。写任何 UE 代码都躲不开。

**② 数学（1）**
`Math`
→ `FVector` / `FRotator` / `FTransform` / `FMath`。

**③ 委托、异步、任务（3）**
`Delegates` `Async` `Tasks`
→ `DECLARE_DELEGATE` 系列、`Async()`、`FGraphEventRef`。Lyra 的异步加载大量用 Async。

**④ 内存（1）**
`Memory`
→ 智能指针、内存池、对齐分配。

**⑤ 平台抽象（13）**
`HAL` `GenericPlatform` `Windows` `Android` `Apple` `IOS` `Linux` `Mac` `Microsoft` `Unix` `Clang` `MSVC` `IntelICX`
→ `HAL` = Hardware Abstraction Layer，把各平台差异抹平，上层代码只调一套接口。编译器专属目录（Clang/MSVC）一般不用看。

**⑥ 文件、序列化、压缩（5）**
`IO` `Serialization` `FileCache` `Compression` `Virtualization`
→ `IFileManager` / `FArchive` / `FBufferArchive`，存档、读盘、网络字节流的基础。

**⑦ 日志、统计、性能（5）**
`Logging` `ProfilingDebugging` `Stats` `MemPro` `FramePro`
→ `UE_LOG` 宏、`stat` 命令、内存与帧性能采集。调 bug 时最常翻 `Logging`。

**⑧ 模块、对象、特性（3）**
`Modules` `UObject` `Features`
→ 模块的加载/卸载接口；注意这里的 `UObject` 子目录是底层支撑，真正的反射对象在 **CoreUObject** 模块。

**⑨ 国际化（1）**
`Internationalization`
→ 多语言文本 `FText` 的底层。

**⑩ 音频底层（1）**
`Audio`
→ 音频基础类型，具体音频引擎在别的模块。

**⑪ 实验、测试、杂项（4）**
`Experimental` `Tests` `Misc` `AutoRTFM`
→ `Misc` 里是各种杂项工具头；`AutoRTFM` 是实验性事务内存，暂时不用管。

> 模块根目录还有 4 项：`Public`（对外头）、`Private`（实现）、`Internal`（内部头）、`Tests`（单测）。

---

## 三、哪些目录你要熟

| 目录 | 熟到什么程度 |
|---|---|
| `Containers` / `String` | ★★★ 天天用，必须熟 |
| `Math` / `Delegates` | ★★ 经常用 |
| `Async` / `Serialization` | ★★ 遇到异步和存档时查 |
| `Logging` / `Stats` | ★ 调 bug 时查 |
| 平台目录 / 编译器目录 | ❌ 基本不碰 |

---

## 四、Lyra 怎么用它（大概）

- `TArray<TObjectPtr<...>>` / `TMap<FGameplayTag, ...>` → `Containers`
- `FVector` / `FRotator` 算位置朝向 → `Math`
- `UE_LOG(LogLyra, ...)` → `Logging`
- `AsyncTask(ENamedThreads::GameThread, ...)` → `Async`
- 每个 `.h` 顶部的 `#include "CoreMinimal.h"` → 就是 Core 的汇总头

---

## 五、本复刻线什么时候碰它

**全程**：只要你在写 C++，就在用 Core。但复刻时**只读不改**——你不会去抄 Core 的源码，只是查 `TArray` 有哪些方法、`FString` 怎么转数字这类用法。

---

## 六、注意

- Core 里的 `UObject` 子目录 **不是** CoreUObject 模块，别搞混：这里是底层支撑头，反射对象本体在 `Runtime\CoreUObject\`。
- 平台目录（Windows/Android/...）是同一套接口的多个实现，看一个 `GenericPlatform` 就够理解设计。
- `CoreMinimal.h` 是 Core 的「门面头」，绝大多数 include 写它就够了，不必逐个 include 子目录。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Core\Public`（44 个子目录实查）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Core\Core.Build.cs`
- 关键头实查：`Core\Public\CoreMinimal.h`、`Core\Public\CoreTypes.h`
