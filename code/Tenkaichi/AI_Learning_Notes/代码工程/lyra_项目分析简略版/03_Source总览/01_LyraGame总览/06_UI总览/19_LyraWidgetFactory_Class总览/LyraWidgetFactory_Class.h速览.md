# `LyraWidgetFactory_Class.h` 速览

> 最常用的工厂实现：**按数据的类型决定用哪个控件类**。

| 成员 | 干嘛的 |
|---|---|
| `: ULyraWidgetFactory` | 继承工厂基类 |
| `FindWidgetClassForData_Implementation()` | 见 cpp |
| `EntryWidgetForClass` | ⭐ `TMap<TSoftClassPtr<UObject>, TSubclassOf<UUserWidget>>` —— 数据类 → 控件类的映射表 |

**优先级**：`FindWidgetClassForData_Implementation`
