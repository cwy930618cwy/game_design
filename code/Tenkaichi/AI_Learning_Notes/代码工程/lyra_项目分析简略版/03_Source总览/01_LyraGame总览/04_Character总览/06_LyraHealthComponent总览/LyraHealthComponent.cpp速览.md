# `LyraHealthComponent.cpp` 速览

> 313 行。**"血归零之后发生了什么"全在这个文件里**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 关 Tick、开复制、初始 `NotDead` |
| `InitializeWithAbilitySystem()` | 防重复初始化 → 取 `ULyraHealthSet` → 注册 3 个回调 → ⭐ **把血量设成满血**（注释：临时方案，以后用表格驱动）→ 广播初始值 |
| `UninitializeFromAbilitySystem()` | 清 Tag + 解绑 3 个回调 |
| `HandleHealthChanged` / `HandleMaxHealthChanged` | 直接转发成 `OnHealthChanged` / `OnMaxHealthChanged` 广播 |
| `HandleOutOfHealth()` | ⭐ 核心，见下 |
| `OnRep_DeathState()` | ⭐ 客户端侧的状态机同步，见下 |
| `StartDeath()` | 非 `NotDead` 就忽略；置 `DeathStarted` + 打 `Status_Death_Dying` 标签 + 广播 + `ForceNetUpdate` |
| `FinishDeath()` | 必须是 `DeathStarted`；置 `DeathFinished` + 打 `Status_Death_Dead` 标签 + 广播 |
| `DamageSelfDestruct()` | 从 `ULyraGameData::Get().DamageGameplayEffect_SetByCaller` 取 GE，加伤害自毁标签，伤害量 = **最大血量** |

## `HandleOutOfHealth()` 做的两件事（仅服务器）

```
① 发 GameplayEvent: GameplayEvent_Death
   → 用来触发"死亡技能"（死亡动画等就是一个 GameplayAbility）
   → 带 Instigator / Target / GE 上下文 / 源目标 Tag / 伤害量

② 发消息总线: Lyra.Elimination.Message
   → 计分板、击杀播报等系统监听它
   → 源码留了 TODO：助攻消息、敌我判定等还没做
```

## `OnRep_DeathState()` 的预测处理（很巧妙）

```
1. 先 DeathState = OldDeathState      ← 故意回退
2. 如果 Old > New：说明客户端"预测超前"了 → 打 Warning 并放弃
3. 否则用 StartDeath() / FinishDeath() 把状态重放一遍
4. 最后 ensure(DeathState == NewDeathState)
```

> 💡 为什么要这么绕？因为死亡状态变化**必须由 `StartDeath`/`FinishDeath` 来驱动**（它们要打 Tag、发广播）。直接赋值会跳过这些副作用。所以拿到网络值后先回退，再"合法地"走一遍流程。

**优先级**：`HandleOutOfHealth` → `OnRep_DeathState` → `DamageSelfDestruct`
