# `LyraGameplayAbility_Reset.cpp` 速览

> 61 行，一个构造函数 + 一个激活函数。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | `InstancedPerActor` + `ServerInitiated`；⭐ 在 CDO 上注册 `GameplayEvent_RequestReset` 触发器 |
| `ActivateAbility()` | ⭐ 见下 |

## `ActivateAbility()` 的流程

```
1. CancelAbilities(...)  忽略 Ability_Behavior_SurvivesDeath
2. SetCanBeCanceled(false)
3. LyraChar->Reset()                      ← 真正的重置
4. 广播 GameplayEvent_Reset 消息           ← 让其它系统知道
5. Super::ActivateAbility()
6. 立刻 EndAbility()                       ← 这是个"一次性"技能
```

> 💡 注意它**没有走激活组**，只做了取消其它技能 + 立刻结束。因为重置是瞬发操作，不需要长期占用激活组。
