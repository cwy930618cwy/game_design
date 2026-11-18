# 19 — `HAL/LowLevelMemTracker.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemTracker.h`（**1127 行 / 59KB**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemTracker.h`
> **⚠️ 本节不能独立编译**——依赖 `FTagTrace` / `AutoRTFM` / `FName`。

---

## 一、这个文件解决什么问题（为什么）

`stat LLM` 面板能告诉你「Textures 用了 800MB、Meshes 用了 300MB」。这套系统的核心就是 **LLM（Low Level Memory Tracker）**：它在**每一次 malloc/free** 时记录「这次分配属于哪个标签」，然后按标签汇总。

`LowLevelMemTracker.h` 是 LLM 的**全部公开接口**，1127 行，包含：

| 类别 | 内容 |
|---|---|
| **配置宏** | `ENABLE_LOW_LEVEL_MEM_TRACKER` / `LLM_ALLOW_ASSETS_TAGS` / `LLM_ALLOW_STATS` / `LLM_ENABLED_STAT_TAGS` |
| **断言宏** | `LLMCheck` / `LLMCheckf` / `LLMEnsure` |
| **枚举** | `ELLMTracker`（2 个）/ `ELLMTagSet`（3 个）/ `ELLMTag`（100+ 个）/ `ELLMAllocType`（4 个）/ `ESizeParams` |
| **标签列表** | `LLM_ENUM_GENERIC_TAGS(macro)` —— 100+ 个标签的 X-Macro |
| **作用域宏** | `LLM_SCOPE` / `LLM_SCOPE_DYNAMIC` / `LLM_PLATFORM_SCOPE` / `LLM_SCOPED_PAUSE_TRACKING` / `LLM_REALLOC_SCOPE` 等 20+ 个 |
| **标签定义宏** | `LLM_DEFINE_TAG` / `LLM_DECLARE_TAG` / `LLM_DEFINE_BOOTSTRAP_TAG` |
| **核心类** | `FLowLevelMemTracker`（主追踪器）/ `FLLMScope` / `FLLMScopeDynamic` / `FLLMPauseScope` / `FLLMClearScope` / `FLLMScopeFromPtr` / `FLLMTagDeclaration` / `FLLMAllocator` |

**核心设计 ①：两个追踪器**（第 120~126 行）

```cpp
enum class ELLMTracker : uint8
{
    Platform,   // 平台层：追踪内存管理系统（如 Binned2 的页）
    Default,    // 默认层：追踪引擎分配（GMalloc）
    Max,
};
```

**为什么要两个**：`Platform` 追踪「分配器自己占了多少内存」，`Default` 追踪「游戏逻辑占了多少内存」。两者相加才是真实占用。

**核心设计 ②：X-Macro 标签列表**（第 163~280 行）

```cpp
#define LLM_ENUM_GENERIC_TAGS(macro) \
    macro(Untagged, "Untagged", NAME_None, NAME_None, -1) \
    macro(Total, "Total", GET_STATFNAME(STAT_TotalLLM), ..., -1) \
    macro(Audio, "Audio", ..., ELLMTag::Audio) \
    ... 100+ 行
```

**为什么用 X-Macro**：同一份列表要展开成 3 种东西（枚举值、名字表、父标签表），X-Macro 保证三者永远同步。

**核心设计 ③：`LLM_SCOPE` 的 RAII**（第 344~345 行）

```cpp
#define LLM_SCOPE(Tag) \
    FLLMScope SCOPE_NAME(Tag, false, ELLMTagSet::None, ELLMTracker::Default); \
    UE_MEMSCOPE(Tag)
```

`SCOPE_NAME` 用 `__LINE__` 生成唯一变量名，所以同一作用域可以写多个 `LLM_SCOPE`。

**核心设计 ④：`LLM_SCOPE_DYNAMIC` 的 do/while**（第 346~371 行）

```cpp
#define LLM_SCOPE_DYNAMIC(UniqueName, Tracker, TagSet, Constructor) \
    FLLMScopeDynamic SCOPE_NAME(Tracker, TagSet); \
    UE_MEMSCOPE_UNINITIALIZED(__LINE__); \
    do { ... } while (false) /* do/while is added to require a semicolon */
```

`do/while(false)` 是为了**强制要求分号** —— 否则 `if (x) LLM_SCOPE_DYNAMIC(...)` 会出问题。

**核心设计 ⑤：`LLM_DEFINE_TAG` 的全局实例**（第 423 行）

```cpp
#define LLM_DEFINE_TAG(UniqueNameWithUnderscores, ...) \
    FLLMTagDeclaration PREPROCESSOR_JOIN(LLMTagDeclaration_, UniqueNameWithUnderscores)(TEXT(#UniqueNameWithUnderscores), ##__VA_ARGS__)
```

定义一个**全局 `FLLMTagDeclaration` 实例**，它的构造函数会自动把标签注册到 LLM。

**说个场景**：

```cpp
// 定义一个自定义标签
LLM_DEFINE_TAG(MyGame_Weapons, TEXT("Weapons"), NAME_None, GET_STATFNAME(STAT_MyWeaponsLLM));

void LoadWeapon()
{
    LLM_SCOPE_BYTAG(MyGame_Weapons);   // 之后所有分配都归到 Weapons
    // ... 加载武器 ...
}
```

**双分支设计**：`#if ENABLE_LOW_LEVEL_MEM_TRACKER` ... `#else`（第 1086~1127 行）—— 关闭 LLM 时所有宏变空，**零开销**。

