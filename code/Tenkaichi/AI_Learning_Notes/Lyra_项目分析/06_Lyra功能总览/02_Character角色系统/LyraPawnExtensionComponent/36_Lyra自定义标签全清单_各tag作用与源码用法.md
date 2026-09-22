# 36 — Lyra 自定义的标签全清单：每个 tag 是啥、干嘛用、源码怎么用

> **定位**：上一篇澄清了"系统是官方的、Lyra 只是报户口"。这篇把 **Lyra 报的那些户口（tag）全部列出来**：每个 tag 的 C++ 常量名、字符串名、注释含义、在代码里怎么被用。
>
> 数据来源：`LyraGameplayTags.h`（声明）+ `LyraGameplayTags.cpp`（定义/注释，真源码）。
>
> **衔接**：第 35 篇（系统 vs 清单）。这篇是清单本体。

---

## 〇、先看总貌：Lyra 的 tag 分几类？

从 `.h` 的声明顺序看，Lyra 的标签大致分 **7 组**（注意字符串的点分命名 = 层级）：

```
Ability.*          能力相关（激活失败原因 / 行为标签）
InputTag.*         输入标签（按键绑定到动作）
InitState.*        初始化状态（状态机的 4 个"站名"！）
GameplayEvent.*    游戏事件（死亡/重置）
SetByCaller.*      效果数值标签（伤害/治疗量用 SetByCaller 传）
Cheat.*            作弊标签（GodMode/无限血）
Status.*           状态标签（蹲下/疾跑/死亡）
Movement.Mode.*    移动模式标签（走路/坠落/游泳/飞行）
```

---

## 一、逐组列出（真实源码：名字 + 字符串 + 注释）

### 组 1：`Ability.ActivateFail.*` —— 技能为啥没放出来（7 个）

源码（`LyraGameplayTags.cpp` L11~17）：
```cpp
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead,        "Ability.ActivateFail.IsDead",   "owner is dead");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cooldown,      "Ability.ActivateFail.Cooldown", "on cool down");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cost,          "Ability.ActivateFail.Cost",     "did not pass cost checks");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsBlocked,   "Ability.ActivateFail.TagsBlocked", "tags blocking it");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsMissing,   "Ability.ActivateFail.TagsMissing", "tags missing");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Networking,    "Ability.ActivateFail.Networking", "network checks");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_ActivationGroup,"Ability.ActivateFail.ActivationGroup", "activation group");
```

| C++ 常量 | 含义（注释直译） |
|---|---|
| `Ability_ActivateFail_IsDead` | 激活失败：施法者已死 |
| `Ability_ActivateFail_Cooldown` | 激活失败：技能在冷却 |
| `Ability_ActivateFail_Cost` | 激活失败：没通过消耗检查（蓝不够/材料不够） |
| `Ability_ActivateFail_TagsBlocked` | 激活失败：被某些 tag 挡着（如被眩晕状态挡） |
| `Ability_ActivateFail_TagsMissing` | 激活失败：缺少必要 tag |
| `Ability_ActivateFail_Networking` | 激活失败：网络校验没过 |
| `Ability_ActivateFail_ActivationGroup` | 激活失败：激活分组冲突（第 14 篇） |

**源码用法**（参考第 14 篇 `CanActivateAbility`）：
```cpp
// 激活分组被挡时，往 OptionalRelevantTags 里塞"失败原因"tag
OptionalRelevantTags->AddTag(LyraGameplayTags::Ability_ActivateFail_ActivationGroup);
return false;
```
> 作用：**告诉 UI/上层"这技能为什么没放出来"**——比如蓝不够就提示"法力不足"，冷却中就显示 CD。

### 组 2：`Ability.Behavior.SurvivesDeath` —— 死亡也不取消的能力

源码（L19）：
```cpp
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Behavior_SurvivesDeath, "Ability.Behavior.SurvivesDeath",
	"An ability with this type tag should not be canceled due to death.");
```
| C++ 常量 | 含义 |
|---|---|
| `Ability_Behavior_SurvivesDeath` | 带此标签的能力**不会被死亡取消**（如复活被动） |

**源码用法**（之前第 01 篇 UninitializeAbilitySystem 见过）：
```cpp
FGameplayTagContainer AbilityTypesToIgnore;
AbilityTypesToIgnore.AddTag(LyraGameplayTags::Ability_Behavior_SurvivesDeath);
AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);  // 取消技能时跳过这些
```

### 组 3：`InputTag.*` —— 输入标签（5 个）

源码（L21~25）：
```cpp
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move,        "InputTag.Move",       "Move input.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse,  "InputTag.Look.Mouse", "Look (mouse) input.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Stick,  "InputTag.Look.Stick", "Look (stick) input.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Crouch,      "InputTag.Crouch",     "Crouch input.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_AutoRun,     "InputTag.AutoRun",    "Auto-run input.");
```

