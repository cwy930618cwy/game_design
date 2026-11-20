# `LyraSettingsListEntrySetting_KeyboardInput.h` 速览

> 改键列表里的**一行**（控件）：主键按钮、副键按钮、清除、恢复默认。

| 成员 | 干嘛的 |
|---|---|
| `: UGameSettingListEntry_Setting` | 设置列表条目的基类 |
| `SetSetting()` | 绑定到一个 `ULyraSettingKeyboardInput` |
| `NativeOnInitialized()` / `NativeOnEntryReleased()` | 初始化 / 释放（**列表条目会被复用**） |
| `OnSettingChanged()` | 设置变了就 `Refresh()` |
| 4 个点击处理 | `HandlePrimaryKeyClicked` / `HandleSecondaryKeyClicked` / `HandleClearClicked` / `HandleResetToDefaultClicked` |
| 4 个选键回调 | 主键/副键 选中、主键/副键 重复键警告确认 |
| `ChangeBinding()` | ⭐ 见 cpp |
| 两个 `HandleKeySelectionCanceled()` | 重载（分别对应两种弹窗） |
| `Refresh()` | 刷新按钮文字和"恢复默认"的显隐 |
| `KeyboardInputSetting` | 绑定的设置项 |
| 两个面板类 | `PressAnyKeyPanelClass` / `KeyAlreadyBoundWarningPanelClass` |
| 4 个 `BindWidget` 按钮 | 主键、副键、清除、恢复默认 |
| `OriginalKeyToBind` | 记录"想绑的键"，用于重复键确认后再执行 |

**优先级**：`ChangeBinding` → `Refresh`
