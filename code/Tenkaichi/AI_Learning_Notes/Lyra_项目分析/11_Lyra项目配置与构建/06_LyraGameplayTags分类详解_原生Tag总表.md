# LyraGameplayTags 分类详解 —— 原生 Tag 总表

> **定位**：把 `LyraGameplayTags.h/.cpp` 里 Epic 声明的所有**原生（Native）GameplayTag** 按功能分类讲清。
>
> **源码位置**：
> - 声明：`Source/LyraGame/LyraGameplayTags.h`
> - 定义：`Source/LyraGame/LyraGameplayTags.cpp`
>
> **一句话**：这些 Tag 是 Lyra 用 C++ 硬编码注册的"官方标签"，覆盖**技能、输入、初始化、事件、数值、作弊、状态、移动**八大类，是 GAS 与各系统通信的"通用语言"。

---

## 一、先搞懂：什么是 Native Gameplay Tag？

Lyra 里的 Tag 有两种来源：

| 来源 | 说明 | 例子 |
|------|------|------|
| **Native Tag（本文）** | 在 C++ 里用 `UE_DECLARE_GAMEPLAY_TAG_EXTERN` 声明、`UE_DEFINE_GAMEPLAY_TAG_COMMENT` 定义 | `Ability.ActivateFail.IsDead` |
| **配置文件 Tag** | 在 `Config/DefaultGameplayTags.ini` 里配置的 | 各种 `GameplayCue.*`、`Input.*` 等 |

**Native Tag 的好处**：
- 编译期就确定，**打错字也不会运行时才发现**（有编译检查）。
- 代码里直接用变量名引用（如 `LyraGameplayTags::Status_Death`），**跳转/重构方便**。
- 自带注释说明用途。

> 宏拆解（回顾）：
> ```cpp
> UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death);   // .h 里：声明"外部有个叫 Status_Death 的 Tag 变量"
> UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death, "Status.Death", "Target has the death status.");
>   // .cpp 里：真正定义，字符串是 "Status.Death"，注释是后面那句
> ```

---

## 二、八大分类总览

| 分类 | 前缀 | 数量 | 作用 |
|------|------|------|------|
| ① 技能失败原因 | `Ability.ActivateFail.*` | 7 | 说明技能为何激活失败 |
| ② 技能行为 | `Ability.Behavior.*` | 1 | 标记技能的特殊行为 |
| ③ 输入标签 | `InputTag.*` | 5 | 绑定输入动作到 Tag |
| ④ 初始化状态 | `InitState.*` | 4 | 驱动组件/角色的分阶段初始化 |
| ⑤ 游戏事件 | `GameplayEvent.*` | 3 | 触发 GAS 事件 |
| ⑥ SetByCaller 数值 | `SetByCaller.*` | 2 | 动态传入伤害/治疗数值 |
| ⑦ 作弊 | `Cheat.*` | 2 | 标记作弊状态 |
| ⑧ 状态 | `Status.*` | 5 | 标记角色当前状态 |
| ⑨ 移动模式 | `Movement.Mode.*` | 6 | 映射 UE 移动模式枚举 |

> 注：⑨ 移动模式严格说是"枚举映射表"，不是普通状态 Tag，单独一类。

---

## 三、逐类详解

### ① 技能激活失败原因 `Ability.ActivateFail.*`（7 个）

技能激活失败时，GAS 会打上对应 Tag，方便调试/UI 提示"为什么没放出来"。

| C++ 变量名 | Tag 字符串 | 含义 |
|-----------|-----------|------|
| `Ability_ActivateFail_IsDead` | `Ability.ActivateFail.IsDead` | 拥有者已死亡 |
| `Ability_ActivateFail_Cooldown` | `Ability.ActivateFail.Cooldown` | 还在冷却中 |
| `Ability_ActivateFail_Cost` | `Ability.ActivateFail.Cost` | 资源/消耗不足 |
| `Ability_ActivateFail_TagsBlocked` | `Ability.ActivateFail.TagsBlocked` | 被其他 Tag 屏蔽 |
| `Ability_ActivateFail_TagsMissing` | `Ability.ActivateFail.TagsMissing` | 缺少所需 Tag |
| `Ability_ActivateFail_Networking` | `Ability.ActivateFail.Networking` | 网络检查未通过 |
| `Ability_ActivateFail_ActivationGroup` | `Ability.ActivateFail.ActivationGroup` | 激活组冲突 |

