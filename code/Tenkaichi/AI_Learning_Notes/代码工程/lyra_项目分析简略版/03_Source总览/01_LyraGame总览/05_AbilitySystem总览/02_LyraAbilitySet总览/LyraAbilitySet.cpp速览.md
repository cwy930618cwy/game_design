# `LyraAbilitySet.cpp` 速览

> 149 行，主要是 `GiveToAbilitySystem` 和 `TakeFromAbilitySystem` 两个方向的对称操作。

| 函数 | 干嘛的 |
|---|---|
| 三个 `Add*Handle` | 只在句柄有效时才记账 |
| `TakeFromAbilitySystem()` | ⭐ **非权威端直接 return**；然后 `ClearAbility` / `RemoveActiveGameplayEffect` / `RemoveSpawnedAttribute`，最后清空三个数组 |
| `GiveToAbilitySystem()` | ⭐ **非权威端直接 return**；按"属性集 → 技能 → GE"的顺序授予 |

## `GiveToAbilitySystem()` 的顺序

```
① 属性集：NewObject + AddAttributeSetSubobject   （必须先有属性，GE 才能改到它）
② 技能：  建 Spec，把 InputTag 塞进 DynamicSpecSourceTags，然后 GiveAbility
③ GE：    ApplyGameplayEffectToSelf
```

> 💡 **顺序不是随便排的**：属性集必须最先加，否则后面的 GE 会找不到要改的属性。

> 💡 **`InputTag` 存在哪**：存在 `AbilitySpec.GetDynamicSpecSourceTags()` 里。所以前面 `LyraASC::AbilityInputTagPressed` 才能用 `HasTagExact(InputTag)` 找到对应技能 —— **这是"改键不影响技能代码"的技术基础**。

**优先级**：`GiveToAbilitySystem` → `TakeFromAbilitySystem`