| C++ 常量 | 含义 |
|---|---|
| `InputTag_Move` | 移动输入 |
| `InputTag_Look_Mouse` | 视角（鼠标） |
| `InputTag_Look_Stick` | 视角（手柄摇杆） |
| `InputTag_Crouch` | 蹲下 |
| `InputTag_AutoRun` | 自动奔跑 |

**源码用法**（第 24/26 篇 HeroComponent 绑输入见过）：
```cpp
LyraIC->BindNativeAction(InputConfig, LyraGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, false);
LyraIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, ...);
```
> 作用：**把"输入配置资产（InputConfig）里的动作"和"某个 InputTag"对应起来**，按键按下时系统按 tag 找到对应处理函数。

### 组 4：`InitState.*` —— 初始化状态（4 个，最熟）

源码（L27~30，注释自带顺序号）：
```cpp
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned",
	"1: Actor/component has initially spawned and can be extended");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable",
	"2: All required data has been loaded/replicated and is ready for initialization");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized",
	"3: The available data has been initialized ... not ready for full gameplay");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady",
	"4: The actor/component is fully ready for active gameplay");
```

| C++ 常量 | 顺序 | 含义（注释自带编号） |
|---|---|---|
| `InitState_Spawned` | 1 | 已生成，可被扩展 |
| `InitState_DataAvailable` | 2 | 所需数据都加载/复制好了，可初始化 |
| `InitState_DataInitialized` | 3 | 数据已初始化，但还不能完整游玩 |
| `InitState_GameplayReady` | 4 | 完全就绪，可进行游戏 |

**源码用法**（第 23/34 篇状态机核心，总指挥大量使用）：
```cpp
static const TArray<FGameplayTag> StateChain = {
	LyraGameplayTags::InitState_Spawned,
	LyraGameplayTags::InitState_DataAvailable,
	LyraGameplayTags::InitState_DataInitialized,
	LyraGameplayTags::InitState_GameplayReady
};   // CheckDefaultInitialization 里的状态链
```
> 作用：**初始化状态机的"4 个站名"**（第 01 篇）。字符串名里的"点分 + 数字注释"就是告诉大家顺序。

### 组 5：`GameplayEvent.*` —— 游戏事件（3 个）

源码（L32~34）：
```cpp
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Death,       "GameplayEvent.Death",       "Event that fires on death. Only fires on the server.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Reset,       "GameplayEvent.Reset",       "Event that fires once a player reset is executed.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_RequestReset, "GameplayEvent.RequestReset", "Event to request a player's pawn to be instantly replaced with a new one...");
```

| C++ 常量 | 含义 |
|---|---|
| `GameplayEvent_Death` | 死亡事件（**只在服务器**触发） |
| `GameplayEvent_Reset` | 玩家重置后触发 |
| `GameplayEvent_RequestReset` | 请求重置：让玩家换一个新 Pawn |

> 作用：**游戏事件总线用 tag 做"频道名"**（配合 LyraVerbMessage / GameplayMessageSubsystem 广播）。

### 组 6：`SetByCaller.*` —— 效果的"数值要填多大量"（2 个）

源码（L36~37）：
```cpp
UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Damage, "SetByCaller.Damage", "SetByCaller tag used by damage gameplay effects.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Heal,   "SetByCaller.Heal",   "SetByCaller tag used by healing gameplay effects.");
```

| C++ 常量 | 含义 |
|---|---|
| `SetByCaller_Damage` | 伤害效果要填的伤害量（按此 tag 传数值） |
| `SetByCaller_Heal` | 治疗效果要填的治疗量 |

**源码用法**（GAS 的 SetByCaller 机制，动态传数值）：
```cpp
// 施放伤害时，用 SetByCaller tag 指定本次伤害量（而不是写死）
Spec.SetSetByCallerMagnitude(LyraGameplayTags::SetByCaller_Damage, DamageAmount);
```
> 作用：**让同一个伤害效果资产，运行时由代码填入不同的数值**。

### 组 7：`Cheat.*` —— 作弊标签（2 个）

源码（L39~40）：
```cpp
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_GodMode,        "Cheat.GodMode",        "GodMode cheat is active on the owner.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_UnlimitedHealth, "Cheat.UnlimitedHealth", "UnlimitedHealth cheat is active on the owner.");
```

| C++ 常量 | 含义 |
|---|---|
| `Cheat_GodMode` | 上帝模式开启 |
| `Cheat_UnlimitedHealth` | 无限血量开启 |

**源码用法**（第 15 篇 HealthSet 见过）：
```cpp
if (Data.Target.HasMatchingGameplayTag(LyraGameplayTags::Cheat_GodMode) && ...)
{
	Data.EvaluatedData.Magnitude = 0.0f;   // 开挂 → 伤害归零
}
```
> 作用：**全局锁血**——角色身上贴个 tag，所有伤害自动无效。

