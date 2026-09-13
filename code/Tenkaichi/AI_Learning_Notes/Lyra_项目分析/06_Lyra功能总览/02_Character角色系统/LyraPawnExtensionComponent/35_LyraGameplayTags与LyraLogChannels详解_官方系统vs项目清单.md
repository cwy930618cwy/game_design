# 35 — `LyraGameplayTags.h` 和 `LyraLogChannels.h`：官方系统 vs 项目自己的清单

> **定位**：`LyraPawnExtensionComponent.cpp` 第 10~11 行：
>
> ```cpp
> #include "LyraGameplayTags.h"    // Lyra 的"游戏标签清单"
> #include "LyraLogChannels.h"     // Lyra 的"日志通道声明"
> ```
>
> 你可能有个误解（很多人都有）：**"LyraGameplayTags 是 Lyra 自己定义的一套标签系统？"** ——不对！这篇把这个关键澄清讲透：
>
> 1. **标签"系统"是官方 UE 的**（GameplayTags 插件），只有一个。
> 2. **LyraGameplayTags.h 不是新系统，只是 Lyra 在官方系统里"登记自己要用的一批标签"的清单文件。**
>
> 顺带讲第二行 `LyraLogChannels.h`（日志通道，和标签无关但常一起出现）。

---

## 〇、先破误解：只有一个系统，Lyra 只是"报户口"

> **GameplayTags 系统 = 官方 UE 自带的插件，全世界只有一个。** 它本身是"空的中转站"——它管的是"如何高效地存/查标签"，但**不管你项目里有哪些标签**。
>
> 你的项目要用哪些标签，得**自己去登记**。Lyra 登记的地方，就是这个 `LyraGameplayTags.h/.cpp`。
>
> **类比**：GameplayTags 系统 = **身份证系统**（官方建的，全国一套）；`LyraGameplayTags` = Lyra 这家人去派出所**报户口**（把自己要用的标签名字登记进官方系统）。

```
官方系统（只有一套）              Lyra 的清单（去官方那登记）
GameplayTags 插件                 LyraGameplayTags.h/.cpp
  - 提供 FGameplayTag 类型         - UE_DECLARE_GAMEPLAY_TAG_EXTERN(...) 声明
  - 提供 UE_DECLARE/DEFINE 宏      - UE_DEFINE_GAMEPLAY_TAG(...) 定义
  - 提供查询/层级/网络同步能力      - 就是"我 Lyra 要用这些 tag"
```

---

## 一、官方 GameplayTags 是什么？（为什么它"没有默认标签"）

### 它是引擎里的一个插件/模块

`GameplayTags` 是 UE 官方的功能模块（GAS 也依赖它）。它提供：
- `FGameplayTag` 类型（一个 tag 的运行时表示）
- tag 的层级组织（`Character.State.Stunned` 这种点分结构）
- 高效存储/查询
- 网络同步（tag 作为名字/ID 在服务器客户端间同步）

### 但它"默认是空的"——这才是你问题的关键

官方系统**不会替你预置业务标签**（不会有"血量""眩晕""Spawned"这些）。原因和之前"官方不给封装"一样：**1000 个游戏有 1000 套标签**，官方没法替你定。

> 所以 Lyra 用到 `InitState_Spawned`、`InputTag_Move`、`Ability_ActivateFail_Cooldown` 这些 tag 时，**必须先自己在某处把它"创建/登记"进官方系统**——否则运行时查不到。

### 标签从哪来？（两种登记途径）

| 途径 | 在哪 | 适合 |
|---|---|---|
| **数据资产**（DataTable） | 编辑器里配 tag 表 | 策划可改、数量多 |
| **原生 C++ 声明**（Native Gameplay Tags） | 代码里 `UE_DECLARE/DEFINE_GAMEPLAY_TAG` | 程序用、需在代码里引用（如状态机 tag） |

**Lyra 用原生 C++ 方式**：因为 `InitState_Spawned` 这类 tag 要在 C++ 代码里当常量用（`LyraGameplayTags::InitState_Spawned`），必须在编译期就有。

---

## 二、`LyraGameplayTags.h` 到底在干嘛？（逐块看）

### 文件顶部（L5）

```cpp
#include "NativeGameplayTags.h"    // ← 官方提供的"原生标签"支持头
```

它 include 的是**官方** `NativeGameplayTags.h`——里面定义了 `UE_DECLARE_GAMEPLAY_TAG_EXTERN` / `UE_DEFINE_GAMEPLAY_TAG` 宏。**Lyra 只是用官方给的宏。**

### 中部：一堆声明（L12~59）

```cpp
namespace LyraGameplayTags    // 包一个命名空间，避免名字冲突
{
	// Declare all of the custom native tags that Lyra will use
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);        // "已生成"状态
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);  // "数据到位"状态
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);            // 移动输入
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);  // 冷却中无法施放
	// ...（约 40 个）
}
```

**每个 `UE_DECLARE_GAMEPLAY_TAG_EXTERN(X)` = 声明一个"外部可用的 tag 常量"**。比如声明后，别处就能写 `LyraGameplayTags::InitState_Spawned`。

### 真正给 tag 赋"名字字符串"的地方在 .cpp

头文件只是**声明**（告诉编译器"有这么个常量"）。真正的**定义**（把 tag 和字符串绑一起）在 `LyraGameplayTags.cpp` 里：

```cpp
UE_DEFINE_GAMEPLAY_TAG(InitState_Spawned, "InitState.Spawned");   // ← 字符串真正注册进官方系统
```

