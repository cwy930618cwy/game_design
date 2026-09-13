# `LyraGameplayAbilityTargetData_SingleTargetHit.cpp` 速览

> 33 行，两个函数。

| 函数 | 干嘛的 |
|---|---|
| `AddTargetDataToContext()` | 先调 Super，再用 `FLyraGameplayEffectContext::ExtractEffectContext` 取出 Lyra 上下文，**把 `CartridgeID` 写进去** |
| `NetSerialize()` | 先调 Super，再 `Ar << CartridgeID` |

> ⚠️ **一个不一致的地方**：这里 `CartridgeID` **参与了网络序列化**，但 `FLyraGameplayEffectContext::NetSerialize` 里明确注释说它**不序列化**。
> 也就是说：目标数据会带着 CartridgeID 传出去，但 GE 上下文这边不会。实际用的时候要注意这个差异。

**优先级**：`AddTargetDataToContext`
