# `SHitMarkerConfirmationWidget.h` 速览

> ⭐ **手写 Slate** 的命中标记控件。这是 `HitMarkerConfirmationWidget` 的真正实现。

| 成员 | 干嘛的 |
|---|---|
| `: SLeafWidget` | 叶子控件（没有子控件，全靠自己画） |
| SLATE 参数默认值 | `PerHitMarkerImage` 用 `Throbber.CircleChunk`；`HitNotifyDuration` 0.4 |
| `Construct()` | 存参数 + 玩家上下文 + 部位覆盖图 |
| `OnPaint()` | ⭐ 画所有命中标记 |
| `Tick()` | ⭐ 算当前透明度 |
| `ComputeDesiredSize()` | 固定返回 `100×100` |
| `ComputeVolatility()` | 返回 `true`（每帧可能变） |
| `HitNotifyOpacity` | 当前透明度（0~1） |
| `MyContext` | 玩家上下文 |

> 💡 **`SLATE_ATTRIBUTE` vs `SLATE_ARGUMENT`**：`HitNotifyDuration` 和 `ColorAndOpacity` 是 ATTRIBUTE（可以绑委托动态变），其余是 ARGUMENT（构造时定死）。

**优先级**：`OnPaint` → `Tick`
