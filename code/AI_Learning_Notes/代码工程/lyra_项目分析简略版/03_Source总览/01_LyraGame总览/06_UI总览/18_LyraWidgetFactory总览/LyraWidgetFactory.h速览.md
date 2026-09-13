# `LyraWidgetFactory.h` 速览

> 控件工厂的**基类**（只有 `.h` 有意义，`.cpp` 就一行）。给 `LyraListView` 用。

| 成员 | 干嘛的 |
|---|---|
| `UCLASS(Abstract, Blueprintable, EditInlineNew)` | ⭐ `EditInlineNew` 让它能**直接内联编辑在 ListView 的规则数组里** |
| `FindWidgetClassForData()` | `BlueprintNativeEvent` —— 给一个数据对象，返回该用哪个控件类 |

**说明**：只有 22 行，看子类 `LyraWidgetFactory_Class` 才知道怎么实现。
