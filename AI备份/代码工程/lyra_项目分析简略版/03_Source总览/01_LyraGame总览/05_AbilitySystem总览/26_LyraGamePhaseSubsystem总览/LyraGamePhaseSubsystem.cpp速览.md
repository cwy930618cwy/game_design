# `LyraGamePhaseSubsystem.cpp` 速览

> 225 行。**核心是 `OnBeginPhase` 里那段"结束掉非祖先阶段"的逻辑**。

| 函数 | 干嘛的 |
|---|---|
| `ShouldCreateSubsystem()` | 恒返回 true（**原来的判断逻辑被注释掉了**） |
| `DoesSupportWorldType()` | 只在 `Game` / `PIE` 世界创建 |
| `StartPhase()` | ⭐ 从 GameState 上找 ASC，用 `GiveAbilityAndActivateOnce` 授予并激活阶段技能；没激活成功就立刻回调结束 |
| 三个 `K2_*` | 用 `CreateWeakLambda` 把动态委托包成普通委托（**弱引用，避免内存泄漏**） |
| `WhenPhaseStartsOrIsActive()` | 注册观察者；**如果阶段已经在跑，立即回调一次** |
| `WhenPhaseEnds()` | 注册结束观察者 |
| `IsPhaseActive()` | 用 `MatchesTag` 判断（所以父阶段也算活跃） |
| `OnBeginPhase()` | ⭐ 见下 |
| `OnEndPhase()` | 执行结束回调 → 从表里移除 → 通知所有结束观察者 |
| `FPhaseObserver::IsMatch()` | `ExactMatch` 用 `==`；`PartialMatch` 用 `MatchesTag` |

## `OnBeginPhase()` 做了什么

```
1. 收集当前所有活跃阶段的 Spec
2. 逐个检查：if (!IncomingPhaseTag.MatchesTag(ActivePhaseTag))
       → 说明新阶段不是它的子孙 → 取消掉它
3. 把自己加进 ActivePhaseMap
4. 通知所有"开始"观察者
```

> 💡 **`MatchesTag` 是整套机制的钥匙**：`Game.Playing.SuddenDeath`.MatchesTag(`Game.Playing`) 为真，所以切子阶段时父阶段能保留；反过来不成立，所以兄弟阶段会被清掉。

> ⚠️ `ShouldCreateSubsystem` 里原来判断 `GetAuthGameMode() != nullptr` 的代码被整段注释了，现在**任何 Game/PIE 世界都会创建**这个子系统。

**优先级**：`OnBeginPhase` → `StartPhase`