> 🔑 **一句话**：`LowLevelMemTracker.h` = **内存追踪的「总控台」**，2 个追踪器 + 100+ 个标签 + 20+ 个作用域宏。

---

## 二、内部结构全景

```
LowLevelMemTracker.h（1127 行）
├─ 第 1~21 行      版权头 + include + PLATFORM_SUPPORTS_LLM + ENABLE_LOW_LEVEL_MEM_TRACKER
├─ 第 23~52 行     ① 配置宏（LLM_ALLOW_ASSETS_TAGS / UOBJECTCLASSES / STATS / ENABLED_STAT_TAGS）
├─ 第 54~65 行     ② 9 个 include + <atomic>
├─ 第 67~108 行    ③ 断言宏（LLMCheck / LLMCheckf / LLMEnsure + TrueOnFirstCallOnly）
├─ 第 110~115 行   ④ LLM_TAG_TYPE / LLM_MEMORY_OVERHEAD
├─ 第 120~126 行   ⑤ enum class ELLMTracker（Platform / Default / Max）
├─ 第 131~142 行   ⑥ enum class ELLMTagSet（None / Assets / UObjectClasses / Max）
├─ 第 147~160 行   ⑦ namespace UE::LLM { enum class ESizeParams }
├─ 第 163~280 行   ⑧ LLM_ENUM_GENERIC_TAGS(macro) —— 100+ 个标签
├─ 第 285~311 行   ⑨ enum class ELLMTag + 4 个常量
├─ 第 317~324 行   ⑩ enum class ELLMAllocType（None / FMalloc / System / RHI / Count）
├─ 第 332~335 行   ⑪ LLM / LLM_IF_ENABLED / LLM_IS_ENABLED / SCOPE_NAME
├─ 第 344~388 行   ⑫ 作用域宏（LLM_SCOPE / DYNAMIC / TAGSET / BYNAME / BYTAG / RENDER_RESOURCE / PLATFORM / CLEAR）
├─ 第 393~396 行   ⑬ 暂停宏（LLM_SCOPED_PAUSE_TRACKING ×4）
├─ 第 401~402 行   ⑭ realloc 宏（LLM_REALLOC_SCOPE ×2）
├─ 第 407~408 行   ⑮ 转储宏（LLM_DUMP_TAG ×2）
├─ 第 423~468 行   ⑯ 标签定义宏（DEFINE_TAG / DEFINE_STATIC_TAG / DECLARE_TAG / TAG_NAME / BOOTSTRAP ×5）
├─ 第 470~471 行   ⑰ LLMAllocFunction / LLMFreeFunction
├─ 第 477~587 行   ⑱ namespace UE::LLMPrivate（FLLMAllocator / ETagReferenceSource / FPrivateCallbacks）
├─ 第 589~604 行   ⑲ FLLMCustomTag（废弃）/ FLLMTagInfo
├─ 第 607~914 行   ⑳ class FLowLevelMemTracker（主追踪器，60+ 个方法）
├─ 第 917~932 行   ㉑ class FLLMScope
├─ 第 938~959 行   ㉒ ILLMDynamicTagConstructor / FLLMDynamicTagConstructorStatString
├─ 第 967~999 行   ㉓ class FLLMScopeDynamic
├─ 第 1002~1014 行 ㉔ class FLLMPauseScope
├─ 第 1017~1021 行 ㉕ class FLLMClearScope
├─ 第 1024~1038 行 ㉖ class FLLMScopeFromPtr
├─ 第 1041~1070 行 ㉗ class FLLMTagDeclaration
├─ 第 1073~1079 行 ㉘ struct FLLMTagSetAllocationFilter
├─ 第 1081~1084 行 ㉙ FLowLevelMemTracker::IsEnabled 内联实现
└─ 第 1086~1127 行 ㉚ #else 分支：30+ 个空宏
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/LowLevelMemTracker.h` | **是**（`FTagTrace` / `FName` / `AutoRTFM`） | ❌ 不能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `CoreTypes.h` | `uint8` / `uint32` / `int64` / `TCHAR` |
| `LowLevelMemTrackerDefines.h` | `LLM_ENABLED_IN_CONFIG` |
| `ProfilingDebugging/TagTrace.h` | `FTagTrace` |
| `AutoRTFM.h` | 事务内存支持 |
| `Containers/Array.h` / `ArrayView.h` / `StringFwd.h` | 容器 |
| `HAL/CriticalSection.h` | `FCriticalSection` |
| `HAL/PlatformCrt.h` / `PlatformMisc.h` | 平台相关 |
| `Templates/AlignmentTemplates.h` / `UnrealTemplate.h` | `Align` / `Forward` |
| `UObject/NameTypes.h` / `UnrealNames.h` | `FName` / `NAME_None` |
| `<atomic>` | `std::atomic<bool>` |

**注意**：`FRWLock` / `FOutputDevice` / `TMap` / `FStringBuilderBase` / `UPTRINT` / `FMemory` 都**没有在本文件 include**，靠上面 9 个头间接引入（引擎的隐式依赖，照抄不改）。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\32_LowLevelMemTracker.h逐行注释版.md` + 还原文件，同一轮产出并验收。

> ⚠️ **本文件特殊**：28 个块注释统一压成**单行 `//` 中文注释**，保证代码行零差异。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `LowLevelMemTrackerDefines.h`（1042 字节）。