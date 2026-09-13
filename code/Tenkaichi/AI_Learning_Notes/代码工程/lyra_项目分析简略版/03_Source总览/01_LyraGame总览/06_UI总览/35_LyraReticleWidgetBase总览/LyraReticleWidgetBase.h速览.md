# `LyraReticleWidgetBase.h` 速览

> **准星的基类**。把"武器当前的散布"换算成准星该画多大。

| 成员 | 干嘛的 |
|---|---|
| `: UCommonUserWidget`（`Abstract`） | 标准控件 |
| `InitializeFromWeapon()` | 绑定一把武器，并触发蓝图事件 |
| `OnWeaponInitialized()` | `BlueprintImplementableEvent`，给蓝图做初始化 |
| `ComputeSpreadAngle()` | ⭐ 当前散布角（度） |
| `ComputeMaxScreenspaceSpreadRadius()` | ⭐ 散布在**屏幕上**的像素半径 |
| `HasFirstShotAccuracy()` | 是否处于"首发精度"状态 |
| `WeaponInstance` / `InventoryInstance` | 绑定的武器实例 / 物品实例 |

**优先级**：`ComputeMaxScreenspaceSpreadRadius` → `ComputeSpreadAngle`
