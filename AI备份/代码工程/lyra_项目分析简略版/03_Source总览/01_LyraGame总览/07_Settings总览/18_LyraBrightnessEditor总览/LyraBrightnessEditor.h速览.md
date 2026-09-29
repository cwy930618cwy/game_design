# `LyraBrightnessEditor.h` 速览

> 亮度调节界面。⚠️ **看代码会发现它其实在调"安全区"，不是亮度**（见 cpp）。

| 成员 | 干嘛的 |
|---|---|
| `: UCommonActivatableWidget` + `IGameSettingActionInterface` | 界面 + 设置动作接口 |
| `OnSafeZoneSet` | `FSimpleMulticastDelegate`，设置完成时广播 |
| `ExecuteActionForSetting_Implementation()` | ⭐ 从传入设置项的子项里取出数值设置 |
| `bCanCancel` | 能不能取消（决定"返回"按钮显不显示） |
| `NativeOnActivated()` / `NativeOnInitialized()` | 生命周期 |
| `NativeOnAnalogValueChanged()` / `NativeOnMouseWheel()` | ⭐ 手柄左摇杆 / 鼠标滚轮 调节 |
| `HandleInputModeChanged()` | 按输入设备换提示文字 |
| `HandleBackClicked()` / `HandleDoneClicked()` | 取消 / 确定 |
| `ValueSetting` | 弱引用到数值设置项 |
| 4 个 `BindWidget` | 切换器、富文本、返回按钮、完成按钮 |

**优先级**：`HandleDoneClicked` → 两个输入处理