> **用途**：这些 Tag 会被 GAS 内部用作 `ActivationBlockedTags` / 失败原因，排查"技能放不出来"时看它们。

---

### ② 技能行为 `Ability.Behavior.*`（1 个）

| C++ 变量名 | Tag 字符串 | 含义 |
|-----------|-----------|------|
| `Ability_Behavior_SurvivesDeath` | `Ability.Behavior.SurvivesDeath` | 带此 Tag 的技能**不会因拥有者死亡而被取消** |

> **用途**：某些技能（如死亡瞬间的终结技、复活技）需要"死后继续跑完"，就打上这个行为 Tag。

---

### ③ 输入标签 `InputTag.*`（5 个）

把"输入动作"抽象成 Tag，让 Enhanced Input 与 GAS 解耦——输入事件用 Tag 传递，而不是硬编码枚举。

| C++ 变量名 | Tag 字符串 | 含义 |
|-----------|-----------|------|
| `InputTag_Move` | `InputTag.Move` | 移动输入 |
| `InputTag_Look_Mouse` | `InputTag.Look.Mouse` | 视角输入（鼠标） |
| `InputTag_Look_Stick` | `InputTag.Look.Stick` | 视角输入（摇杆） |
| `InputTag_Crouch` | `InputTag.Crouch` | 下蹲输入 |
| `InputTag_AutoRun` | `InputTag.AutoRun` | 自动奔跑输入 |

> **用途**：在输入配置（`DefaultInput.ini` / DataAsset）里，把按键映射到这些 Tag；代码里监听 Tag 触发对应逻辑。鼠标和摇杆分开，是为了分别处理两种设备的视角输入。

---

### ④ 初始化状态 `InitState.*`（4 个）★重点

这是 Lyra 的**分阶段初始化状态机**，配合之前学的 `IGameFrameworkInitStateInterface` + 大管家使用。

| C++ 变量名 | Tag 字符串 | 阶段 | 含义 |
|-----------|-----------|------|------|
| `InitState_Spawned` | `InitState.Spawned` | 1 | 已生成，可被扩展 |
| `InitState_DataAvailable` | `InitState.DataAvailable` | 2 | 所需数据已加载/同步完毕 |
| `InitState_DataInitialized` | `InitState.DataInitialized` | 3 | 数据已初始化，但还没完全就绪 |
| `InitState_GameplayReady` | `InitState.GameplayReady` | 4 | 完全就绪，可参与游戏 |

> **用途**：角色/组件从出生到"能玩"要经过 4 个阶段，大管家按顺序协调，避免"数据还没加载完就开始玩"的时序 bug。**这是 Lyra 架构的精髓之一。**

---

### ⑤ 游戏事件 `GameplayEvent.*`（3 个）

GAS 里用 `GameplayEvent` 触发技能/逻辑。

| C++ 变量名 | Tag 字符串 | 含义 |
|-----------|-----------|------|
| `GameplayEvent_Death` | `GameplayEvent.Death` | 死亡事件（**仅服务器触发**） |
| `GameplayEvent_Reset` | `GameplayEvent.Reset` | 玩家重置执行时触发 |
| `GameplayEvent_RequestReset` | `GameplayEvent.RequestReset` | 请求把 Pawn 立即替换到有效出生点 |

> **用途**：死亡、重置这类全局性动作，通过事件 Tag 广播，能监听它的技能/组件各自响应。

---

### ⑥ SetByCaller 数值 `SetByCaller.*`（2 个）

