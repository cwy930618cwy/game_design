# 12 — `HAL/IConsoleManager.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\IConsoleManager.h`（**2310 行 / 85KB**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IConsoleManager.h`
> **⚠️ 本节不能独立编译**——依赖 `IModularFeature` / `FText` / `FDelegateHandle` 等。

---

## 一、这个文件解决什么问题（为什么）

你在游戏里按 `~` 打开控制台，敲 `r.ScreenPercentage 50`，画面立刻变糊。这条命令背后就是 `IConsoleManager.h` 定义的系统 —— **控制台变量（CVar）与控制台命令（CCmd）**。

**它解决三个问题**：

| 问题 | 解法 |
|---|---|
| 运行时改参数（不改代码、不重编译） | **CVar**：`r.ScreenPercentage` / `sg.ShadowQuality` |
| 执行一次性动作（如 `stat fps`） | **CCmd**：`FAutoConsoleCommand` |
| 参数优先级冲突（ini / 命令行 / 用户 / 热修都想设同一个值） | **`ECVF_SetBy*` 15 级优先级** |

**核心类型**：

| 类型 | 作用 |
|---|---|
| `IConsoleObject` | 控制台对象基类（变量和命令的公共父类） |
| `IConsoleVariable` | 控制台变量接口（`GetInt` / `Set` / `GetString` …） |
| `IConsoleCommand` | 控制台命令接口（`Execute`） |
| `IConsoleManager` | 管理器接口（`RegisterConsoleVariable` / `FindConsoleVariable` …） |
| `TConsoleVariableData<T>` | 带**双影子副本**的 cvar 数据（主线程 / 渲染线程各一份） |
| `FAutoConsoleVariable` / `TAutoConsoleVariable` | RAII 自动注册 cvar |
| `FAutoConsoleVariableRef` | RAII 自动注册 cvar 引用（直接绑到你的变量） |
| `FAutoConsoleCommand` | RAII 自动注册命令 |

**说个场景**：

```cpp
// 声明一个 cvar，默认 100，帮助文本，可伸缩性标志
static TAutoConsoleVariable<int32> CVarScreenPercentage(
    TEXT("r.ScreenPercentage"), 100, TEXT("屏幕分辨率百分比"), ECVF_Scalability);

// 读值（游戏线程）
int32 Val = CVarScreenPercentage.GetValueOnGameThread();
```

**`ECVF_SetBy*` 15 级优先级**（第 143~171 行）—— 这是本文件最精妙的设计：

```
Constructor(0x00) < Scalability(0x01) < GameSetting(0x02) < ProjectSetting(0x03)
< SystemSettingsIni(0x04) < PluginLowPriority(0x05) < DeviceProfile(0x06)
< PluginHighPriority(0x07) < GameOverride(0x08) < ConsoleVariablesIni(0x09)
< Hotfix(0x0A) < Preview(0x0B) < Commandline(0x0C) < Code(0x0D) < Console(0x0E)
```

高优先级覆盖低优先级。用户在控制台敲的值（`Console`）永远赢过 ini 里的值（`SystemSettingsIni`）。

> 🔑 **一句话**：`IConsoleManager.h` = **引擎的「运行时参数总线」**，CVar 是参数、CCmd 是动作、`SetBy` 是优先级仲裁。

---

## 二、内部结构全景

