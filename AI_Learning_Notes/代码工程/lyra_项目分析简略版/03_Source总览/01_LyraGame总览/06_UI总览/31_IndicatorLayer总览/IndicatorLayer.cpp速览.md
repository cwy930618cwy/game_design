# `IndicatorLayer.cpp` 速览

> 45 行，三个函数。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | `bIsVariable = true`；设为 `HitTestInvisible`（**标记层不能挡住鼠标**） |
| `ReleaseSlateResources()` | `MyActorCanvas.Reset()` |
| `RebuildWidget()` | ⭐ 见下 |

## `RebuildWidget()` 的两个注意点

```
1. 跳过设计器；没有 LocalPlayer 就 Error
2. SNew(SActorCanvas, FLocalPlayerContext(LocalPlayer), &ArrowBrush)
3. 失败时返回 SNew(SBox) —— ★ 注释说明：不能返回 NullWidget，不安全
```

> 💡 **`FLocalPlayerContext`** 这个参数很关键 —— `SActorCanvas` 需要知道是哪个玩家的视口，才能拿到投影数据。多人分屏时每个玩家的这一层各自投影自己的画面。
