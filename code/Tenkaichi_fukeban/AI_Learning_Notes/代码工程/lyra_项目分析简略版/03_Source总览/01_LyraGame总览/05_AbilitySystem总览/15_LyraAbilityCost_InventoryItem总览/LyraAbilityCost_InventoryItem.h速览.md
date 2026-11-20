# `LyraAbilityCost_InventoryItem.h` 速览

> 消耗**背包里的某个物品**。菜单里显示 "Inventory Item"。
> ⚠️ **注意：这个功能目前被官方关掉了。**

| 成员 | 干嘛的 |
|---|---|
| `: ULyraAbilityCost` | 继承消耗基类 |
| `CheckCost()` / `ApplyCost()` | 两个 override |
| `Quantity` | 按技能等级取数量 |
| `ItemDefinition` | 消耗哪种物品（`TSubclassOf<ULyraInventoryItemDefinition>`） |

> ⚠️ 对应的 `.cpp` 里两个函数体**整个被 `#if 0 ... #endif` 包住**，`CheckCost` 直接 `return false`。也就是说**当前版本任何配了这个消耗的技能都放不出来**。

**说明**：先知道它的存在即可，实际别用。
