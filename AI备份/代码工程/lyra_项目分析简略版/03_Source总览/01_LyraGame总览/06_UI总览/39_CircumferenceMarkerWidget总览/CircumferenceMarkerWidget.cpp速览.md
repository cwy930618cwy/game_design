# `CircumferenceMarkerWidget.cpp` 速览

> 50 行，四个函数。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | `HitTestInvisible` + `bIsVolatile = true` |
| `ReleaseSlateResources()` | `MyMarkerWidget.Reset()` |
| `RebuildWidget()` | `SNew(SCircumferenceMarkerWidget)` 并传入 图片 / 半径 / 列表 |
| `SynchronizeProperties()` | ⭐ 把 Radius 和 MarkerList 同步下去 |
| `SetRadius()` | ⭐ 存值并转发给 Slate 控件（**带 IsValid 判空**） |

> 💡 **`SetRadius` 是关键**：准星要跟着武器散布实时张开收拢，就是每帧调这个函数。而 `Radius` 的值通常来自 `LyraReticleWidgetBase::ComputeMaxScreenspaceSpreadRadius()`。
>
> 两个类配合起来就是完整的动态准星：一个算该多大，一个负责画。

**优先级**：`SetRadius`
