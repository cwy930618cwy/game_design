# `LyraButtonBase.h` 速览

> 所有 Lyra 按钮的基类。**核心功能：按钮上的文字可以自动变成"当前按键名"**。

| 成员 | 干嘛的 |
|---|---|
| `: UCommonButtonBase`（`Abstract, BlueprintType, Blueprintable`） | 标准继承 |
| `SetButtonText()` | 设置文字 |
| `NativePreConstruct()` | 初始化时刷一次样式和文字 |
| `UpdateInputActionWidget()` | 输入动作变化时刷新 |
| `OnInputMethodChanged()` | 输入设备变化时刷样式 |
| `RefreshButtonText()` | ⭐ 决定到底显示什么文字 |
| `UpdateButtonText()` / `UpdateButtonStyle()` | 两个 `BlueprintImplementableEvent`，交给蓝图去真正更新 |
| `bOverride_ButtonText` + `ButtonText` | ⭐ 开关 + 值 |

> 💡 **"转发给蓝图事件"这个模式**值得注意：C++ 只负责决定"该显示什么"，具体怎么显示交给蓝图 —— 这样不同风格的按钮可以有完全不同的实现。

**优先级**：`RefreshButtonText` → `UpdateInputActionWidget`
