# `LyraListView.h` 速览

> 列表视图。**核心改造：每个条目用哪个控件类，由"工厂规则"动态决定**。

| 成员 | 干嘛的 |
|---|---|
| `: UCommonListView`（`DisableNativeTick`） | 标准继承 |
| `OnGenerateEntryWidgetInternal()` | ⭐ 生成条目时，先用工厂规则挑类 |
| `ValidateCompiledDefaults()`（编辑器） | ⭐ **没配工厂规则就编译报错** |
| `FactoryRules` | `Instanced` 的 `ULyraWidgetFactory` 数组 |

> 💡 **为什么需要这个**：普通 ListView 一整列只能用同一种条目控件。有了工厂规则，同一个列表里可以混排"设置项""标题""分隔线"等不同控件 —— **按数据类型自动选**。设置界面正是这么做的。

**优先级**：`OnGenerateEntryWidgetInternal`
