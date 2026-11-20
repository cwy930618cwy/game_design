# `LyraGameplayEffectContext.h` 速览

> 自定义 GE 上下文。原生 `FGameplayEffectContext` 里没有的东西，往这里加。

| 成员 | 干嘛的 |
|---|---|
| `: FGameplayEffectContext` | 标准继承 |
| `ExtractEffectContext()` | ⭐ 静态：从句柄里安全取出 Lyra 版上下文（类型不对返回 nullptr） |
| `SetAbilitySource()` / `GetAbilitySource()` | ⭐ 记录"伤害是谁造成的"（实现 `ILyraAbilitySourceInterface` 的对象） |
| `Duplicate()` | 深拷贝，**特别注意把 HitResult 也深拷贝** |
| `GetScriptStruct()` | 返回自己的 StaticStruct |
| `NetSerialize()` | 网络序列化（子类新增字段要在这处理） |
| `GetPhysicalMaterial()` | 从命中结果里取物理材质（打中石头还是木头） |
| `CartridgeID` | ⭐ 弹夹 ID：同一发霰弹的多颗弹丸共享一个 ID |
| `AbilitySourceObject` | 伤害来源对象的弱指针（**注释：暂不复制**） |
| `TStructOpsTypeTraits` | 打开 `WithNetSerializer` + `WithCopy` |

> 💡 **`CartridgeID` 是干嘛的**：霰弹枪一次打出 8 颗弹丸，会产生 8 次伤害。有了 CartridgeID，伤害数字就能**合并成一个显示**，而不是跳 8 个数字。

**优先级**：`ExtractEffectContext` → `GetAbilitySource`
