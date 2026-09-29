# `LyraBoundActionButton.cpp` 速览

> 46 行，两个函数。

| 函数 | 干嘛的 |
|---|---|
| `NativeConstruct()` | 订阅 `OnInputMethodChangedNative`，并**立刻用当前输入方式调一次**（保证初始状态就对） |
| `HandleInputMethodChanged()` | `Gamepad → GamepadStyle`；`Touch → TouchStyle`；其余（键鼠）→ `KeyboardStyle`；样式非空才 `SetStyle` |

> 💡 **那个"立刻调一次"很重要**：`OnInputMethodChanged` 只在**切换时**触发，如果玩家从一开始就是手柄，不主动调一次的话样式就是错的。

> ⚠️ 三个样式都是 `EditAnywhere` 但**没设默认值**，也没判空保护 —— 某个样式没配的话就用回上一个（或初始）样式，不会崩但可能显示不对。
