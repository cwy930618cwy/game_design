# `LyraHUDLayout.cpp` 速览

> 192 行。**手柄断连处理占了三分之二**。

| 函数 | 干嘛的 |
|---|---|
| 三个静态 Tag | `UI.Layer.Menu`、`UI.Action.Escape`、`Platform.Trait.Input.PrimarlyController` |
| 构造函数 | 默认给 `PlatformRequiresControllerDisconnectScreen` 加上"主要用手柄的平台"这个 Tag |
| `NativeOnInitialized()` | 绑定 ESC 动作；**若平台需要断连提示**才去监听设备插拔/配对 |
| `NativeDestruct()` | 解绑并摘掉 Ticker |
| `HandleEscapeAction()` | `PushStreamedContentToLayer_ForPlayer(..., UI.Layer.Menu, EscapeMenuClass)` |
| 两个 `Handle...Changed()` | 先判断**是不是本玩家的设备**（不是就忽略），是才通知 |
| `ShouldPlatformDisplayControllerDisconnectScreen()` | 平台 Trait 要包含全部必需 Tag；**编辑器下还要并上"模拟的可见性 Tag"** |
| `NotifyControllerStateChangeForDisconnectScreen()` | ⭐ 用 `FTSTicker` 延迟到下一 tick，且**同时只排一个** |
| `ProcessControllerDevicesHavingChangedForDisconnectScreen()` | ⭐ 见下 |
| 两个 `..._Implementation()` | 推入/弹出 `UI.Layer.Menu` 层 |

## 断连判断逻辑

```
1. 取本玩家 PlatformUserId
2. 列出映射到他的所有输入设备
3. 逐个检查：连接状态 == Connected 且 主设备类型 == Gamepad
4. 一个都没有 → DisplayControllerDisconnectedMenu()
   有且当前正显示 → HideControllerDisconnectedMenu()
```

> 💡 **为什么要延迟一 tick？** 手柄拔插会在一帧内触发多个事件（连接变化 + 配对变化），直接处理会重复弹窗。用 Ticker 合并到下一帧只处理一次。

> 💡 编辑器下额外检查 `UCommonUIVisibilitySubsystem::GetVisibilityTags()`，是为了**能在 PC 上模拟主机平台的表现**来调试这个功能。

**优先级**：`ProcessControllerDevicesHavingChangedForDisconnectScreen` → `HandleEscapeAction`