```
IConsoleManager.h（2310 行）
├─ 第 1~25 行      版权头 + 11 个 include + TRACK_CONSOLE_FIND_COUNT + cvarCheckCode 宏
├─ 第 27~46 行     ① 控制台变量使用指南（20 行注释）
├─ 第 48~175 行    ② enum EConsoleVariableFlags（40+ 个标志位）
│    ├─ 第 54~119 行   普通标志（FlagMask 低 16 位）
│    ├─ 第 124~133 行  设置类标志（SetFlagMask 第 16~23 位）
│    └─ 第 138~171 行  设置来源（SetByMask 高 8 位，15 级优先级）
├─ 第 178~193 行   ③ ENUMERATE_SET_BY(op) 宏（15 个 op）
├─ 第 196~200 行   ④ GetConsoleVariableSetByName / GetConsoleVariableSetByValue
├─ 第 202~215 行   ⑤ namespace ConsoleVariablePlatformMapping（4 个映射函数）
├─ 第 219~375 行   ⑥ 委托类型（NO_CVARS 双分支）
│    ├─ 第 222~249 行  8 个 DECLARE_DELEGATE（真实版）
│    └─ 第 253~373 行  FNullConsoleVariableDelegate（空壳版，20 个 Create*）
├─ 第 379~548 行   ⑦ class IConsoleObject（控制台对象基类）
├─ 第 550~711 行   ⑧ class IConsoleVariable（控制台变量接口）
├─ 第 713~726 行   ⑨ struct IConsoleCommand（控制台命令接口）
├─ 第 728~738 行   ⑩ struct IConsoleThreadPropagation（线程传播接口）
├─ 第 740~747 行   ⑪ FConsoleObjectVisitor 委托
├─ 第 750~779 行   ⑫ class FConsoleVariableSinkHandle（sink 句柄）
├─ 第 781~793 行   ⑬ struct FConsoleSuggestion（自动补全建议）
├─ 第 795~874 行   ⑭ class IConsoleCommandExecutor（命令执行器，IModularFeature）
├─ 第 877~1262 行  ⑮ struct IConsoleManager（管理器接口，60+ 个方法）
├─ 第 1265~1291 行 ⑯ template TGuardConsoleVariable（作用域守卫）
├─ 第 1293~1313 行 ⑰ class FAutoConsoleVariableSink
├─ 第 1316~1367 行 ⑱ class FAutoConsoleObject（自动注册基类）
├─ 第 1369~1504 行 ⑲ class FAutoConsoleVariable（8 个构造 + 4 个运算符）
├─ 第 1506~1675 行 ⑳ class FAutoConsoleVariableRef（8 个构造 + 4 个运算符）
├─ 第 1678~1746 行 ㉑ template TConsoleVariableData（双影子副本）
├─ 第 1748~1981 行 ㉒ template TAutoConsoleVariable（模板版）
├─ 第 1983~2096 行 ㉓ class FAutoConsoleCommand（6 个构造）
├─ 第 2099~2221 行 ㉔ 5 个 FAutoConsoleCommandWith* 变体
├─ 第 2224~2236 行 ㉕ enum class EShadowCVarBehavior（5 个值）
├─ 第 2238~2256 行 ㉖ class FAutoConsoleVariableShadow
├─ 第 2258~2278 行 ㉗ class FAutoConsoleVariableDeprecated
├─ 第 2280~2295 行 ㉘ class FAutoConsoleCommandDeprecated
├─ 第 2298~2306 行 ㉙ template TAutoRangedConsoleVariable
└─ 第 2309 行      ㉚ DECLARE_LOG_CATEGORY_EXTERN(LogConsoleResponse)
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/IConsoleManager.h` | **是**（`IModularFeature` / `FText` / `FDelegateHandle`） | ❌ 不能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `CoreTypes.h` | `uint8` / `uint32` / `int32` / `TCHAR` |
| `Misc/AccessDetection.h` | `UE::AccessDetection::ReportAccess`（检测 CVar 跨线程误用） |
| `Misc/AssertionMacros.h` | `check` / `ensure` / `ensureMsgf` |
| `Templates/UnrealTemplate.h` | `TIdentity` / `TMemFunPtrType` |
| `Containers/UnrealString.h` | `FString` |
| `Logging/LogMacros.h` | `DECLARE_LOG_CATEGORY_EXTERN` |
| `Delegates/IDelegateInstance.h` | `FDelegateHandle` |
| `Delegates/Delegate.h` | `DECLARE_DELEGATE` / `DECLARE_MULTICAST_DELEGATE` |
| `Features/IModularFeature.h` | `IModularFeature` |
| `Templates/EnableIf.h` | `TEnableIf` |
| `Internationalization/Text.h` | `FText` |

**注意**：`UWorld` / `FOutputDevice` / `TArray` / `FName` / `NAME_None` 都**没有在本文件 include**，靠上面 11 个头间接引入（引擎的隐式依赖，照抄不改）。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\25_IConsoleManager.h逐行注释版.md` + 还原文件，同一轮产出并验收。

> ⚠️ **本文件特殊**：2310 行 / 150 个块注释。块注释统一压成**单行 `//` 中文注释**（不保留 `/** */` 多行结构），保证代码行零差异。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `IConsoleManager.inl`（如果存在）或 `IConsoleObject.h`。