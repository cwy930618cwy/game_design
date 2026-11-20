# `LyraBoundActionButton.h` 速览

> "绑定按键提示"的按钮（显示"按 X 执行"那种）。**核心功能：按当前输入设备自动换样式**。

| 成员 | 干嘛的 |
|---|---|
| `: UCommonBoundActionButton`（`Abstract`, `DisableNativeTick`） | 标准继承 |
| `NativeConstruct()` | 订阅输入方式变化 |
| `HandleInputMethodChanged()`（private） | ⭐ 按设备换样式 |
| `KeyboardStyle` / `GamepadStyle` / `TouchStyle` | 三套按钮样式类 |

> 💡 这是 CommonUI 的典型用法：手柄上要显示"按 A"，键鼠上要显示"按 E"，样式完全不同 —— 这里做成三套自动切换。

**优先级**：`HandleInputMethodChanged`