> **声明 + 定义分离**：`.h` 声明（`extern`，别人能用），`.cpp` 定义（把字符串注册进官方系统）。这和第 20 篇 `NAME_ActorFeatureName` 的"声明/定义分离"是同一套 C++ 惯例。

### 所以回到你的问题

> "我以为是自己定义的 gas tags 系统，还有默认的吗？"
>
> **答**：标签**系统**是官方默认自带的（GameplayTags，一个）；但标签**内容**（哪些 tag）官方没有默认，Lyra 用 `LyraGameplayTags.h/.cpp` 把自己要用的约 40 个 tag **登记进官方系统**。它不是"自己定义了一个系统"，而是"用官方的系统 + 报了自己这批户口"。

---

## 三、为什么 `LyraPawnExtensionComponent.cpp` 要 include 它？

回到这个文件——它哪里用到了 LyraGameplayTags？看之前读过的代码：

```cpp
// CanChangeInitState 里（L229/237/259/264）全是这些 tag
if (DesiredState == LyraGameplayTags::InitState_Spawned) ...
if (CurrentState == LyraGameplayTags::InitState_DataAvailable) ...
```

**总指挥的状态机判断，全靠这些 tag 当"状态名"**。不 include `LyraGameplayTags.h`，代码里写 `LyraGameplayTags::InitState_Spawned` 就"未定义"编译不过。

> 所以这行 include 的真实目的：**拿到状态机要用的那 4 个状态 tag**（Spawned/DataAvailable/DataInitialized/GameplayReady），它们是状态链的"站名"（第 01/34 篇）。

---

## 四、第二行：`LyraLogChannels.h`（顺带讲）

这个文件和标签**没关系**，是**日志系统**的事：

```cpp
// LyraLogChannels.h 全部内容
DECLARE_LOG_CATEGORY_EXTERN(LogLyra, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogLyraExperience, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogLyraAbilitySystem, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogLyraTeams, Log, All);
```

**它干嘛？** 声明 Lyra 自己的**日志分类**（log category）。UE 打日志要用分类区分来源：

```cpp
// 不同模块打日志，用不同分类，方便过滤查看
UE_LOG(LogLyra, Log, TEXT("Pawn data set"));
UE_LOG(LogLyraAbilitySystem, Error, TEXT("Ability failed"));
```

`LyraPawnExtensionComponent.cpp` 里用过 `UE_LOG(LogLyra, ...)`（L89/125）——所以它 include 这个文件拿日志分类。

**类比**：`LyraLogChannels` = Lyra 给自己几个子系统各发了**一个专属的"日志分类章"**，打日志时盖章，这样 Output Log 里能按章筛选（只看能力系统的日志 / 只看队伍的日志）。

| 分类 | 管谁的日志 |
|---|---|
| `LogLyra` | 通用 |
| `LogLyraExperience` | Experience（玩法体验） |
| `LogLyraAbilitySystem` | 能力系统 |
| `LogLyraTeams` | 队伍系统 |

---

## 五、一张图总结（两个 include 各拿来干嘛）

```
LyraPawnExtensionComponent.cpp 的 L10~L11
────────────────────────────────────────────
#include "LyraGameplayTags.h"      #include "LyraLogChannels.h"
        │                                   │
        ▼                                   ▼
Lyra 的"标签清单"                      Lyra 的"日志分类"
（用官方 GameplayTags 系统              （UE 日志系统里
   登记 Lyra 自己的 tag）                声明 LogLyra 等分类）
        │                                   │
用途：状态机当"站名"                   用途：打日志时盖章
  LyraGameplayTags::InitState_Spawned    UE_LOG(LogLyra, ...)
  等 4 个状态 tag + 输入 tag 等           方便在 Output Log 筛选

     ┌─────────────────────────────────────────┐
     │ 都是"Lyra 自己的文件"                     │
     │ 但都不是"新系统"——                      │
     │  · GameplayTags 系统是官方的              │
     │  · 日志系统是官方的                       │
     │  · 这两个文件只是 Lyra 报的"户口/分类"     │
     └─────────────────────────────────────────┘
```

---

## 六、总结一句话

> **`LyraGameplayTags.h` 不是 Lyra 自造的标签系统，而是 Lyra 在官方 GameplayTags 系统里"登记自己那批 tag"的清单**（用官方的 `UE_DECLARE/DEFINE_GAMEPLAY_TAG` 宏，声明+定义约 40 个 tag，如 `InitState_Spawned`）。官方系统本身"默认没有业务标签"，所以项目要自己登记。总指挥 include 它，是为了用状态机的 4 个"站名"tag。而 `LyraLogChannels.h` 是另一个东西——Lyra 的日志分类声明（LogLyra 等），用于打日志时区分模块。**两个都是"Lyra 自己的文件"，但都只是借官方系统报户口，没有自创系统。**

---

## 七、下一步

- 打开 `LyraGameplayTags.cpp`，看 `UE_DEFINE_GAMEPLAY_TAG(InitState_Spawned, "InitState.Spawned")` 的真实写法，理解"字符串如何注册进官方系统"。
- 去引擎搜 `NativeGameplayTags.h`，看 `UE_DECLARE_GAMEPLAY_TAG_EXTERN` 宏怎么把声明变成 FGameplayTag 常量。
- 在 Output Log 里试 `Log LyraAbilitySystem` 过滤，感受"日志分类"的筛选作用。
