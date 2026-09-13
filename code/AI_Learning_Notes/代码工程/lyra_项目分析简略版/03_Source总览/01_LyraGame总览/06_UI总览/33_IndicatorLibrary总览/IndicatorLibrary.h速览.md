# `IndicatorLibrary.h` 速览

> 一个蓝图函数库，**只有一个静态函数**。

| 成员 | 干嘛的 |
|---|---|
| `: UBlueprintFunctionLibrary`（`MinimalAPI`） | 标准函数库 |
| `GetIndicatorManagerComponent()` | `BlueprintCallable`，从 Controller 上取标记管理器 |

**说明**：纯粹是为了让蓝图能方便地拿到 Manager —— 因为 `ULyraIndicatorManagerComponent::GetComponent` 是静态 C++ 函数，蓝图调不到。
