# `LyraCombatSet.h` 速览

> 战斗属性集 —— **攻击方**的属性（和 `LyraHealthSet` 是防守方，正好一对）。

| 成员 | 干嘛的 |
|---|---|
| `: ULyraAttributeSet`（`BlueprintType`） | 标准继承 |
| `BaseDamage` | ⭐ 基础伤害值，由 `LyraDamageExecution` 读取 |
| `BaseHeal` | ⭐ 基础治疗值，由 `LyraHealExecution` 读取 |
| `OnRep_BaseDamage()` / `OnRep_BaseHeal()` | 复制回调 |

> 💡 **为什么只复制给 Owner？** 见 cpp —— 用的是 `COND_OwnerOnly`。因为**你不需要知道敌人的攻击力是多少**，这是防作弊也是省带宽。

**优先级**：看 `LyraDamageExecution` 怎么用它
