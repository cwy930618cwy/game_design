# `LyraListView.cpp` 速览

> 55 行，三个函数。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 空 |
| `ValidateCompiledDefaults()`（编辑器） | `FactoryRules.Num() == 0` → **Error**："has no Factory Rules defined, can't create widgets without them." |
| `OnGenerateEntryWidgetInternal()` | ⭐ 见下 |

## 挑控件类的流程

```
WidgetClass = DesiredEntryClass（蓝图上配的默认值）
遍历 FactoryRules：
    第一个能返回非空类的规则 → 用它，break
GenerateTypedEntry(WidgetClass, OwnerTable)
```

> 💡 **规则是"短路"的** —— 找到第一个匹配的就用，所以**规则顺序有意义**，具体的放前面、通用的放后面。

**优先级**：`OnGenerateEntryWidgetInternal`
