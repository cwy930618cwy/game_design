# `LyraAbilitySourceInterface.h` 速览

> 一个纯接口（**只有 `.h` 有内容**），定义"伤害衰减该怎么算"。武器、技能等伤害来源去实现它。

| 成员 | 干嘛的 |
|---|---|
| `ULyraAbilitySourceInterface` | UINTERFACE 壳（UE 接口的标准写法） |
| `ILyraAbilitySourceInterface` | 真正的接口类 |
| `GetDistanceAttenuation()` | ⭐ 距离衰减：子弹飞了多远 → 伤害乘多少 |
| `GetPhysicalMaterialAttenuation()` | ⭐ 材质衰减：打中的是什么材质（石头/木头/肉）→ 伤害乘多少 |

两个函数都带 `SourceTags` / `TargetTags` 参数，方便按 Tag 做更细的判断。

> 💡 **这两个衰减因子在哪被用**：`AbilitySystem/Executions/LyraDamageExecution.cpp` 计算伤害时会取出它们相乘。

**说明**：`.cpp` 文件里只有一个空的 `ULyraAbilitySourceInterface` 构造函数（UINTERFACE 的标配）。
