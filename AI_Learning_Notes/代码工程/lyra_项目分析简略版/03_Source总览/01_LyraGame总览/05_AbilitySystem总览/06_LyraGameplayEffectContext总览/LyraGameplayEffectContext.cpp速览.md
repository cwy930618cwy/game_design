# `LyraGameplayEffectContext.cpp` 速览

> 68 行，六个短函数。

| 函数 | 干嘛的 |
|---|---|
| `ExtractEffectContext()` | 取出基类指针后，用 `GetScriptStruct()->IsChildOf(...)` **做类型校验**再强转；不匹配返回 nullptr |
| `NetSerialize()` | ⚠️ 只调 Super，**注释明确写着：`CartridgeID` 不参与网络序列化**（它只在激活后本地使用） |
| Iris 转发序列化器 | `#if UE_WITH_IRIS` 下转发给 `FGameplayEffectContextNetSerializer`；注释警告：**如果改了 `NetSerialize`，必须配套写自定义序列化器** |
| `SetAbilitySource()` | 转成 UObject 存弱指针；`InSourceLevel` 参数**被注释掉没用** |
| `GetAbilitySource()` | 强转回接口 |
| `GetPhysicalMaterial()` | 从 `GetHitResult()->PhysMaterial` 取 |

> ⚠️ **两个"故意没做"的地方值得注意**：
> 1. `CartridgeID` 不走网络（所以客户端收不到它）
> 2. `SetAbilitySource` 的第二个参数 `InSourceLevel` 被注释了（源码里那行是 `//SourceLevel = InSourceLevel;`）

**优先级**：`ExtractEffectContext`
