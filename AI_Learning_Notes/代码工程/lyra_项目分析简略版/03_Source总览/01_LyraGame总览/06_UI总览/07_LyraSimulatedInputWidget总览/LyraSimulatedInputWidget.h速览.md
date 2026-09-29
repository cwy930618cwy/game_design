# `LyraSimulatedInputWidget.h` 速览

> ⭐ **移动端虚拟输入的基类**：把触摸事件注入到 EnhancedInput 子系统里，让触摸"假装"成手柄输入。

| 成员 | 干嘛的 |
|---|---|
| `: UCommonUserWidget` | 标准控件 |
| `GetEnhancedInputSubsystem()` / `GetPlayerInput()` | 拿当前本地玩家的 EnhancedInput 子系统 |
| `GetAssociatedAction()` / `GetSimulatedKey()` | 查询：模拟哪个 InputAction、用哪个键 |
| `InputKeyValue()` / `InputKeyValue2D()` | ⭐ 注入输入（一维 / 二维） |
| `FlushSimulatedInput()` | 清空已注入的按键状态 |
| `QueryKeyToSimulate()` | ⭐ 从 EnhancedInput 反查"这个 Action 当前绑了哪个键" |
| `OnControlMappingsRebuilt()` | 改键后重新查询 |
| `CommonVisibilityBorder` | `BindWidget`，可指定只在特定平台显示 |
| `AssociatedAction` | 要模拟哪个 `UInputAction` |
| `FallbackBindingKey` | 查不到绑定时的兜底键（默认 `Gamepad_Right2D`） |
| `KeyToSimulate` | 实际使用的键 |

> 💡 **这个设计的妙处**：不直接模拟按键，而是优先用 `InjectInputVectorForAction` 注入到 **InputAction** 上。所以玩家改键后虚拟摇杆依然生效 —— 靠 `OnControlMappingsRebuilt` 重新查询。

**优先级**：`InputKeyValue` → `QueryKeyToSimulate`