GAS 的 **SetByCaller** 机制：在运行时动态给 GameplayEffect 传数值（而不是写死）。

| C++ 变量名 | Tag 字符串 | 含义 |
|-----------|-----------|------|
| `SetByCaller_Damage` | `SetByCaller.Damage` | 伤害数值通道 |
| `SetByCaller_Heal` | `SetByCaller.Heal` | 治疗数值通道 |

> **用途**：同一个"扣血/加血"的 GameplayEffect，用 `SetByCaller.Damage` / `SetByCaller.Heal` 这个 Tag 作为"数据槽名"，运行时告诉它"这次扣多少/加多少"。**一个 Effect 复用给多种数值场景。**

---

### ⑦ 作弊 `Cheat.*`（2 个）

| C++ 变量名 | Tag 字符串 | 含义 |
|-----------|-----------|------|
| `Cheat_GodMode` | `Cheat.GodMode` | 无敌模式开启 |
| `Cheat_UnlimitedHealth` | `Cheat.UnlimitedHealth` | 无限血量开启 |

> **用途**：调试/测试时给拥有者打上作弊 Tag，逻辑层据此跳过伤害、锁定血量等。

---

### ⑧ 状态 `Status.*`（5 个）

标记角色当前"处于什么状态"，GAS 用它们做 `ActivationBlockedTags`（比如死亡时不能放技能）。

| C++ 变量名 | Tag 字符串 | 含义 |
|-----------|-----------|------|
| `Status_Crouching` | `Status.Crouching` | 正在下蹲 |
| `Status_AutoRunning` | `Status.AutoRunning` | 正在自动奔跑 |
| `Status_Death` | `Status.Death` | 处于死亡状态 |
| `Status_Death_Dying` | `Status.Death.Dying` | 死亡过程已开始（濒死） |
| `Status_Death_Dead` | `Status.Death.Dead` | 死亡过程已结束（已死） |

> **用途**：`Status.*` 是"事实状态"，配合技能里的 `BlockedTags`，实现"死亡/蹲下时不能放某些技能"这类约束。死亡拆成 Dying/Dead 两阶段，能精细控制"濒死还能做点什么"。

---

### ⑨ 移动模式 `Movement.Mode.*`（6 个）★特殊

这一类不是普通状态 Tag，而是**把 UE 的 `EMovementMode` 枚举映射成 Tag**，方便用 Tag 统一处理移动状态。

| C++ 变量名 | Tag 字符串 | 对应枚举 |
|-----------|-----------|---------|
| `Movement_Mode_Walking` | `Movement.Mode.Walking` | `MOVE_Walking` |
| `Movement_Mode_NavWalking` | `Movement.Mode.NavWalking` | `MOVE_NavWalking` |
| `Movement_Mode_Falling` | `Movement.Mode.Falling` | `MOVE_Falling` |
| `Movement_Mode_Swimming` | `Movement.Mode.Swimming` | `MOVE_Swimming` |
| `Movement_Mode_Flying` | `Movement.Mode.Flying` | `MOVE_Flying` |
| `Movement_Mode_Custom` | `Movement.Mode.Custom` | `MOVE_Custom`（占位，需替换为自定义 Tag） |

**关键——它俩是"映射表"，不是手动打的 Tag：**

```cpp
// .cpp 里的两张映射表：把枚举值 → Tag
const TMap<uint8, FGameplayTag> MovementModeTagMap = {
    { MOVE_Walking,    Movement_Mode_Walking },
    { MOVE_NavWalking, Movement_Mode_NavWalking },
    { MOVE_Falling,    Movement_Mode_Falling },
    { MOVE_Swimming,   Movement_Mode_Swimming },
    { MOVE_Flying,     Movement_Mode_Flying },
    { MOVE_Custom,     Movement_Mode_Custom }
};

const TMap<uint8, FGameplayTag> CustomMovementModeTagMap = {
    // 扩展 Lyra 时，把你的自定义移动模式填进来
};
```

