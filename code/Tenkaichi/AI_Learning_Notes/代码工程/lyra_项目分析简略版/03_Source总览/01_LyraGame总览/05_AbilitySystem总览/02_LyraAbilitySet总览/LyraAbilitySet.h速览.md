# `LyraAbilitySet.h` 速览

> 一个数据资产，把"一组技能 + GE + 属性集"打包，**一次性授予、也能一次性收回**。

## 4 个配置结构体

| 结构体 | 字段 | 干嘛的 |
|---|---|---|
| `FLyraAbilitySet_GameplayAbility` | `Ability`、`AbilityLevel`、`InputTag` | 授予一个技能，并指定它绑哪个 InputTag |
| `FLyraAbilitySet_GameplayEffect` | `GameplayEffect`、`EffectLevel` | 授予一个 GE |
| `FLyraAbilitySet_AttributeSet` | `AttributeSet` | 授予一个属性集 |
| `FLyraAbilitySet_GrantedHandles` | 三个数组 | ⭐ **记账**：记住授予了什么，用于精确收回 |

## `ULyraAbilitySet`

| 成员 | 干嘛的 |
|---|---|
| `: UPrimaryDataAsset`（`BlueprintType, Const`） | 数据资产、只读 |
| `GrantedGameplayAbilities` | 要授予的技能列表 |
| `GrantedGameplayEffects` | 要授予的 GE 列表 |
| `GrantedAttributes` | 要授予的属性集列表 |
| `GiveToAbilitySystem()` | ⭐ 授予，把句柄写到 `OutGrantedHandles` |
| `FLyraAbilitySet_GrantedHandles::TakeFromAbilitySystem()` | ⭐ 反向：全部收回 |

> 💡 **`GrantedHandles` 这个设计很关键** —— 没有它，GameFeature 卸载时就没法精确知道要收回哪些技能。它的存在是"玩法可插拔"的前提。

**优先级**：`GiveToAbilitySystem` → `TakeFromAbilitySystem`
