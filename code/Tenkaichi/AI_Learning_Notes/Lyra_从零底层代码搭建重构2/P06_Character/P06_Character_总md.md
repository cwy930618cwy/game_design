# P06 Character —— 阶段总 md

> **定位**：P06 Character 阶段的**目录/路线图**（这一节有哪些类、各依赖什么、先教谁）。
> 具体某一个类怎么教，由**该类自己的总 md** 负责（铁律 35：上级总 md 不能替代子类总 md）。

---

## 一、这一节要解决什么

P05 把 GAS 核心（ASC / GA / 属性集 / 效果上下文 / 技能）全部搭好了。
但 **GAS 是"能力系统"，它自己不会主动挂到角色身上** —— P06 就是**把 GAS 接到角色上**，并补上最后一个关键组件：**血量**。

一句话：**P06 = 让角色真正"能用技能 + 有血量 + 会死"**。

---

## 二、全景：Lyra `Character/` 文件夹 16 个文件

对照 `E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Character\`：

| 编号 | Lyra 文件 | 行数(h/cpp) | 工程现状 | 状态 |
|------|----------|------------|---------|------|
| **P06.1** | `LyraPawnData` | 1.7K / 332B | ✅ 已落盘 | ✅ 已完成 |
| **P06.2** | `LyraPawnExtensionComponent` | 4.6K / 9.5K | ✅ 已落盘（`.cpp` 整体 `#if 0` 挂起） | ✅ 已教（挂起⑥） |
| **P06.3** | `LyraHeroComponent` | 4.4K / 16.6K | ✅ 已落盘（`.h`+`.cpp` 全挂起） | ✅ 已教（挂起⑦） |
| **P06.4** | `LyraPawn` | 1.9K / 3.1K | ✅ 已落盘 | ✅ 已完成 |
| **P06.5** | `LyraCharacter` | 8.0K / 21.0K | ✅ 已落盘（`.cpp` 整体 `#if 0`） | ✅ 已教（挂起⑧） |
| **P06.6** | `LyraCharacterWithAbilities` | 1.2K / 1.5K | ✅ 已落盘（`.cpp` 整体 `#if 0`） | ✅ 已教（挂起⑨） |
| **P06.7** | `LyraCharacterMovementComponent` | 2.1K / 4.1K | ✅ 已落盘 | ✅ 已完成 |
| **P06.8** | **`LyraHealthComponent`** | **4.6K / 10.4K** | ❌ **工程不存在** | 🟡 **总 md ✅**（`.h` 135 行零挂起 / `.cpp` 312 行挂起 ㉘ 待 P14） |

> 工程 `Character\` 现有 14 个文件，Lyra 有 16 个 —— **差的正是 `HealthComponent.h/.cpp` 这一对**。

---

## 三、依赖分析（铁律 30/31，已读 `LyraHealthComponent.h` 135 行 + `.cpp` 全文 312 行）

统一方向依赖树（被依赖在下）：

```
ULyraHealthComponent（P06.8，本阶段唯一待教）
    │ 继承
    ▼
UGameFrameworkComponent（引擎 ModularGameplay 插件 ①，只讲不拆）
    │
    ├── .h 前置声明 ──► ULyraAbilitySystemComponent（P05.2 ✅）
    │                   ULyraHealthSet（P05.4-1 ✅）
    │
    └── .cpp 硬调用 ──► LyraAttributeSet.h（P05.4-1 ✅）
                        LyraLogChannels.h（第 0 步 ✅）
                        LyraGameplayTags.h（第 0 步 ✅）
                        LyraAbilitySystemComponent.h（P05.2 ✅）
                        LyraHealthSet.h（P05.4-1 ✅）
                        Messages/LyraVerbMessage.h（P01 ✅）
                        Messages/LyraVerbMessageHelpers.h（P01 ✅）
                        GameFramework/GameplayMessageSubsystem.h（S1 插件 ✅）
                        ❌ System/LyraAssetManager.h（P14，工程无！）
                        ❌ System/LyraGameData.h（P02.2 ✅ 已落盘）
