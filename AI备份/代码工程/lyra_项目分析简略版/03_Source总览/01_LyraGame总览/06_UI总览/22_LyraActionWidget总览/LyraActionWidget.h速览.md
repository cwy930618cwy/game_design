# `LyraActionWidget.h` 速览

> 31 行。**显示"某个动作当前绑定在哪个键"的图标**，且支持玩家改键后自动更新。

| 成员 | 干嘛的 |
|---|---|
| `: UCommonActionWidget`（`BlueprintType, Blueprintable`） | 标准继承 |
| `GetIcon()` | ⭐ 唯一的 override |
| `AssociatedInputAction` | 关联的 `UInputAction` |
| `GetEnhancedInputSubsystem()`（private） | 拿子系统 |

**说明**：看 cpp。
