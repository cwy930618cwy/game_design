# `SCircumferenceMarkerWidget.h` 速览

> ⭐ 手写 Slate 的环形刻度控件。**注意它也带 `generated.h`** —— 因为 `FCircumferenceMarkerEntry` 是个 USTRUCT。

| 成员 | 干嘛的 |
|---|---|
| `FCircumferenceMarkerEntry`（USTRUCT） | 一条刻度：`PositionAngle`（放在圆周哪个角度）+ `ImageRotationAngle`（图片自身旋转多少），单位都是度 |
| `: SLeafWidget` | 叶子控件，全靠自己画 |
| SLATE 参数默认值 | `MarkerBrush` 用 `Throbber.CircleChunk`；`Radius` 默认 48 |
| `OnPaint()` | 画所有刻度 |
| `ComputeDesiredSize()` | `(图片尺寸 + 半径) × 2` |
| `ComputeVolatility()` | `true` |
| `SetRadius()` / `SetMarkerList()` | 运行时改 |
| `GetMarkerRenderTransform()`（private） | ⭐ 算每个刻度的变换矩阵 |

**优先级**：`GetMarkerRenderTransform` → `OnPaint`
