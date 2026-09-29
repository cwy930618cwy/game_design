# `HitMarkerConfirmationWidget.cpp` 速览

> 41 行，三个函数。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 设为 `HitTestInvisible`；`bIsVolatile = true`（**每帧重绘**）；`AnyHitsMarkerImage.DrawAs = NoDrawType`（**默认不画**） |
| `ReleaseSlateResources()` | `MyMarkerWidget.Reset()` |
| `RebuildWidget()` | ⭐ 见下 |

## `RebuildWidget()` 的一个细节

```
取外层的 UUserWidget → 拿它的 FLocalPlayerContext
（拿不到就用空上下文）
SNew(SHitMarkerConfirmationWidget, PlayerContextRef, PerHitMarkerZoneOverrideImages)
    .PerHitMarkerImage(...)
    .AnyHitsMarkerImage(...)
    .HitNotifyDuration(...)
```

> 💡 为什么非要拿 `FLocalPlayerContext`？因为 Slate 那层要**自己去 `ULyraWeaponStateComponent` 上查最近的命中记录**，而那需要知道是哪个玩家。分屏时每个玩家各有自己的命中记录。
>
> ⚠️ 如果外层不是 UUserWidget（比如放在纯 Slate 容器里），`DummyContext` 是空的，命中标记就永远不显示 —— 这是个使用上的限制。

**优先级**：`RebuildWidget`