```

| 依赖 | 用在哪 | 状态 |
|------|--------|------|
| `UTenkaichiAbilitySystemComponent`（P05.2） | `.h` 成员 `TObjectPtr` + `.cpp` 硬调用 | ✅ 已落盘 |
| `UTenkaichiHealthSet`（P05.4-1） | `.h` 成员 + `.cpp` 取血量 | ✅ 已落盘（⚠️ 在 `AbilitySystem\` 根目录，应移到 `Attributes\`） |
| `TenkaichiAttributeSet.h`（P05.4-1） | `.cpp` include | ✅ 已落盘 |
| `TenkaichiLogChannels.h`（第 0 步） | `UE_LOG(LogTenkaichi, ...)` | ✅ 已就位 |
| `TenkaichiGameplayTags.h`（第 0 步） | Tag 常量 | ✅ 已就位 |
| `Messages/TenkaichiVerbMessage.h` + `Helpers.h`（P01） | `HandleOutOfHealth` 里广播击杀消息 | ✅ 已落盘 |
| `GameFramework/GameplayMessageSubsystem.h`（S1） | `Get` + `BroadcastMessage` | ✅ 已还原 |
| `System/TenkaichiGameData.h`（P02.2） | `DamageSelfDestruct` 里取 GE | ✅ 已落盘（⚠️ 但 `Get()` 挂起 ⑮） |
| **`System/TenkaichiAssetManager.h`（P14）** | **`DamageSelfDestruct` 里 `GetSubclass`** | ❌ **工程不存在** |

**逐文件结论**：

| 文件 | 结论 |
|------|------|
| **`.h`** | 全部依赖已就位（ASC/HealthSet 只前置声明）→ **零挂起** ✅ |
| **`.cpp`** | 大部分可编译；**`DamageSelfDestruct` 函数体 + `System/TenkaichiAssetManager.h` include 需挂起**（依赖 P14 `ULyraAssetManager`）→ 登记**注释备忘录 ㉘** |

---

## 四、🔓 阶段开头必做：翻注释备忘录（铁律 34）

P06 是**多个挂起项的解锁时机**。请先处理这些：

| 条目 | 内容 | 状态 |
|------|------|------|
| **㉖** | `TenkaichiGameplayAbility_Death.cpp`：HealthComponent include + `StartDeath`/`FinishDeath` 函数体 + `EndAbility` 里的 `FinishDeath();` 调用行 | 🔓 **依赖本阶段 `UTenkaichiHealthComponent`** → **教完 HealthComponent 即可解开** |
| **㉗** | `TenkaichiGameplayAbility_Reset.cpp`：`TenkaichiChar->Reset();` 一行 | 🔓 **依赖本阶段 `ATenkaichiCharacter::Reset()` 实现放开**（`TenkaichiCharacter.cpp` 整体 `#if 0` 解开时） |
| ⑥ | `TenkaichiPawnExtensionComponent.cpp` 整个 `.cpp` | ✅ **已就位（P05.2 ASC + P06.8 HealthComponent 全就位）→ 可放开** |
| ⑧ | `TenkaichiCharacter.cpp` 整个 `.cpp` | 🔓 **P05 GAS 核心已就位** → 差 Camera/PlayerController/PlayerState/PawnExtension，**可尝试解开** |
| ⑨ | ~~`TenkaichiCharacterWithAbilities.cpp` 整个 `.cpp`~~ | 🔓 **P05 ASC + HealthSet + CombatSet 已就位** → **现在可解开** |
| ㉒ | `TenkaichiGameplayAbility.cpp` 6 函数 + 2 include | 🔓 **P05.2 + P05.5 双就位，待你放开** |
| ㉔ | `TenkaichiAbilitySystemGlobals.cpp` `AllocGameplayEffectContext` + include | 🔓 **P05.5 已就位，待你放开** |
| ⑦ | `TenkaichiHeroComponent.h` + `.cpp` | ⏸ 仍需 **P15 GameFeatures**，本阶段不解 |
| ㉘ | （待新增）`TenkaichiHealthComponent.cpp` 的 `DamageSelfDestruct` + AssetManager include | 🆕 本阶段教到时登记，依赖 P14 |

> ⚠️ **按死结处理铁律**：这些挂起**不是障碍**，解开时机到了就放开；没到的（⑦ ㉘）继续挂起，**不停下、不纠结**。

---

## 五、教学顺序（铁律 20/29/35）

**每个类都要走「自己的总 md → `.h` → `.cpp`」三步**。

| 序 | 内容 | 状态 |
|----|------|------|
| 0 | 本阶段总 md（目录 + 依赖 + 挂起提醒） | ✅ 已建 |
| 1 | **P06.8 `ULyraHealthComponent`** ✅ **全部教完**（总 md ✅ + `.h` ✅ 工程已落盘核对通过 + `.cpp` ✅ `P06.8_TenkaichiHealthComponent_cpp.md`，312 行，仅 `DamageSelfDestruct` + AssetManager include 挂起 **㉘**） |
| 2 | 解挂起：**㉖ ㉒ ㉔ ⑨ ⑥ ㉗ ⑧ 已解开** ✅（2026-10-26）｜**仅剩 ㉙**（Significance，待 P14） | ✅ **已完成** |
| 3 | P06 收官 → 进 P07 Teams | ⬜ |

> **为什么先教 HealthComponent**：它是 P06 唯一没落盘的类，且是 **㉖（Death 技能）的解锁钥匙**。

---

## 六、几个要记住的点

1. **P06 = GAS 接入角色 + 血量组件**。GAS 本身（P05）不会自己挂上来。
2. **HealthComponent 是桥梁**：把 `UTenkaichiHealthSet`（GAS 侧数值）翻译成"游戏层的血量/死亡事件"。
3. **`ELyraDeathState` 三态**：`NotDead` → `DeathStarted` → `DeathFinished`，和 Death 技能的 `StartDeath`/`FinishDeath` 一一对应。
4. **`UGameFrameworkComponent` 基类**（引擎 ①，只讲不拆）—— 配合 `ModularGameplay` 总线的初始化时机。
5. **只差 `HealthComponent` 一对文件** —— 工程 Character 目录 14/16。
6. **⚠️ 老待办**：`TenkaichiAttributeSet` / `TenkaichiCombatSet` / `TenkaichiHealthSet` 仍在 `AbilitySystem\` 根目录，应按 Lyra 移到 `AbilitySystem\Attributes\`。

---

## 七、节奏

你先看这份阶段总 md（含**挂起解开通知**），确认"懂了 / 下一步"，我再建 **P06.8 `ULyraHealthComponent` 它自己的总 md**。

> 文件落盘路径：`Source\TenkaichiGame\Character\TenkaichiHealthComponent.h/.cpp`。