### 组 8：`Status.*` —— 状态标签（5 个）

源码（L42~46）：
```cpp
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Crouching,    "Status.Crouching",    "Target is crouching.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_AutoRunning,  "Status.AutoRunning",  "Target is auto-running.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death,        "Status.Death",        "Target has the death status.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dying,  "Status.Death.Dying",  "Target has begun the death process.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dead,   "Status.Death.Dead",   "Target has finished the death process.");
```

| C++ 常量 | 含义 |
|---|---|
| `Status_Crouching` | 在蹲下 |
| `Status_AutoRunning` | 在自动奔跑 |
| `Status_Death` | 有死亡状态 |
| `Status_Death_Dying` | 死亡中（刚开始死亡流程） |
| `Status_Death_Dead` | 已死（死亡流程完成） |

> 注意 `Status.Death.Dying` 和 `Status.Death.Dead` 是 `Status.Death` 的**子标签**（点分层级）——这正是 GameplayTags 的优势：`HasTag(Status_Death)` 能同时命中所有 `Status.Death.*` 子标签。死亡是个**过程**：先 Dying（播动画）再 Dead（真正移除）。

### 组 9：`Movement.Mode.*` —— 移动模式（6 个 + 2 映射表）

源码（L49~67）：
```cpp
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Walking,   "Movement.Mode.Walking",  "...");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_NavWalking, "Movement.Mode.NavWalking","...");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Falling,   "Movement.Mode.Falling",  "...");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Swimming,  "Movement.Mode.Swimming", "...");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Flying,    "Movement.Mode.Flying",   "...");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Custom,    "Movement.Mode.Custom",   "should be replaced with custom tags...");

// 引擎枚举 → tag 的映射表（把 MOVE_Walking 等转成 tag）
const TMap<uint8, FGameplayTag> MovementModeTagMap = {
	{ MOVE_Walking, Movement_Mode_Walking },
	{ MOVE_NavWalking, Movement_Mode_NavWalking },
	{ MOVE_Falling, Movement_Mode_Falling },
	{ MOVE_Swimming, Movement_Mode_Swimming },
	{ MOVE_Flying, Movement_Mode_Flying },
	{ MOVE_Custom, Movement_Mode_Custom }
};
```

| C++ 常量 | 对应引擎枚举 |
|---|---|
| `Movement_Mode_Walking` | MOVE_Walking |
| `Movement_Mode_Falling` | MOVE_Falling |
| ... | 依次对应 |

> 作用：**把角色的移动模式（引擎枚举）翻译成 tag**，方便统一用 tag 系统判断"我现在在走路/坠落/游泳"（比如走路时播脚步 tag、坠落时不能放某技能）。

---

## 二、源码"声明→定义"两半（回顾）

```
.h（声明，别人能用）              .cpp（定义，注册进官方系统）
namespace LyraGameplayTags        namespace LyraGameplayTags
{                                 {
  UE_DECLARE_GAMEPLAY_TAG_EXTERN(  UE_DEFINE_GAMEPLAY_TAG_COMMENT(
    InitState_Spawned);              InitState_Spawned,
                                      "InitState.Spawned",
                                      "1: ...");
}                                 }
```
- 声明：`.h`，告诉别处"有 `LyraGameplayTags::InitState_Spawned` 这个常量"
- 定义：`.cpp`，把字符串 `"InitState.Spawned"` 真正注册进官方 GameplayTags 系统

---

## 三、总结一句话

> **Lyra 自定义的约 40 个 tag 分 7 大类**：`Ability.*`（激活失败原因/死亡保留）、`InputTag.*`（按键绑定）、`InitState.*`（初始化状态机的 4 站名）、`GameplayEvent.*`（死亡/重置事件）、`SetByCaller.*`（伤害/治疗数值）、`Cheat.*`（上帝模式/锁血）、`Status.*`（蹲/跑/死亡过程）、`Movement.Mode.*`（移动模式转 tag，含映射表）。它们全部在 `LyraGameplayTags.h` 声明、`.cpp` 用 `UE_DEFINE_GAMEPLAY_TAG_COMMENT` 注册进**官方** GameplayTags 系统，然后各处代码通过 `LyraGameplayTags::XXX` 引用——**当状态名、当输入绑定名、当失败原因、当事件频道、当数值标签、当作弊/状态开关**。

---

## 四、下一步

- 选一个组深挖它的"消费方"：比如搜 `Status_Death_Dying` 谁添加/谁监听，看状态 tag 的生命周期。
- 看 `MovementModeTagMap` 被谁调用（`GetMovementModeTagMap`？），理解枚举→tag 翻译的实际使用。
- 试在编辑器 GameplayTags 管理器里搜 `InitState.`，看这些 tag 的层级结构长啥样。
