# `LyraSafeZoneEditor.cpp` 速览

> 110 行。**这个是"原版"**，`LyraBrightnessEditor` 是从它复制改名的。

| 函数 | 干嘛的 |
|---|---|
| 两个常量 | `JoystickDeadZone = 0.2`、`SafeZoneChangeSpeed = 0.1` |
| 构造函数 | Visible + 可聚焦 |
| `NativeOnInitialized()` | 切换器显示默认文本 |
| `NativeOnActivated()` | `SSafeZone::SetGlobalSafeZoneScale(GetSafeZone())`；绑按钮 |
| `ExecuteActionForSetting_Implementation()` | 取子设置项第一个转成数值设置 |
| `NativeOnAnalogValueChanged()` | 左摇杆 Y：钳到 0~1，步进 0.1 |
| `NativeOnMouseWheel()` | 滚轮：同上 |
| `HandleInputModeChanged()` | 提示文字：**"Use Left Stick / Mouse Wheel to adjust the corners so it lines up with the edges of your display."** |
| `HandleBackClicked()` | 恢复原值并关闭 |
| `HandleDoneClicked()` | ⭐ 有 ValueSetting 就 `SetValue`，否则 `ULyraSettingsLocal::SetSafeZone`；广播并关闭 |

> 💡 **安全区的作用**：电视/显示器常有"过扫描"，画面边缘会被裁掉一点。安全区缩放让 UI 往里缩，保证完整可见。手柄和鼠标滚轮两种调节方式都支持，是为了兼容主机和 PC。

**优先级**：`HandleDoneClicked` → 两个输入处理