> **用途**：角色移动模式切换时，代码查表把枚举转成 Tag，就能用 Tag 统一做 GAS 逻辑（如"飞行时不能放某技能"）。`Custom` 是占位符——因为 `MOVE_Custom` 有 4 个子模式（Custom1~4），必须替换成你自己定义的具体 Tag。

---

## 四、一张全景图（按 Tag 层级树）

```
Lyra 原生 GameplayTags
│
├─ Ability（技能）
│   ├─ ActivateFail（失败原因）
│   │   ├─ IsDead / Cooldown / Cost
│   │   └─ TagsBlocked / TagsMissing / Networking / ActivationGroup
│   └─ Behavior（行为）
│       └─ SurvivesDeath（死后不取消）
│
├─ InputTag（输入）
│   ├─ Move / Crouch / AutoRun
│   └─ Look.Mouse / Look.Stick
│
├─ InitState（初始化 4 阶段）★
│   └─ Spawned → DataAvailable → DataInitialized → GameplayReady
│
├─ GameplayEvent（事件）
│   └─ Death / Reset / RequestReset
│
├─ SetByCaller（GAS 数值通道）
│   └─ Damage / Heal
│
├─ Cheat（作弊）
│   └─ GodMode / UnlimitedHealth
│
├─ Status（状态）
│   ├─ Crouching / AutoRunning
│   └─ Death（.Dying / .Dead）
│
└─ Movement.Mode（移动模式映射）★
    └─ Walking / NavWalking / Falling / Swimming / Flying / Custom
```

---

## 五、怎么找到自己项目里的 Tag？

除了本文讲的 Native Tag，Lyra 还有**大量 Tag 在配置文件**里：

```
Config/DefaultGameplayTags.ini
```

那里能看到 `GameplayCue.*`、`Input.*`（成体系的输入 Tag）、`UI.*` 等。Native Tag 只是"冰山一角"，但它们是**代码里最常用、最核心**的那批。

> **查找技巧**：
> - C++ 里用 `LyraGameplayTags::` 前缀跳转，能看到所有 Native Tag。
> - 想看全量 Tag，用 `UGameplayTagsManager` 或编辑器 Project Settings → GameplayTags 面板。

---

## 六、总结（记忆要点）

```
LyraGameplayTags = Lyra 用 C++ 硬编码的"官方标签库"
分 8~9 大类：
  ① Ability.ActivateFail.*  技能为何失败（7）
  ② Ability.Behavior.*      技能特殊行为（1）
  ③ InputTag.*              输入动作抽象（5）
  ④ InitState.*             4 阶段初始化状态机 ★
  ⑤ GameplayEvent.*         GAS 事件触发（3）
  ⑥ SetByCaller.*           动态数值通道（2）
  ⑦ Cheat.*                 作弊标记（2）
  ⑧ Status.*                角色事实状态（5）
  ⑨ Movement.Mode.*         移动枚举→Tag 映射表 ★

核心思想：Tag 是 GAS 各系统通信的"通用语言"，
        Native Tag 让核心标签在编译期就确定、可跳转、带注释。
```

**一句话**：`LyraGameplayTags` 把 Lyra 的核心标签按前缀分成**技能失败/行为、输入、初始化状态、事件、数值、作弊、状态、移动模式**几大类；其中 `InitState.*`（4 阶段初始化）和 `Movement.Mode.*`（枚举映射表）最特殊、最值得记住——前者是 Lyra 架构精髓，后者是"枚举转 Tag"的设计范式。

---

## 七、下一步

- 看 `Config/DefaultGameplayTags.ini` 里配置的完整 Tag 列表（尤其是 `Input.*` 和 `GameplayCue.*`）。
- 回顾 `IGameFrameworkInitStateInterface` + 大管家如何用 `InitState.*` 驱动初始化（见 06_Lyra功能总览 相关篇）。
- 看 GAS 里 `ActivationBlockedTags` 如何用 `Status.*` / `Ability.ActivateFail.*` 做技能约束。
