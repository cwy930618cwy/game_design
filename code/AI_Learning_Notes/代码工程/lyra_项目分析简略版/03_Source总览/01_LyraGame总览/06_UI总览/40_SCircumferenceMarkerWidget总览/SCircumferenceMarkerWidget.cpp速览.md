# `SCircumferenceMarkerWidget.cpp` 速览

> 100 行，六个函数。

| 函数 | 干嘛的 |
|---|---|
| `Construct()` | 存参数 |
| `GetMarkerRenderTransform()` | ⭐ 见下 |
| `OnPaint()` | 透明度够就遍历刻度逐个画 |
| `ComputeDesiredSize()` | `(图片尺寸 + 半径) × 2` |
| `SetRadius()` | ⭐ 变了才赋值 + `Invalidate(Layout)`（**要重新布局**） |
| `SetMarkerList()` | 直接赋值（**没有 Invalidate**） |

## `GetMarkerRenderTransform()` 的两步变换

```
① 先绕图片自身中心旋转 ImageRotationAngle
   （Concatenate: 平移到原点 → 旋转 → 平移回去）
② 再平移到圆周上的位置：
   X = Radius × Sin(PositionAngle) × HUDScale
   Y = -Radius × Cos(PositionAngle) × HUDScale
   （注意 Y 取负，因为屏幕坐标 Y 向下）
```

> 💡 如果勾了 `bReticleCornerOutsideSpreadRadius`，半径会再加上半个图片宽度，让刻度**落在散布圈外侧**而不是压在线上。
>
> ⚠️ `SetMarkerList` 没调 `Invalidate` —— 改了刻度列表后**不会自动重绘**，需要靠别的方式触发（比如同时改 Radius）。这是个容易踩的小坑。

**优先级**：`GetMarkerRenderTransform`
