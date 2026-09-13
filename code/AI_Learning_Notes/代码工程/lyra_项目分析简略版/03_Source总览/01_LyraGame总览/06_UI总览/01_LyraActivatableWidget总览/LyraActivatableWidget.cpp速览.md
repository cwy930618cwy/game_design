# `LyraActivatableWidget.cpp` 速览

> 55 行，两个函数。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 空 |
| `GetDesiredInputConfig()` | `switch (InputConfig)` 把枚举翻译成 `FUIInputConfig`；`Menu` 模式固定用 `NoCapture`（鼠标要能自由移动去点按钮）；其余用 `GameMouseCaptureMode`；`Default` 返回空（不改） |
| `ValidateCompiledWidgetTree()`（编辑器） | ⭐ 如果子类没实现 `BP_GetDesiredFocusTarget`：直接继承本类 → **Warning**；隔了一层原生子类 → **Note**（因为可能父类已经实现了） |

> 💡 **那条编译警告说了什么**：`"GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen."`
> 手柄需要知道"焦点默认落在哪个控件上"，没实现就用手柄操作不了这个界面。这是 Lyra 为手柄适配加的编译期提醒。

**优先级**：`GetDesiredInputConfig`
