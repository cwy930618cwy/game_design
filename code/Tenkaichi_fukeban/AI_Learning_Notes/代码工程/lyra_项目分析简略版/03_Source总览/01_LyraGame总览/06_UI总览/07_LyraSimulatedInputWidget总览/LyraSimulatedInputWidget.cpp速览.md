# `LyraSimulatedInputWidget.cpp` 速览

> 171 行。**最值得看的是 `InputKeyValue` 里处理"成对按键"的那段**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | `SetConsumePointerInput(true)` —— 吃掉指针输入，不往下传 |
| `GetPaletteCategory()`（编辑器） | 控件面板分类设为 "Input" |
| `NativeConstruct()` | `QueryKeyToSimulate()` + 订阅 `ControlMappingsRebuiltDelegate` |
| `NativeDestruct()` | 取消订阅 |
| `NativeOnTouchEnded()` | 触摸结束 → `FlushSimulatedInput()` |
| `GetEnhancedInputSubsystem()` / `GetPlayerInput()` | 从 OwningPlayer → LocalPlayer 上取子系统 |
| `InputKeyValue()` | ⭐ 见下 |
| `InputKeyValue2D()` | 转成 FVector 再调上面 |
| `FlushSimulatedInput()` | `Input->FlushPressedKeys()` |
| `QueryKeyToSimulate()` | `QueryKeysMappedToAction(AssociatedAction)`，取第一个；取不到用 `FallbackBindingKey` |
| `OnControlMappingsRebuilt()` | 重新查询 |

## `InputKeyValue()` 的两条路

```
① 有 AssociatedAction → System->InjectInputVectorForAction(...)
   （注释：不需要修饰器和触发器，但函数签名要求传，所以传空的）

② 没有 Action 才退化成模拟按键：
   - 通过 IPlatformInputDeviceMapper 找到用户的主设备
   - ★ 如果这个键是"成对键"（如 Mouse2D = MouseX + MouseY）
       必须对 X/Y 分别调 InputKey，不能对 Mouse2D 本身调
   - 用 FInputKeyEventArgs::CreateSimulated 构造事件
```

> 💡 **成对键那段注释写得非常详细**：引擎里所有来自消息处理器和视口的输入事件都是"分别上报 X/Y"的，所以模拟输入时也必须照做，否则 `UPlayerInput` 的内部按键状态不会正确累积。这是个很容易踩的坑。

**优先级**：`InputKeyValue` → `QueryKeyToSimulate`
