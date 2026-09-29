# `SHitMarkerConfirmationWidget.cpp` 速览

> 106 行。**数据源是 `ULyraWeaponStateComponent`**。

| 函数 | 干嘛的 |
|---|---|
| `Construct()` | 存参数、`bColorAndOpacitySet` 标记、玩家上下文 |
| `OnPaint()` | ⭐ 见下 |
| `ComputeDesiredSize()` | 固定 `100×100` |
| `Tick()` | ⭐ 见下 |

## `Tick()` 怎么算透明度

```
HitNotifyOpacity = 0
取 ULyraWeaponStateComponent
TimeSinceLastHitNotification = 距上次命中的时间
if (< HitNotifyDuration)
    HitNotifyOpacity = 1 - (时间 / 时长)     ← 线性淡出
```

## `OnPaint()` 画什么

```
透明度 > 0 才画

① 逐命中点画标记：
   从 WeaponStateComponent 取 GetLastWeaponDamageScreenLocations()
   每个点：按 HitZone Tag 在覆盖图 Map 里找，找不到用默认图
   ★ 位置换算：Hit.Location + MyCullingRect.GetTopLeft()
     （注释：非全屏时要算上窗口边框的偏移）
   
② 如果配了 AnyHitsMarkerImage：在准星中心再画一个
```

> 💡 **数据来源有意思**：这个 Slate 控件**不接收任何事件**，而是每帧主动去 `ULyraWeaponStateComponent` 上"拉"最新的命中列表。这是 Slate 层的常见做法（比让 UMG 层推数据更简单）。

**优先级**：`OnPaint` → `Tick`
