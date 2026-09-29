# `LyraSafeZoneEditor.h` 速览

> 安全区调节界面（电视上常见的"调整到与屏幕边缘对齐"）。

| 成员 | 干嘛的 |
|---|---|
| `: UCommonActivatableWidget` + `IGameSettingActionInterface` | 界面 + 设置动作接口 |
| `OnSafeZoneSet` | 设置完成时广播 |
| `ExecuteActionForSetting_Implementation()` | 从子设置项取数值设置 |
| `bCanCancel` | 能否取消 |
| `NativeOnActivated()` / `NativeOnInitialized()` | 生命周期 |
| `NativeOnAnalogValueChanged()` / `NativeOnMouseWheel()` | 手柄左摇杆 / 鼠标滚轮 调节 |
| `HandleInputModeChanged()` | 换提示文字（"Left Stick" / "Mouse Wheel"） |
| `HandleBackClicked()` / `HandleDoneClicked()` | 取消 / 确定 |
| `ValueSetting` | 弱引用数值设置项 |
| 4 个 `BindWidget` | 切换器、富文本、返回、完成 |

**说明**：这个文件的结构和 `LyraBrightnessEditor` 完全一致（后者是从它复制过去的）。
