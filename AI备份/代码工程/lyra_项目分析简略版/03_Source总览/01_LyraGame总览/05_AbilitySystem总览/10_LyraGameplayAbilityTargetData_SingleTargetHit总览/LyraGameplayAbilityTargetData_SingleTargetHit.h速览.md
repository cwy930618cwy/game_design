# `LyraGameplayAbilityTargetData_SingleTargetHit.h` 速览

> 在引擎原生的"单体命中"目标数据上，**加了一个 `CartridgeID`**。

| 成员 | 干嘛的 |
|---|---|
| `: FGameplayAbilityTargetData_SingleTargetHit` | 标准继承 |
| `AddTargetDataToContext()` | 把自己的数据写进 GE 上下文 |
| `CartridgeID` | ⭐ 弹夹 ID，默认 -1 |
| `NetSerialize()` | 序列化（注释：少了它 `FGameplayAbilityTargetDataHandle` 的网络序列化就不工作） |
| `GetScriptStruct()` | 返回自己的 StaticStruct |
| `TStructOpsTypeTraits` | 打开 `WithNetSerializer` |

> 💡 和 `LyraGameplayEffectContext::CartridgeID` 是一对：这个结构负责**产生** ID，上下文负责**携带**它，最终让同一次射击的多颗弹丸能合并伤害数字。

**优先级**：`AddTargetDataToContext`
