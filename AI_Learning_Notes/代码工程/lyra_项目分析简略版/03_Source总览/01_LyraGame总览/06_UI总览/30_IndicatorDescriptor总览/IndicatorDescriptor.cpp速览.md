# `IndicatorDescriptor.cpp` 速览

> 141 行，**主体就是 `FIndicatorProjection::Project` 这个函数**。

| 函数 | 干嘛的 |
|---|---|
| `FIndicatorProjection::Project()` | ⭐ 见下 |
| `SetIndicatorManagerComponent()` | `ensure(ManagerPtr 必须是空)` —— **只能设一次，防止重复绑定** |
| `UnregisterIndicator()` | 转调 Manager 的 `RemoveIndicator` |

## 投影的三个分支（对应 5 种模式）

```
① ComponentPoint
    取组件位置（有插槽就用插槽）→ GetPixelPoint → 加屏幕偏移
    ★ 如果目标在相机背后，把屏幕坐标"翻到"屏幕外侧（沿用原方向的延长线）

② *ScreenBoundingBox（两种）
    投影包围盒得到屏幕矩形的左下/右上
    按 BoundingBoxAnchor 在矩形内插值取点

③ *BoundingBox（两种）
    在包围盒内按 Anchor 取一个世界点 → 再投影成屏幕点
```

> 💡 **"相机背后"的处理值得一看**：目标在身后时投影出来的坐标是镜像的，Lyra 会把这个点**沿屏幕中心向外推到屏幕外**，这样贴边逻辑才能正常工作，指示箭头也能指对方向。
>
> 输出是 `FVector(X, Y, Depth)` —— 第三个分量是**到相机的距离**，用于排序（近的盖住远的）。

**优先级**：`Project` → `SetIndicatorManagerComponent`
