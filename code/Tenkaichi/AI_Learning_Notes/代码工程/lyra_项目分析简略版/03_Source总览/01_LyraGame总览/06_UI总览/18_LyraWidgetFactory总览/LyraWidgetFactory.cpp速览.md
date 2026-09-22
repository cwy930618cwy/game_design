# `LyraWidgetFactory.cpp` 速览

> 14 行，**一个函数，返回空**。

| 函数 | 干嘛的 |
|---|---|
| `FindWidgetClassForData_Implementation()` | `return TSubclassOf<UUserWidget>();` —— **基类的默认实现什么都不匹配** |

**说明**：基类故意不匹配任何东西，实际逻辑全在子类。看 `LyraWidgetFactory_Class`。
