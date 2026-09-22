# `LyraGameplayAbility_Death.cpp` 速览

> 92 行。**死亡时"为什么其它技能都放不出来了"的答案就在这**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | `InstancedPerActor` + **`ServerInitiated`**（死亡由服务器发起）；`bAutoStartDeath = true`；⭐ **在 CDO 上注册 `GameplayEvent_Death` 触发器** |
| `ActivateAbility()` | ⭐ 见下 |
| `EndAbility()` | 无论如何都调一次 `FinishDeath()`（注释：防止蓝图忘了） |
| `StartDeath()` / `FinishDeath()` | 转调 `ULyraHealthComponent` 的对应方法 |

## `ActivateAbility()` 做的四件事

```
1. CancelAbilities(...)，但忽略带 Ability_Behavior_SurvivesDeath 的技能
2. SetCanBeCanceled(false)         ← 死亡不可打断
3. ChangeActivationGroup(Exclusive_Blocking)  ← 挡住其它所有 Exclusive 技能
4. bAutoStartDeath → StartDeath()
```

> 💡 **`Ability_Behavior_SurvivesDeath` 这个 Tag 是"死亡后仍然保留"的逃生通道** —— 比如某些被动效果不该因为死亡被清掉。同一个 Tag 在 `LyraPawnExtensionComponent::UninitializeAbilitySystem` 里也被用来跳过取消。

> 💡 **在 CDO 里加触发器**（`if (HasAnyFlags(RF_ClassDefaultObject))`）是 Lyra 的惯用写法：让所有子类自动继承这个触发配置，不用每个蓝图手动配。

**优先级**：`ActivateAbility`
