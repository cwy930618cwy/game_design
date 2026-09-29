# `LyraAbilityCost_PlayerTagStack.h` 速览

> 消耗 **PlayerState 上的 Tag 堆栈**（典型用法：法力、能量）。菜单里显示 "Player Tag Stack"。

| 成员 | 干嘛的 |
|---|---|
| `: ULyraAbilityCost` | 继承消耗基类 |
| `CheckCost()` / `ApplyCost()` | 两个 override |
| `Quantity` | `FScalableFloat`，**按技能等级**取数量 |
| `Tag` | 消耗哪个 Tag |

> 💡 用 `FScalableFloat` 而不是 `int32`，是为了让"1 级技能耗 10 蓝，5 级技能耗 30 蓝"这类曲线**能在数据里配**，不用改代码。

**优先级**：`CheckCost` → `ApplyCost`
