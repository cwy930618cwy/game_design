# `LyraJoystickWidget.h` 速览

> 虚拟摇杆。UMG 包装，**算出一个 -1~1 的二维向量注入给 EnhancedInput**，模拟手柄摇杆。

| 成员 | 干嘛的 |
|---|---|
| `: ULyraSimulatedInputWidget` | 继承模拟输入基类 |
| 五个 UUserWidget 重写 | 触摸开始/移动/结束、鼠标离开、Tick |
| `HandleTouchDelta()` | ⭐ 核心：由触摸位置算出摇杆向量 |
| `StopInputSimulation()` | 停止并归零 |
| `StickRange` | 内圈能移动多远（默认 50） |
| `JoystickBackground` / `JoystickForeground` | `BindWidget` 的两张图 |
| `bNegateYAxis` | ⭐ 是否反转 Y 轴（移动摇杆通常要反转） |
| `TouchOrigin` / `StickVector` | 触摸原点、当前向量（`Transient`） |

**优先级**：`HandleTouchDelta` → `NativeTick`
