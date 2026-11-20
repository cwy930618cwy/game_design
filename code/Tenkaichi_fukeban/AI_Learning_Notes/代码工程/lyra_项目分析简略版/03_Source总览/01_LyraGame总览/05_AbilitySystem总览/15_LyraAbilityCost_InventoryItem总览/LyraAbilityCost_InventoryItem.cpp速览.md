# `LyraAbilityCost_InventoryItem.cpp` 速览

> 54 行，**但两个函数体整个被 `#if 0` 包住**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | `Quantity.SetValue(1.0f)` |
| `CheckCost()` | ⚠️ 实现被 `#if 0` 注释，实际只有 `return false` |
| `ApplyCost()` | ⚠️ 实现被 `#if 0` 注释，实际什么都不做 |

> 被注释掉的逻辑本来是：从 Controller 上找 `ULyraInventoryManagerComponent`，检查/消耗指定物品定义的数量。
>
> 原因不明（可能是背包 API 变动后没跟上）。**结论：这个消耗类型目前不可用。**

**优先级**：无（当前不可用）
