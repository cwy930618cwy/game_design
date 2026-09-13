# `LyraTabButtonBase.h` 速览

> Tab 按钮。**注意它继承了两个东西**：按钮基类 + Tab 按钮接口。

| 成员 | 干嘛的 |
|---|---|
| `: ULyraButtonBase` + `ILyraTabButtonInterface` | 既是 Lyra 按钮，又能接收 Tab 描述 |
| `SetIconFromLazyObject()` | 用软引用异步设置图标 |
| `SetIconBrush()` | 直接设置笔刷 |
| `SetTabLabelInfo_Implementation()` | ⭐ 由 TabList 回调，设置文字 + 图标 |
| `LazyImage_Icon` | `BindWidgetOptional` 的懒加载图片（**可选**，没有就不显示图标） |

**优先级**：`SetTabLabelInfo_Implementation`
