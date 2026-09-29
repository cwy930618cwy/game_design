# `LyraHUDLayout.h` 速览

> HUD 的整体布局控件（由 Experience 的 "Add Widgets" 指定）。**负责两件事：ESC 菜单、手柄断连提示**。

| 成员 | 干嘛的 |
|---|---|
| `: ULyraActivatableWidget`（`Abstract, Blueprintable`） | 继承界面基类 |
| `NativeOnInitialized()` / `NativeDestruct()` | 绑定/解绑 |
| `HandleEscapeAction()` | ⭐ 打开 ESC 菜单 |
| `HandleInputDeviceConnectionChanged()` / `HandleInputDevicePairingChanged()` | 监听手柄插拔和配对变化 |
| `NotifyControllerStateChangeForDisconnectScreen()` | 延迟到下一 tick 再处理（避免一帧内多次触发） |
| `ProcessControllerDevicesHavingChangedForDisconnectScreen()` | ⭐ 真正判断"还有没有手柄连着" |
| `ShouldPlatformDisplayControllerDisconnectScreen()` | 当前平台需不需要这个功能（按平台 Trait Tag） |
| `DisplayControllerDisconnectedMenu()` / `HideControllerDisconnectedMenu()` | `BlueprintNativeEvent`，显示/隐藏断连提示 |
| `EscapeMenuClass` | ESC 菜单的控件类 |
| `ControllerDisconnectedScreen` | 断连提示的控件类 |
| `PlatformRequiresControllerDisconnectScreen` | ⭐ 需要此功能的平台 Tag 容器 |
| `SpawnedControllerDisconnectScreen` | 当前显示的断连提示（`Transient`） |
| `RequestProcessControllerStateHandle` | `FTSTicker` 句柄，用于延迟一帧 |

**优先级**：`HandleEscapeAction` → `ProcessControllerDevicesHavingChangedForDisconnectScreen`
