# `MaterialProgressBar.cpp` 速览

> 263 行。**最有意思的是 `OnWidgetRebuilt` 里"从材质读回默认值"**。

| 函数 | 干嘛的 |
|---|---|
| `SynchronizeProperties()` | 换材质（描边/不描边）→ 清缓存 → 编辑器下应用 `DesignTime_Progress` → 逐个应用"勾了覆盖"的参数 |
| `OnWidgetRebuilt()`（编辑器） | ⭐ 反过来：对**没勾**覆盖的参数，从材质里把默认值读回来填进字段 |
| `OnAnimationFinished_Implementation()` | 是 `BoundAnim_FillBar` 就广播 |
| 五个 `Set*()` | ⭐ **都先比较缓存值，没变就不写材质**（省性能） |
| `AnimateProgressFromStart()` | 设起始/结束 + `PlayAnimation` |
| `AnimateProgressFromCurrent()` | ⭐ 见下 |
| 五个 `Set*_Internal()` | 真正写材质参数 |
| `GetBarDynamicMaterial()` | 懒创建并缓存 MID |

## `AnimateProgressFromCurrent()` 的算法

```
读材质当前值：CurrentStart / CurrentEnd / CurrentFill
NewStart = Lerp(CurrentStart, CurrentEnd, CurrentFill)
→ 以 NewStart 为新起点播放到 End
```

> 💡 这样**连续调用时动画不会跳变** —— 不管当前动画播到一半的哪个位置，都能从"视觉上现在的位置"平滑接上。血条连续掉血时这个细节很关键。

> 💡 材质参数名是硬编码的字符串：`Progress` / `StartProgress` / `FillAmount` / `ColorA` / `ColorB` / `Unfilled Color` / `SegmentEdge` / `Segments` / `FillEdgeSoftness` / `GlowEdge` / `GlowSoftness` / `OutlineScale`。**换材质时必须保证这些名字对得上**。

**优先级**：`AnimateProgressFromCurrent` → `SynchronizeProperties`
