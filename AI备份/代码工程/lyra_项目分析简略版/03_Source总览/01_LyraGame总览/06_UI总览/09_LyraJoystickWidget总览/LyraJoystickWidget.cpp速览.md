# `LyraJoystickWidget.cpp` 速览

> 108 行。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | `SetConsumePointerInput(true)` |
| `NativeOnTouchStarted()` | 记录 `TouchOrigin`，并捕获鼠标（`CaptureMouse`） |
| `NativeOnTouchMoved()` | `HandleTouchDelta()` + 确保捕获鼠标 |
| `NativeOnTouchEnded()` | `StopInputSimulation()` + 释放鼠标捕获 |
| `NativeOnMouseLeave()` | 同样停止（防止手指滑出去卡住） |
| `NativeTick()` | ⭐ 见下 |
| `HandleTouchDelta()` | ⭐ 见下 |
| `StopInputSimulation()` | 原点和向量都归零 |

## `HandleTouchDelta()` 怎么算

```
1. 取触摸的屏幕坐标 ScreenSpacePos
2. 算出摇杆中心的屏幕坐标（几何体本地尺寸 → 绝对坐标）
3. 偏移 = 触摸点 - 中心
4. bNegateYAxis 时反转 Y
5. 拆成方向 + 长度，长度钳制到 StickRange
6. StickVector = 偏移 / StickRange      ← 归一化到 -1 ~ 1
```

## `NativeTick()` 做的两件事

```
① 移动前景图：SetRenderTranslation(StickVector × 背景图尺寸 × 0.5)
② InputKeyValue2D(StickVector)
```

> 💡 **为什么要在 Tick 里做而不是在 TouchMoved 里？** 因为即使手指不动（比如滑到边缘后停住），也需要**持续每帧注入**当前向量，否则输入系统认为"没输入了"。这和 `LyraTouchRegion` 是同样的思路。

**优先级**：`HandleTouchDelta` → `NativeTick`
