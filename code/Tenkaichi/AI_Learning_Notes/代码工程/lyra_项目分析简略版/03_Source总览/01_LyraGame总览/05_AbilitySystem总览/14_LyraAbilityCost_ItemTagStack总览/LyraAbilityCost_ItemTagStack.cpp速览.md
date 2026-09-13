# `LyraAbilityCost_ItemTagStack.cpp` 速览

> 61 行。

| 函数 | 干嘛的 |
|---|---|
| `TAG_ABILITY_FAIL_COST` | 定义 `Ability.ActivateFail.Cost` 这个全局 Tag |
| 构造函数 | `Quantity = 1`；`FailureTag = TAG_ABILITY_FAIL_COST` |
| `CheckCost()` | 把 Ability 转成 `ULyraGameplayAbility_FromEquipment` → `GetAssociatedItem()` → 比较 Tag 堆栈数量；**不够就往 `OptionalRelevantTags` 加 `FailureTag`** |
| `ApplyCost()` | **只在权威端** → `ItemInstance->RemoveStatTagStack(Tag, NumStacks)` |

> 💡 **和 `PlayerTagStack` 的关键区别**：这个消耗是**绑定在具体武器上的**，所以换枪时弹药跟着走；而 PlayerTagStack 是绑定在玩家身上的（换武器不影响蓝量）。
