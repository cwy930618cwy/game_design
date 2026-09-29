# `LyraTabButtonBase.cpp` 速览

> 35 行，三个函数，**每个都带判空**。

| 函数 | 干嘛的 |
|---|---|
| `SetIconFromLazyObject()` | 有 `LazyImage_Icon` 才 `SetBrushFromLazyDisplayAsset` |
| `SetIconBrush()` | 有才 `SetBrush` |
| `SetTabLabelInfo_Implementation()` | `SetButtonText(TabText)` + `SetIconBrush(IconBrush)` |

> 💡 **图标用 `UCommonLazyImage` 而不是普通 Image**，是为了**异步加载** —— Tab 很多时不会一次性把所有图标都加载进内存。
>
> 而且它是 `BindWidgetOptional`，所以纯文字的 Tab 按钮蓝图不需要放图片控件也不会报错。
