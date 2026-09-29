# `LyraHealthComponent.h` 速览

> 血量与死亡。**它不自己算伤害** —— 数值在 `ULyraHealthSet` 上，它只负责监听和广播。

| 成员 | 干嘛的 |
|---|---|
| `ELyraDeathState` | `NotDead` → `DeathStarted` → `DeathFinished` 三态 |
| `FLyraHealth_DeathEvent` / `FLyraHealth_AttributeChanged` | 两个动态多播委托类型（后者带 旧值/新值/造成者） |
| `: UGameFrameworkComponent`（`Blueprintable`） | 蓝图可加 |
| `FindHealthComponent()` | 静态查找 |
| `InitializeWithAbilitySystem()` / `UninitializeFromAbilitySystem()` | 接上/断开 ASC |
| `GetHealth()` / `GetMaxHealth()` / `GetHealthNormalized()` | 血量查询 |
| `GetDeathState()` / `IsDeadOrDying()` | 死亡状态（`IsDeadOrDying` 带 `ExpandBoolAsExecs`，蓝图出双分支） |
| `StartDeath()` / `FinishDeath()` | ⭐ 推进死亡状态机 |
| `DamageSelfDestruct()` | 给自己造成致命伤害（掉出世界、机器人移除时用） |
| 4 个 `BlueprintAssignable` 委托 | `OnHealthChanged` / `OnMaxHealthChanged` / `OnDeathStarted` / `OnDeathFinished` |
| `HandleHealthChanged` / `HandleMaxHealthChanged` / `HandleOutOfHealth` | 三个来自 HealthSet 的回调 |
| `OnRep_DeathState()` | ⭐ 客户端的死亡状态同步（含预测回退处理） |
| `ClearGameplayTags()` | 清掉 `Status_Death_Dying` / `Status_Death_Dead` |
| `AbilitySystemComponent` / `HealthSet` / `DeathState` | 内部状态（DeathState 是复制的） |

**优先级**：`HandleOutOfHealth` → `StartDeath` / `FinishDeath` → `OnRep_DeathState`
