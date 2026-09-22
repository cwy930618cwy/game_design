# `LyraIndicatorManagerComponent.h` 速览

> 屏幕标记系统的**数据端**：挂在 Controller 上，维护"当前有哪些标记"。

| 成员 | 干嘛的 |
|---|---|
| `: UControllerComponent`（`BlueprintType, Blueprintable`） | ⭐ 挂在 **Controller** 上（每个玩家自己的标记列表） |
| `GetComponent()` | 静态：从 Controller 上取本组件 |
| `AddIndicator()` / `RemoveIndicator()` | 增删标记（`BlueprintCallable`） |
| `OnIndicatorAdded` / `OnIndicatorRemoved` | ⭐ 两个事件，`SActorCanvas` 就是监听它们来同步 UI 的 |
| `GetIndicators()` | 取全部标记 |
| `Indicators` | `TArray<UIndicatorDescriptor*>` |

> 💡 **这个组件和 `SActorCanvas` 是数据/视图分离的一对**：这边只管"有哪些标记"，那边负责"怎么画到屏幕上"，通过那两个事件通信。

**优先级**：`AddIndicator` / `RemoveIndicator`
