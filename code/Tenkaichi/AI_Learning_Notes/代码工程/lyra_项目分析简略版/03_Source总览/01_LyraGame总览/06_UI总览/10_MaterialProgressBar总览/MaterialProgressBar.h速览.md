# `MaterialProgressBar.h` 速览

> **用材质做的进度条**（不是图片拉伸）。支持分段、发光、描边，还能播放填充动画。

| 成员 | 干嘛的 |
|---|---|
| `: UCommonUserWidget`（`Abstract`, `DisableNativeTick`） | 标准控件 |
| `SetProgress()` / `SetStartProgress()` | 设置结束值 / 起始值（用于动画） |
| `SetColorA()` / `SetColorB()` / `SetColorBackground()` | 三个颜色 |
| `AnimateProgressFromStart()` / `AnimateProgressFromCurrent()` | ⭐ 从指定值 / 从当前值播放填充动画 |
| `OnFillAnimationFinished` | 动画播完的广播 |
| `SynchronizeProperties()` | 同步所有参数到材质 |
| `OnWidgetRebuilt()`（编辑器） | ⭐ **反向**从材质读回默认值 |
| `Image_Bar` / `BoundAnim_FillBar` | `BindWidget` / `BindWidgetAnim` |
| 一堆 `bOverrideDefaultXXX` + 对应值 | ⭐ 每个参数都是"可选覆盖"模式 |
| `StrokeMaterial` / `NoStrokeMaterial` | 带描边 / 不带描边两套材质 |
| `DesignTime_Progress` | 编辑器里预览用的进度 |

> 💡 **"可选覆盖"模式值得学**：每个参数配一个 `bool bOverride...`，不勾就用材质里预设的值，勾了才用这边的。这样美术能在材质里调默认外观，程序只在需要时覆盖。

**优先级**：`AnimateProgressFromCurrent` → `SynchronizeProperties`
