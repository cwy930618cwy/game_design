# `LyraPerfStatContainerBase.cpp` 速览

> 70 行，四个函数。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 空 |
| `NativeConstruct()` | 先刷新一次，再订阅 `ULyraSettingsLocal::OnPerfStatDisplayStateChanged` |
| `NativeDestruct()` | 退订 |
| `UpdateVisibilityOfChildren()` | ⭐ 见下 |

## 显隐判定（两层过滤）

```
① 容器的过滤：StatDisplayModeFilter
     TextOnly      → bShowTextWidgets = true
     GraphOnly     → bShowGraphWidgets = true
     TextAndGraph  → 两个都 true

② 用户的设置：SettingsLocal->GetPerfStatDisplayState(这个 stat)
     Hidden        → 不显示
     TextOnly      → bShowTextWidgets 说了算
     GraphOnly     → bShowGraphWidgets 说了算
     TextAndGraph  → 任一为真就显示
```

然后 `WidgetTree->ForEachWidget` 遍历，对 `ULyraPerfStatWidgetBase` 设 `HitTestInvisible` 或 `Collapsed`。

> 💡 **两层过滤的意思**：容器说"我这栏只放图表"，用户说"FPS 我要文字+图表" → 结果只显示图表。反过来容器说"都要"，用户说"只要文字" → 显示文字。

**优先级**：`UpdateVisibilityOfChildren`
