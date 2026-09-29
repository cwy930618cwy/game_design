# 10 — `ULyraAbilitySystemComponent` 详解：Lyra 项目里"能力系统"的运行核心

> **定位**：前面学了 `ULyraPawnData`（配方）和 `ULyraPawnExtensionComponent`（总指挥）。它身上挂着、协调着的那个"能力系统组件"（ASC）到底是谁、是干嘛的？这一篇把它彻底讲清楚。
>
> 讲 `ULyraAbilitySystemComponent` 的**职责、继承、源码原理、核心机制**。

---

## 一、一句话看懂它

> **`ULyraAbilitySystemComponent`（下文简称 Lyra ASC）= Lyra 项目自定义的"能力系统组件"，是所有 `UGameplayAbility`（游戏能力）的执行大脑。** 它继承自 UE 官方的 `UAbilitySystemComponent`，在官方基础上追加了**输入驱动激活、激活分组互斥、Tag 关系映射、全服广播**等 Lyra 专属玩法逻辑。

看它的类注释（`.h` 第 22~26 行）：

```
/**
 * ULyraAbilitySystemComponent
 *
 *	Base ability system component class used by this project.
 */
（本项目使用的基础能力系统组件类）
```

**关键词**：`Base ability system component class used by this project`——它不是官方那套原版 ASC，而是 Lyra 给它包了一层"项目专属外壳"的加强版。

---

## 二、为什么需要它？（没有它会怎样）

官方 `UAbilitySystemComponent` 已经很强大——它能授予技能、施加效果、管理属性、处理网络预测。

但**官方版是"通用工具"，不关心游戏规则**。Lyra 想要这些"玩法级"需求，官方版不直接给：

| Lyra 想要的玩法能力 | 官方 ASC 是否直接支持 | 谁来解决 |
|---|---|---|
| "按住按键持续施法""按一下触发一次"这类**输入激活策略** | ❌ 只支持通用输入 | Lyra ASC 的 `ProcessAbilityInput` |
| "开大不能同时开别的""处决技互相打断"这类**激活互斥/分组** | ❌ | Lyra ASC 的 `ActivationGroup`（激活分组） |
| "位移技能和霸体技能的 Tag 该怎么互相 cancel/block" | ⚠️ 有基础 Tag，但需要一张**可配置的关系表** | `TagRelationshipMapping` + `ULyraAbilityTagRelationshipMapping` |
| "所有角色都被全局上某种效果（如全局禁赛标记）" | ❌ | `ULyraGlobalAbilitySystem`（世界子系统广播） |

> **类比**：
> - 官方 `UAbilitySystemComponent` = 一台**功能齐全但裸奔的 CPU**（能运算，但不知道你的业务）。
> - Lyra `ULyraAbilitySystemComponent` = 给这台 CPU 装了**游戏专用操作系统**（知道"输入怎么触发技能""哪些技能该互斥""Tag 关系怎么管"）。

---

## 三、继承关系：站在官方肩膀上

```
UObject
  └─ UActorComponent
       └─ UAbilitySystemComponent   ← UE 官方 GAS 核心
            └─ ULyraAbilitySystemComponent   ← Lyra 加强版（本项目主角）
```

`.h` 第 27~28 行：
```cpp
UCLASS(MinimalAPI)
class ULyraAbilitySystemComponent : public UAbilitySystemComponent
```

**它 override 了哪些官方虚函数**（这是理解它"改了什么"的关键）：

| Override 的虚函数 | 它额外干了什么 |
|---|---|
| `InitAbilityActorInfo` | 拿到新 Pawn 化身时：通知所有技能 `OnPawnAvatarSet`、注册进全局系统、初始化动画实例、尝试出生即激活 |
| `AbilitySpecInputPressed/Released` | 转发"按下/松开"为可复制的通用事件（给 `WaitInputPress` 任务用） |
| `NotifyAbilityActivated/Ended/Failed` | 技能激活/结束/失败时的钩子 → 维护激活分组计数、播失败回调 |
| `ApplyAbilityBlockAndCancelTags` | 用 `TagRelationshipMapping` 把"能力 Tag"扩展成真正的 block/cancel 列表 |
| `HandleChangeAbilityCanBeCanceled` | 预留的口子（Lyra 未填逻辑，留 TODO） |

---

## 四、四个核心玩法机制（源码逐一拆）

### 机制 1：输入驱动激活 —— `ProcessAbilityInput`

这是**把"玩家按的按键"翻译成"技能激活"**的翻译器。配套两个枚举（来自 `ULyraGameplayAbility`，`LyraGameplayAbility.h` 第 38~49 行）：

```cpp
UENUM(BlueprintType)
enum class ELyraAbilityActivationPolicy : uint8
{
	OnInputTriggered,    // 按一下 → 触发一次（如：单发技能）
	WhileInputActive,    // 按住 → 持续激活（如：蓄力/引导）
	OnSpawn              // 获得化身那一刻就激活（出生自带被动）
};
```

