# P01 Messages（消息总线）—— 总 md

> **目录**：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Messages\`
> **对应工程**：`E:\ue5\game_design\code\Tenkaichi\Source\TenkaichiGame\Messages\`
> **版本**：重构版2 v1（2026-10-20）｜ **P01 是"消息总线"层**——游戏里各种事件（击杀、拾取、通知）靠它广播/订阅，是最底层的通信机制之一，排在 P00 Physics 之后。

---

## 一、这个文件夹解决什么问题（为什么）

游戏里"A 发生了什么事，B 想知道"——比如"有人被击杀了"，击杀者、被击杀者、观战者、UI 都想响应。如果都用直接引用，代码会写成一团乱麻（A 得知道所有关心它的 B）。

Lyra 用**消息总线**（GameplayMessageRouter 插件）解决：**发消息的人不关心谁在听，听消息的人不关心谁发的**。广播方喊一嗓子"XX 事件发生了"，订阅方按 Tag 过滤，各取所需。

`Messages/` 文件夹就定义这套消息的**数据结构 + 广播工具**：

1. **`FLyraVerbMessage`** —— 核心消息体："谁（Instigator）对谁（Target）做了什么（Verb），带多少数值（Magnitude）"。
2. **`LyraVerbMessageHelpers`** —— 工具函数：从 UObject 反推 PlayerState/PlayerController，在 VerbMessage 和 GameplayCue 之间互转。
3. **`FLyraVerbMessageReplication`** —— 把消息**从服务器复制到客户端**再广播（FastArraySerializer 增量复制）。
4. **`UGameplayMessageProcessor`** —— 消息处理器基类：监听消息，可能重新发出更新（如连击检测）。
5. **`FLyraNotificationMessage`** —— 瞬态日志消息（击杀播报、拾取提示），显示在 UI 上。

> 🔑 **一句话**：这个文件夹 = 游戏世界的"**广播电台**"——定义"消息长什么样"（VerbMessage / NotificationMessage），并配套"转发工具"（Helpers / Replication / Processor）。

---

## 二、全景：这个文件夹里有什么（9 个文件）

| 文件 | 大小 | 类型 | 干嘛的 |
|------|------|------|--------|
| `LyraVerbMessage.h` | 39 行 | 结构体声明 | 核心消息体 `FLyraVerbMessage`（Instigator/Verb/Target + 3 组 Tags + Magnitude；**无独立 `.cpp`**，`ToString()` 实现在 `LyraVerbMessageHelpers.cpp`） |
| `LyraVerbMessageHelpers.h` | 39 行 | 蓝图函数库 | `ULyraVerbMessageHelpers`：反推 PlayerState/Controller + VerbMessage↔GameplayCue 互转 |
| `LyraVerbMessageHelpers.cpp` | 97 行 | 实现 | 上面函数的实现 |
| `LyraVerbMessageReplication.h` | 87 行 | 结构体声明 | 消息复制容器 `FLyraVerbMessageReplication`（FastArraySerializer） |
| `LyraVerbMessageReplication.cpp` | 60 行 | 实现 | 复制/重广播实现 |
| `GameplayMessageProcessor.h` | 47 行 | 类声明 | 消息处理器基类 `UGameplayMessageProcessor` |
| `GameplayMessageProcessor.cpp` | 57 行 | 实现 | BeginPlay 开始监听 / EndPlay 停止监听 |
| `LyraNotificationMessage.h` | 40 行 | 结构体声明 | 瞬态日志消息 `FLyraNotificationMessage` + Tag 声明 |
| `LyraNotificationMessage.cpp` | 9 行 | 实现 | 定义 `TAG_Lyra_AddNotification_Message` |

**依赖树**（被依赖在下）：

```
GameplayMessageProcessor.h/.cpp          LyraVerbMessageReplication.h/.cpp
   └── GameplayMessageSubsystem（S1 插件✅）   └── LyraVerbMessage.h
        └── 引擎 GameplayMessageRouter         └── FastArraySerializer（引擎 Net）
                                                   └── 引擎 GameplayTagContainer

LyraVerbMessageHelpers.h/.cpp
   └── LyraVerbMessage.h
   └── FGameplayCueParameters（★GAS，P05）── ⚠️ .cpp 硬依赖

LyraNotificationMessage.h/.cpp
   └── NativeGameplayTags（引擎 GameplayTags 模块）
