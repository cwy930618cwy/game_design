# `LyraButtonBase.cpp` 速览

> 55 行，五个函数。

| 函数 | 干嘛的 |
|---|---|
| `NativePreConstruct()` | `UpdateButtonStyle()` + `RefreshButtonText()` |
| `UpdateInputActionWidget()` | Super 之后再刷一次样式和文字（**输入动作变了**） |
| `SetButtonText()` | ⚠️ 注意：`bOverride_ButtonText = InText.IsEmpty()` —— **传空字符串反而是"开"覆盖** |
| `RefreshButtonText()` | ⭐ 见下 |
| `OnInputMethodChanged()` | 只刷样式，不刷文字 |

## `RefreshButtonText()` 的优先级

```
if (bOverride_ButtonText || ButtonText.IsEmpty())
    如果有 InputActionWidget 且它有显示文字 → 用它（如 "按 A"）
否则
    用 ButtonText（自己配的文字）
```

> 💡 **这就是"按钮文字自动跟着改键变"的实现**：没配文字（或主动要求覆盖）时，就用绑定动作的显示名。玩家把"跳跃"从空格改成 F，按钮上的文字会自动跟着变成 F。

> ⚠️ `SetButtonText` 那行 `bOverride_ButtonText = InText.IsEmpty()` 语义有点绕 —— 传空 = 开覆盖，传非空 = 关覆盖。读代码时容易看反。
