# `LyraReticleWidgetBase.cpp` 速览

> 96 行。**最精彩的是那个"把散布角换算成屏幕像素"的算法**。

| 函数 | 干嘛的 |
|---|---|
| `InitializeFromWeapon()` | 存武器 → 从 `GetInstigator()` 取物品实例 → 调蓝图事件 |
| `ComputeSpreadAngle()` | `基础角 × 倍率`（都从 `ULyraRangedWeaponInstance` 读）；非远程武器返回 0 |
| `HasFirstShotAccuracy()` | 转发给远程武器实例 |
| `ComputeMaxScreenspaceSpreadRadius()` | ⭐ 见下 |

## 散布角 → 屏幕像素

```
1. 取相机位置和朝向
2. 在正前方 LongShotDistance（10000）处，
   沿相机 Up 方向偏移 Tan(半角) × 10000
   → 得到"散布圆锥边缘上的一个点"
3. 把这个点投影到屏幕
4. 返回它到屏幕中心的距离（像素）
```

> 💡 源码注释解释了这个思路：**武器的散布可以想成一个圆锥**，在很远处取圆锥边缘上的一点，投影回屏幕，它到中心的距离就是准星该张开的半径。
>
> 注释也坦承这**不是完美的** —— 因为相机位置和枪口之间有一段距离，所以会有误差。

**优先级**：`ComputeMaxScreenspaceSpreadRadius`
