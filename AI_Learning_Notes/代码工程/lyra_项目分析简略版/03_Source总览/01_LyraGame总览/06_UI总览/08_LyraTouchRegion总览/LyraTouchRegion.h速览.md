# `LyraTouchRegion.h` 速览

> 触摸区域 —— 手指按住屏幕这块区域，就持续触发某个输入。最简单的模拟输入控件。

| 成员 | 干嘛的 |
|---|---|
| `: ULyraSimulatedInputWidget` | 继承模拟输入基类 |
| 四个触摸/ Tick 重写 | `NativeOnTouchStarted` / `Moved` / `Ended` / `NativeTick` |
| `ShouldSimulateInput()` | `BlueprintCallable`，查询当前是否在模拟 |
| `bShouldSimulateInput` | 手指是否正按着 |

**说明**：和 `LyraJoystickWidget` 的区别 —— 这个只输出"按下了"（值恒为 1），摇杆会输出二维向量。