```

---

## 三、关键概念：VerbMessage 是什么

`FLyraVerbMessage` 是 Lyra 消息系统的**万能消息体**，一句话概括：

> **"谁（Instigator）用啥（Verb）对谁（Target）做了什么，带多少数值（Magnitude），以及各自的标签（InstigatorTags / TargetTags / ContextTags）。"**

字段拆解：

| 字段 | 类型 | 含义 |
|------|------|------|
| `Verb` | `FGameplayTag` | 事件类型标签（如 `Lyra.Elimination.Message`）——消息订阅方靠它过滤 |
| `Instigator` | `TObjectPtr<UObject>` | 发起者（谁干的） |
| `Target` | `TObjectPtr<UObject>` | 目标（对谁干的） |
| `InstigatorTags` | `FGameplayTagContainer` | 发起者身上的标签 |
| `TargetTags` | `FGameplayTagContainer` | 目标身上的标签 |
| `ContextTags` | `FGameplayTagContainer` | 环境/上下文标签 |
| `Magnitude` | `double` | 数值（如伤害量），默认 `1.0` |

> 💡 因为 `Verb` 是 GameplayTag，广播方发消息时用 `Verb` 做广播 Tag，订阅方就能**精确只收到自己关心的那类事件**。

---

## 四、关键概念：为什么需要 Helpers 和 Replication

**Helpers（为什么）**：消息里存的是 `TObjectPtr<UObject>`（可能是个 Pawn、PlayerState、Controller……），要拿到"这消息属于哪个玩家"很麻烦。`LyraVerbMessageHelpers` 提供统一工具：`GetPlayerStateFromObject()` / `GetPlayerControllerFromObject()` 自动向下兼容判断。还有一组函数在 **VerbMessage ↔ GameplayCue 之间互转**——这样同一件事既能走消息总线，也能走 GAS 的 GameplayCue。

**Replication（为什么）**：消息默认只在本地进程广播。服务器要告诉所有客户端"谁击杀了谁"，就得**复制**。`FLyraVerbMessageReplication` 用引擎的 `FastArraySerializer`（增量数组复制，只同步变化的部分）把消息从服务器推到客户端，客户端收到后再本地广播一次。

---

## 五、怎么教（推进节奏 + 依赖挂起）

按铁律 35/20：**先建本总 md（已建）→ 停，等确认 → 逐个文件教 `.h` → 教 `.cpp`**。

P01 拆 5 个教学单元（每个先总 md 拆开，按依赖顺序）：

| 步骤 | 内容 | 状态 | 依赖/挂起 |
|------|------|------|-----------|
| P01.1 | `LyraVerbMessage.h`（核心消息体） | ✅ 已教完 `.h`（纯头文件，`ToString()` 实现并入 P01.5） | 纯引擎 ✅ 可编译 |
| P01.2 | `LyraNotificationMessage.h/.cpp`（瞬态日志消息） | ✅ 已教完（总md + `.h` + `.cpp`，纯引擎） | 纯引擎 ✅ 可编译 |
| P01.3 | `LyraVerbMessageReplication.h/.cpp`（消息复制） | ✅ 已教完（总md + `.h` + `.cpp`，FastArray 消息复制器） | 依赖 P01.1 + S1 插件 ✅ 可编译 |
| P01.4 | `GameplayMessageProcessor.h/.cpp`（消息处理器基类） | ✅ 已教完（总md + `.h` + `.cpp`，消息处理器基类） | 依赖 S1 插件 ✅ 可编译 |
| P01.5 | `LyraVerbMessageHelpers.h/.cpp`（工具函数库，**含 `FLyraVerbMessage::ToString()` 实现**） | ✅ 已教完（总md + `.h` + `.cpp`；`.cpp` 中 `ToString()`+两个 Player 函数可编译，两个 GAS 桥接函数 `#if 0` 挂起，已登记注释备忘录⑬） | ⚠️ GAS 桥接函数依赖 GAS `FGameplayCueParameters`（P05），注释挂起 |

> ⚠️ 挂起说明：`LyraVerbMessageHelpers.cpp` 里 `VerbMessageToCueParameters` / `CueParametersToVerbMessage` 两个函数**硬调用 GAS 的 `FGameplayCueParameters`**（`GameplayEffectTypes.h`），而 GAS 核心（P05）还没教。按铁律 33/死结处理铁律：**代码一比一完整写上，但这几个 GAS 相关 `.cpp` 段注释挂起**，登记注释备忘录，到 P05 就位后解开。

---

## 六、依赖分析总结（铁律 30/31 核查）

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `LyraVerbMessage.h` | 无 | ✅ 能 | 否 |
| `LyraNotificationMessage.h/.cpp` | 无 | ✅ 能 | 否 |
| `LyraVerbMessageReplication.h/.cpp` | `LyraVerbMessage` + S1 插件 | ✅ 能 | 否 |
| `GameplayMessageProcessor.h/.cpp` | S1 插件 | ✅ 能 | 否 |
| `LyraVerbMessageHelpers.h/.cpp` | GAS `FGameplayCueParameters` | ⚠️ `.cpp` 不能 | **`.cpp` 挂起**（GAS 相关段） |

**结论**：P01 大部分可独立编译；仅 `LyraVerbMessageHelpers.cpp` 因 GAS 硬依赖需**注释挂起**（铁律 33），到 P05 GAS 核心就位后解开。

---

**下一步**：✅ **P01 Messages 整章已全部教完**（P01.1~P01.5 总md + `.h` + `.cpp`；仅 P01.5 两个 GAS 桥接函数挂起待 P05 解开）。进 **P02**（System 叶子：`GameplayTagStack`/`ActorUtilities`/`SystemStatics`）。