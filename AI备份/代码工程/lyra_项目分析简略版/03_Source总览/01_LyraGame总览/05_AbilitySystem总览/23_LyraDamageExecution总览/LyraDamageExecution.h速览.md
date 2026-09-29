# `LyraDamageExecution.h` 速览

> 伤害的**计算公式**。GE 用 `SetByCaller` 传入伤害值，这里负责算出最终伤害。

| 成员 | 干嘛的 |
|---|---|
| `: UGameplayEffectExecutionCalculation` | GAS 的自定义执行计算基类 |
| `Execute_Implementation()` | 唯一的虚函数，所有逻辑在这 |

**说明**：头文件极简，看 cpp。
