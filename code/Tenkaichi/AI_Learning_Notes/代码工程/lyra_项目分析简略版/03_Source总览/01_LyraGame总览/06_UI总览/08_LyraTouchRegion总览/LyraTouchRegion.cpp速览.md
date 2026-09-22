# `LyraTouchRegion.cpp` 速览

> 41 行，**四个函数全是几行代码**。

| 函数 | 干嘛的 |
|---|---|
| `NativeOnTouchStarted()` | `bShouldSimulateInput = true` |
| `NativeOnTouchMoved()` | 同样置 true（注释说明有一行 `InputKeyValue` 被注释掉了） |
| `NativeOnTouchEnded()` | `bShouldSimulateInput = false` |
| `NativeTick()` | ⭐ `if (bShouldSimulateInput) InputKeyValue(FVector::OneVector)` —— 每帧注入 1 |

> 💡 **注意它是在 Tick 里持续注入的**，不是靠事件。因为"按住"这个状态需要每帧告诉输入系统。
>
> ⚠️ 另外 `NativeOnTouchMoved` 里那句 `//InputKeyValue(FVector::OneVector);` 被注释掉了 —— 移动时不额外注入，只靠 Tick 就够了（但会多一次状态写入）。
