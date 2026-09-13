# `LyraBrightnessEditor.cpp` 速览

> 110 行。**读完会发现一个明显的复制粘贴痕迹**。

| 函数 | 干嘛的 |
|---|---|
| 两个常量 | `JoystickDeadZone = 0.2`、`SafeZoneChangeSpeed = 0.1` |
| 构造函数 | 设为 Visible + 可聚焦 |
| `NativeOnInitialized()` | 把切换器显示到默认文本 |
| `NativeOnActivated()` | ⭐ `SSafeZone::SetGlobalSafeZoneScale(GetSafeZone())`；绑"完成"；按 `bCanCancel` 决定"返回" |
| `ExecuteActionForSetting_Implementation()` | 取子设置项的第一个，转成 `UGameSettingValueScalar` 存起来 |
| `NativeOnAnalogValueChanged()` | 手柄左摇杆 Y：`当前值 + 模拟量 × 0.1`，钳到 0~1 |
| `NativeOnMouseWheel()` | 鼠标滚轮：同理，用 `WheelDelta` |
| `HandleInputModeChanged()` | 提示文字里**说的是 "adjust the brightness"** |
| `HandleBackClicked()` | 取消：恢复安全区并关闭 |
| `HandleDoneClicked()` | ⭐ 有 ValueSetting 就写回它；否则直接 `SetSafeZone`；广播并关闭 |

## ⚠️ 类名和内容对不上

```
类名叫 BrightnessEditor，提示文字也说 "adjust the brightness"
但实际调的是 SSafeZone::SetGlobalSafeZoneScale（安全区缩放）
```

> 这个文件**和 `LyraSafeZoneEditor.cpp` 几乎一模一样**（常量、结构、逻辑全同，只有提示文字和类名不同）。看起来是复制了安全区编辑器改名而来，**但改动没做完** —— 亮度其实应该去改 `DisplayGamma`。
>
> 而且 `HandleInputModeChanged` 似乎没被调用（没看到订阅输入方式变化的代码），所以提示文字可能一直是默认的。

**优先级**：`HandleDoneClicked` → 两个输入处理
