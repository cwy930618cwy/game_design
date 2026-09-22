# `LyraConfirmationScreen.h` 速览

> 确认对话框（"确定要退出吗"那种）。由 `LyraUIMessaging` 推到 `UI.Layer.Modal` 层。

| 成员 | 干嘛的 |
|---|---|
| `: UCommonGameDialog` | 来自 CommonGame 插件 |
| `SetupDialog()` | ⭐ 填标题、正文、动态创建按钮 |
| `KillDialog()` | 只调 Super |
| `NativeOnInitialized()` | 绑定"点击外部关闭" |
| `CloseConfirmationWindow()` | 关闭并回调结果 |
| `HandleTapToCloseZoneMouseButtonDown()` | 点空白处 = 取消 |
| `Text_Title` / `RichText_Description` / `EntryBox_Buttons` / `Border_TapToCloseZone` | 四个 `BindWidget` |
| `CancelAction` | 取消动作的数据表行（`RowType = CommonInputActionDataBase`） |
| `OnResultCallback` | 结果回调 |

**优先级**：`SetupDialog` → `CloseConfirmationWindow`