调用流程（`.cpp` 第 216~311 行 `ProcessAbilityInput`）：

```
AbilityInputTagPressed(Tag)  ──►  找到身上所有绑了该 Tag 的技能，记进缓存数组
      │
      ▼  （每帧）
ProcessAbilityInput(dt, bPaused)
  ① 若身上有 AbilityInputBlocked 标记 → 清空全部输入，直接返回（被禁赛时）
  ② 遍历 InputHeldSpecHandles     → WhileInputActive 类：加入"待激活"队列
  ③ 遍历 InputPressedSpecHandles  → OnInputTriggered 类：加入"待激活"队列
  ④ 统一 TryActivateAbility(...)  → 一次批处理激活
  ⑤ 遍历 InputReleasedSpecHandles → 正在跑的技能收到"松开"事件
  ⑥ 清空本周期的缓存数组
```

> **注意 ②/③ 分开处理的原因**（源码注释写了）：避免"按住持续激活的技能"和"按下触发的技能"同时把输入事件发进去造成重复。

### 机制 2：激活分组（互斥）—— `ActivationGroup`

Lyra 想表达"某些技能天生互斥"（比如处决类大招）。枚举（`LyraGameplayAbility.h` 第 57~70 行）：

```cpp
UENUM(BlueprintType)
enum class ELyraAbilityActivationGroup : uint8
{
	Independent,          // 独立：谁都不管（跑步、翻滚）
	Exclusive_Replaceable,// 独占·可替换：会被其他独占顶掉（普通攻击）
	Exclusive_Blocking,   // 独占·阻挡：它一跑，所有独占都不能开（无敌斩/大招）
	MAX
};
```

**内部数据**是一个计数数组（`.h` 第 107 行）：
```cpp
int32 ActivationGroupCounts[(uint8)ELyraAbilityActivationGroup::MAX];
```

**核心规则**（`IsActivationBlocked` / `AddAbilityToActivationGroup` / `CancelActivationGroupAbilities`）：

- `Independent`：永不阻塞、也从不取消别人。
- 两个 `Exclusive`：只要"跑着一个 `Exclusive_Blocking`"，任何 `Exclusive` 都开不了。
- 新开一个 `Exclusive_Replaceable`/`Exclusive_Blocking` 时，会**自动取消场上其他 `Exclusive_Replaceable`**（顶掉旧招式）。

调用时机：`NotifyAbilityActivated`（激活时加入并计数）、`NotifyAbilityEnded`（结束时移除并减计数）——见 `.cpp` 第 320~354 行。

### 机制 3：Tag 关系映射 —— `TagRelationshipMapping`

能力之间的"该挡谁、该取消谁"如果写死在代码里就很丑。Lyra 把它做成**一张可配置的资产** `ULyraAbilityTagRelationshipMapping`。

`.h` 第 93~95 行：
```cpp
// If set, this table is used to look up tag relationships for activate and cancel
UPROPERTY()
TObjectPtr<ULyraAbilityTagRelationshipMapping> TagRelationshipMapping;
```

这张表在 `ULyraPawnExtensionComponent::InitializeAbilitySystem`（`LyraPawnExtensionComponent.cpp` 第 146 行）里从 `PawnData` 拷进来：

```cpp
if (ensure(PawnData))
{
	InASC->SetTagRelationshipMapping(PawnData->TagRelationshipMapping);
}
```

然后在 `ApplyAbilityBlockAndCancelTags`、`GetAdditionalActivationTagRequirements` 中被查表，把"能力自带的 Tag"扩展成"真正的 block/cancel/required 集合"。**这样设计变更无需改代码，改资产即可。**

### 机制 4：全局广播 —— `ULyraGlobalAbilitySystem`

不是 ASC 自己实现的，但它会主动配合。这是一个 **`UWorldSubsystem`**（每个世界一份，跨所有 ASC 广播）：

- 拥有 Pawn 化身那一刻，ASC 调 `GlobalAbilitySystem->RegisterASC(this)`（`.cpp` 第 71~74 行）。
- 结束播放时调 `UnregisterASC(this)`（`.cpp` 第 31~34 行）。
- 作用：`ApplyAbilityToAll` / `ApplyEffectToAll` 能**给世界上所有已注册的 ASC 统一授予某技能/某效果**（比如"全局天气效果""全服公告式 buff"）。

---

## 五、它是怎么被"挂"上 Pawn 的？（和前面学的总指挥串起来）

回顾 `ULyraPawnExtensionComponent` 是"初始化总指挥"。它把 ASC 请上来的关键代码：

```
ULyraPawnExtensionComponent::InitializeAbilitySystem(InASC, InOwnerActor)
  ├─ 若已有旧 ASC → UninitializeAbilitySystem() 清理
  ├─ 若旧化身还存在 → 把它的 PawnExtension 的 ASC 卸掉（网络延迟兜底）
  ├─ AbilitySystemComponent = InASC            ← 缓存下来
  ├─ AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn)  ← 关键！
  ├─ InASC->SetTagRelationshipMapping(PawnData->TagRelationshipMapping) ← 拷入 Tag 关系表
  └─ OnAbilitySystemInitialized.Broadcast()    ← 通知其他系统"ASC 就绪了"
```

