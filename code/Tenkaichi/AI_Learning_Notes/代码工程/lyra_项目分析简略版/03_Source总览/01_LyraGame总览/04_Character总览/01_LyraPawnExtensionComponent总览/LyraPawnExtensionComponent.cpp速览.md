# `LyraPawnExtensionComponent.cpp` 速览

> 313 行。**InitState 状态机的这一半全在这里**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 关 Tick、开复制 |
| `OnRegister()` | 两个 `ensureAlwaysMsgf`：必须挂在 Pawn 上、**一个 Pawn 只能有一个**；然后 `RegisterInitStateFeature()` |
| `BeginPlay()` | 监听**所有** feature 的状态变化 → `TryToChangeInitState(Spawned)` → `CheckDefaultInitialization()` |
| `EndPlay()` | 卸载 ASC + 反注册特性 |
| `SetPawnData()` | ⭐ 只在本机权威端生效；已有 PawnData 再设会 Error 并**拒绝**；设完 `ForceNetUpdate()` |
| `OnRep_PawnData()` | 客户端收到后重试一次初始化 |
| `InitializeAbilitySystem()` | ⭐ 见下 |
| `UninitializeAbilitySystem()` | 取消技能（**跳过带 `Ability_Behavior_SurvivesDeath` 的技能**）→ 清输入 → 清 Cue → 解绑 Avatar → 广播 |
| 三个"时机通知" | `HandleControllerChanged` / `HandlePlayerStateReplicated` / `SetupPlayerInputComponent` **都只调 `CheckDefaultInitialization()`** |
| `CheckDefaultInitialization()` | ① 先推进依赖方 `CheckDefaultInitializationForImplementers()` ② 再 `ContinueInitStateChain(4 个状态)` |
| `CanChangeInitState()` | ⭐ 四段判断，见下 |
| `OnActorInitStateChanged()` | 别的 feature 到了 `DataAvailable` → 再试一次推进 |

## `CanChangeInitState()` 的四道关

| 从 → 到 | 条件 |
|---|---|
| 无 → `Spawned` | 有 Pawn 就行 |
| `Spawned` → `DataAvailable` | ⭐ **必须有 PawnData**；且若权威或本地控制，**必须已被 Controller 占有** |
| `DataAvailable` → `DataInitialized` | `HaveAllFeaturesReachedInitState(Pawn, DataAvailable)` —— **等其他所有组件都就位** |
| `DataInitialized` → `GameplayReady` | 无条件通过 |

## `InitializeAbilitySystem()` 的一个细节

```
如果 ASC 上已经有别的 Avatar（不是我）：
    ensure(!ExistingAvatar->HasAuthority())   // 断言只在客户端发生
    → 把那个旧 Avatar 的 PawnExtComponent 先 UninitializeAbilitySystem()
```
> 源码注释解释了这个场景：**客户端卡顿时，新的 Pawn 可能在旧的死亡 Pawn 被移除前就生成并占有了**，所以要把旧的踢掉。

**优先级**：`CanChangeInitState` → `CheckDefaultInitialization` → `InitializeAbilitySystem`
