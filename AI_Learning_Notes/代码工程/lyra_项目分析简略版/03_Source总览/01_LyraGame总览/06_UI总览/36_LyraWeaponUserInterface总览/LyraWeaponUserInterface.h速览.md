# `LyraWeaponUserInterface.h` 速览

> 武器 HUD 的容器：**监听当前武器，换了就通知蓝图重建界面**。

| 成员 | 干嘛的 |
|---|---|
| `: UCommonUserWidget` | 标准控件 |
| `NativeConstruct()` / `NativeDestruct()` | ⚠️ **都只调 Super，什么都没做** |
| `NativeTick()` | ⭐ 每帧检查武器是否变了 |
| `OnWeaponChanged()` | `BlueprintImplementableEvent`，换武器时通知蓝图 |
| `RebuildWidgetFromWeapon()`（private） | ⚠️ **空函数** |
| `CurrentInstance` | 当前武器实例（`Transient`） |

**优先级**：`NativeTick`