而 `InitAbilityActorInfo`（Lyra ASC 覆写版，`.cpp` 第 39~83 行）一旦发现**新 Pawn 化身**，就会连锁反应：
1. 遍历所有可激活技能 → 通知每个实例 `OnPawnAvatarSet()`（技能知道"我附在谁身上了"）。
2. `RegisterASC` 进全局系统。
3. 初始化 `ULyraAnimInstance`（动画拿到 ASC，可播放蒙太奇）。
4. `TryActivateAbilitiesOnSpawn()` —— 尝试激活 `OnSpawn` 策略的技能（出生即被动）。

---

## 六、`.h` vs `.cpp` 各干嘛的

### `.h`（头文件）——"声明这个加强版有什么"

- 前向声明依赖类型（`AActor`、`UGameplayAbility`、`ULyraAbilityTagRelationshipMapping` 等）。
- 声明一个全局 Tag：`TAG_Gameplay_AbilityInputBlocked`（"技能输入被禁"的开关 Tag）。
- 列出全部新增/覆写的函数（输入驱动、激活分组、Tag 关系、动态 Tag）。
- 声明内部状态：三个输入缓存数组 + `ActivationGroupCounts` + `TagRelationshipMapping`。

### `.cpp`（源文件）——"实现玩法逻辑"

- 构造函数：把三个输入缓存数组清空、`ActivationGroupCounts` 归零。
- `EndPlay`：从全局系统注销。
- `InitAbilityActorInfo`：化身就位时的一串连锁初始化。
- `ProcessAbilityInput`：把输入翻译成技能激活（最重要的每帧逻辑）。
- 一堆 `NotifyAbility*` + 激活分组函数：维护"哪些技能在跑、谁顶谁"。
- `Add/RemoveDynamicTagGameplayEffect`：用动态 GE 临时给角色"增删"一个 Tag（如无敌瞬间的标记）。

---

## 七、和前面学过内容的关系总图

```
  ULyraPawnData（配方）                    ULyraPawnExtensionComponent（总指挥）
   - 包含 TagRelationshipMapping 资产            │
          │                                    │  InitializeAbilitySystem()
          └─────────── 拷入 ───────────────────►│
                                                ▼
                                    ┌──────────────────────────┐
                                    │ ULyraAbilitySystemComponent│ ← 本篇主角（执行大脑）
                                    │  继承官方 UAbilitySystemComponent
                                    │  ├─ 输入驱动激活 (ProcessAbilityInput)
                                    │  ├─ 激活分组互斥 (ActivationGroup)
                                    │  ├─ Tag 关系映射 (TagRelationshipMapping)
                                    │  └─ 配合全局广播 (ULyraGlobalAbilitySystem)
                                    └──────────────────────────┘
                                                │
                                    ULyraGameplayAbility（被它执行的具体技能）
                                    ALyraAnimInstance（被它喂给动画，播蒙太奇）
```

---

## 八、学完这一篇，你应该记住

1. **它是 Lyra 自定义的 ASC**，继承官方 `UAbilitySystemComponent`，是这个项目"技能系统的执行大脑"。
2. **官方版是通用工具**，Lyra 的玩法级需求靠它补：输入激活策略、激活互斥分组、Tag 关系表、全局广播。
3. **`ProcessAbilityInput`** 把"按键"翻译成"激活"，并区分 `OnInputTriggered`/`WhileInputActive`/`OnSpawn`。
4. **`ActivationGroup`**（`Independent`/`Exclusive_Replaceable`/`Exclusive_Blocking`）用计数数组实现"技能间互斥/顶替"。
5. **`TagRelationshipMapping`** 把能力间的 block/cancel 规则做成可配置资产，从 `PawnData` 拷入，改规则不用改代码。
6. **它是被 `ULyraPawnExtensionComponent::InitializeAbilitySystem` 挂上 Pawn 的**，配合 `InitAbilityActorInfo` 完成化身绑定的连锁初始化。
7. 它会主动**注册进 `ULyraGlobalAbilitySystem`**（世界子系统），从而被全局效果/技能覆盖。

---

## 九、下一步

- `ULyraGameplayAbility` 详解：技能本身怎么定义"激活策略""激活分组"，`OnPawnAvatarSet` 干了啥。
- `ULyraAbilityTagRelationshipMapping`：Tag 关系资产到底怎么配置、怎么生效。
- `ULyraGlobalAbilitySystem`：全服能力/效果广播的完整实现。
- 输入层：`ULyraAbilityInputComponent` 怎么把"按键 Tag"喂给 ASC 的 `AbilityInputTagPressed`。
- 回看官方 `UAbilitySystemComponent`，把 `InitAbilityActorInfo`、预测等底层原理补全。
