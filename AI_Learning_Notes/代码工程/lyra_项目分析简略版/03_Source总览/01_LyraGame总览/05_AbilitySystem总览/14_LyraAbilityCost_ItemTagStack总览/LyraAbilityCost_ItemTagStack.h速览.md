# `LyraAbilityCost_ItemTagStack.h` 速览

> 消耗**关联物品实例上的 Tag 堆栈**（典型用法：这把枪的弹药）。菜单里显示 "Item Tag Stack"。

| 成员 | 干嘛的 |
|---|---|
| `: ULyraAbilityCost` | 继承消耗基类 |
| `CheckCost()` / `ApplyCost()` | 两个 override |
| `Quantity` | 按技能等级取数量 |
| `Tag` | 消耗哪个 Tag |
| `FailureTag` | ⭐ 付不起时**回传哪个失败 Tag**（比 PlayerTagStack 多的一个字段） |

> 💡 它要求技能必须是 `ULyraGameplayAbility_FromEquipment`（装备赋予的技能），才能通过 `GetAssociatedItem()` 拿到物品实例。

**优先级**：`CheckCost` → `ApplyCost`
