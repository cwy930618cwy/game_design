# `LyraLoadingScreenSubsystem.cpp` 速览

> 33 行，三个函数，**全目录最简单的实现之一**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 空 |
| `SetLoadingScreenContentWidget()` | ⭐ **先比较再赋值**，不一样才广播 |
| `GetLoadingScreenContentWidget()` | 直接返回 |

> 💡 那个 `if (LoadingScreenWidgetClass != NewWidgetClass)` 是标准做法 —— 避免重复广播导致的界面重建。
