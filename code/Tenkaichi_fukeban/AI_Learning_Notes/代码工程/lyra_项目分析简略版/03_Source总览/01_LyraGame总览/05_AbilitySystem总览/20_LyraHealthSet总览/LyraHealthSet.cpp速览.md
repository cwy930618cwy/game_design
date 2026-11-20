# `LyraHealthSet.cpp` 速览

> 235 行。**"伤害怎么变成掉血"的全过程在这**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 血量/最大血量默认 **100** |
| `GetLifetimeReplicatedProps()` | 只复制 `Health` 和 `MaxHealth`，都用 `REPNOTIFY_Always` |
| `OnRep_Health()` | 客户端：广播变化（**没有 Instigator**，注释说明未来可能改成显式 RPC）；跨过 0 时广播 `OnOutOfHealth` |
| `PreGameplayEffectExecute()` | ⭐ 见下 |
| `PostGameplayEffectExecute()` | ⭐ 见下 |
| `ClampAttribute()` | 血量钳到 [0, MaxHealth]；最大血量下限 **1.0** |
| `PostAttributeChange()` | 最大血量变小导致当前血量超了 → 用 `ApplyModToAttribute(Override)` 把血量压下去 |

## `PreGameplayEffectExecute()`：三道"免伤"检查

```
① 目标有 Gameplay.DamageImmunity → 伤害归零（但自毁伤害例外）
② 非 Shipping 下开了 Cheat_GodMode → 伤害归零（自毁例外）
③ 记录变更前的 Health / MaxHealth
```

> ⚠️ 注意注释里的一句：**GodMode 只在这里挡伤害，无限血（UnlimitedHealth）是在下面的 `PostGameplayEffectExecute` 里把最低血设为 1**。两个作弊是分开实现的。

## `PostGameplayEffectExecute()`：把元属性换成真血量

```
① 无限血作弊 → MinimumHealth = 1.0
② Damage 属性变化 → 广播伤害消息 → Health -= Damage，然后 Damage 归零
③ Healing 属性变化 → Health += Healing，然后 Healing 归零
④ Health 直接被改 → 钳制
⑤ MaxHealth 变化 → 广播 OnMaxHealthChanged
⑥ 血确实变了 → 广播 OnHealthChanged
⑦ 血 <= 0 且之前没归零 → 广播 OnOutOfHealth   ← HealthComponent 就是听这个触发死亡的
⑧ 最后再查一次血（因为上面的回调可能又改了它）
```

> 💡 **第 ⑧ 步很重要**：注释明确写了 "Check health again in case an event above changed it" —— 因为广播的回调里完全可能再次修改血量（比如某个被动技能触发回血）。

**优先级**：`PostGameplayEffectExecute` → `PreGameplayEffectExecute`
