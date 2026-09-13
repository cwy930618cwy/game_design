# `LyraAbilityCost_PlayerTagStack.cpp` 速览

> 52 行，两个对称的函数。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | `Quantity.SetValue(1.0f)` |
| `CheckCost()` | 拿 Controller → PlayerState → `GetStatTagStackCount(Tag) >= NumStacks` |
| `ApplyCost()` | **只在权威端**执行 → `PS->RemoveStatTagStack(Tag, NumStacks)` |

> 💡 数量用 `Quantity.GetValueAtLevel(AbilityLevel)` 取，再 `TruncToInt` 截断成整数。

**说明**：这是三种消耗里**唯一真正完整可用**的一个（另一个 ItemTagStack 也完整，InventoryItem 被 `#if 0` 关掉了）。
