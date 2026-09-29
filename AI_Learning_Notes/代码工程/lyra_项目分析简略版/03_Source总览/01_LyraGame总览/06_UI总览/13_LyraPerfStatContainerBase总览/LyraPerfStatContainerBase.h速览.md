# `LyraPerfStatContainerBase.h` 速览

> 性能面板的**容器**：管里面一排 `ULyraPerfStatWidgetBase` 的显隐。

| 成员 | 干嘛的 |
|---|---|
| `: UCommonUserWidget`（`Abstract`） | 标准控件 |
| `NativeConstruct()` / `NativeDestruct()` | 订阅/退订设置变化 |
| `UpdateVisibilityOfChildren()` | ⭐ 遍历子控件决定显隐 |
| `StatDisplayModeFilter` | 这个容器只显示"文字"还是"图表"（默认 `TextAndGraph`） |

**说明**：它自己不知道有哪些 stat，靠遍历 WidgetTree 找出所有 `ULyraPerfStatWidgetBase`。

**优先级**：`UpdateVisibilityOfChildren`
