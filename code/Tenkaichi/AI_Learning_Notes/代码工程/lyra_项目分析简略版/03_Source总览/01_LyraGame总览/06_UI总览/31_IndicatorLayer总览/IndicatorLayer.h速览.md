# `IndicatorLayer.h` 速览

> UMG 侧的"标记层"控件，内部包着一个手写的 `SActorCanvas`。

| 成员 | 干嘛的 |
|---|---|
| `: UWidget` | 直接继承 UWidget（不是 UserWidget） |
| `ArrowBrush` | 贴边时显示的箭头笔刷 |
| `RebuildWidget()` / `ReleaseSlateResources()` | 建/销毁 Slate 控件 |
| `MyActorCanvas` | 内部的 `SActorCanvas` |

**说明**：这是"在 UMG 里嵌手写 Slate 控件"的又一个例子，和 `LyraPerfStatGraph` 是同一套路。
