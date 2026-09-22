# `LyraGlobalAbilitySystem.h` 速览

> 一个 `UWorldSubsystem`，用来**给场上所有 ASC 批量施加技能或 GE**（比如全场 buff、全场眩晕）。

## 两个记账结构体

| 结构体 | 干嘛的 |
|---|---|
| `FGlobalAppliedAbilityList` | 记录"这个技能被施加到了哪些 ASC"，以及各自的句柄 |
| `FGlobalAppliedEffectList` | 同上，针对 GE |

## `ULyraGlobalAbilitySystem`

| 成员 | 干嘛的 |
|---|---|
| `: UWorldSubsystem` | 世界级子系统 |
| `ApplyAbilityToAll()` / `RemoveAbilityFromAll()` | 给所有已注册 ASC 施加/移除技能（`BlueprintAuthorityOnly`） |
| `ApplyEffectToAll()` / `RemoveEffectFromAll()` | 同上，针对 GE |
| `RegisterASC()` / `UnregisterASC()` | ⭐ 注册/注销一个 ASC；**注册时会把当前所有全局效果补给它** |
| `AppliedAbilities` / `AppliedEffects` | 按类记账 |
| `RegisteredASCs` | 当前已注册的 ASC 列表 |

> 💡 `RegisterASC` 是这套机制的关键：**后加入的 ASC 也会自动获得之前施加的全局效果**，不用重新调一次。

**优先级**：`RegisterASC` → `ApplyAbilityToAll`
