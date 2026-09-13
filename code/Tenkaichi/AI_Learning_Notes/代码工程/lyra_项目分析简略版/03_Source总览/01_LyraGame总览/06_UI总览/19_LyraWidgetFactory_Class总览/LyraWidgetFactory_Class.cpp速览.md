# `LyraWidgetFactory_Class.cpp` 速览

> 23 行，一个函数。

| 函数 | 干嘛的 |
|---|---|
| `FindWidgetClassForData_Implementation()` | ⭐ 见下 |

## 匹配算法

```
从 Data->GetClass() 开始，沿继承链向上逐个查映射表
    找到就返回
走到顶还没找到 → 返回空（调用方会用默认类）
```

> 💡 **"沿继承链向上"是关键**：这意味着你可以只给基类配一个控件，所有子类都能用；也可以给某个子类单独配一个更特殊的控件，**它会优先命中**。这正是"具体规则放前面"能生效的底层原因（配合 ListView 的短路遍历）。
>
> 键用 `TSoftClassPtr` 而不是硬引用，避免因为配了一张表就把所有数据类都加载进来。

**优先级**：`FindWidgetClassForData_Implementation`
