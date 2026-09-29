# `LyraCombatSet.cpp` 速览

> 37 行。**只有构造、复制、两个 RepNotify，完全没有业务逻辑**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | `BaseDamage = 0` / `BaseHeal = 0` |
| `GetLifetimeReplicatedProps()` | 两个属性都 **`COND_OwnerOnly` + `REPNOTIFY_Always`** |
| `OnRep_BaseDamage()` / `OnRep_BaseHeal()` | 只调 `GAMEPLAYATTRIBUTE_REPNOTIFY` |

**说明**：真正的计算不在这，在 `Executions/LyraDamageExecution` 和 `LyraHealExecution` 里。
