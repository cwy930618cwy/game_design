# `LyraControllerDisconnectedScreen.h` 速览

> 手柄全部断开时显示的提示界面（由 `LyraHUDLayout` 推出来）。

| 成员 | 干嘛的 |
|---|---|
| `: UCommonActivatableWidget` | 标准可激活界面 |
| `NativeOnActivated()` | ⭐ 决定是否显示"切换用户"按钮 |
| `HandleChangeUserClicked()` | 打开平台的用户选择界面 |
| `HandleChangeUserCompleted()` | ⚠️ 选择完成回调 —— **空的，只有日志和 TODO** |
| `ShouldDisplayChangeUserButton()` | 按平台 Trait 判断 |
| `PlatformSupportsUserChangeTags` | 需要哪些平台 Trait 才显示该按钮 |
| `HBox_SwitchUser` / `Button_ChangeUser` | 两个 `BindWidget` |

> 💡 "切换用户"是主机特有的需求：手柄和登录用户是绑定的，换个人拿手柄就得切换用户身份。PC 上不需要，所以按平台 Trait 控制显隐。

**优先级**：`NativeOnActivated` → `ShouldDisplayChangeUserButton`
