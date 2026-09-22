# `IActorIndicatorWidget.h` 速览

> 标记控件要实现的**接口**（只有 `.h`，没有 `.cpp`）。

| 成员 | 干嘛的 |
|---|---|
| `UIndicatorWidgetInterface` / `IIndicatorWidgetInterface` | UE 接口的标准写法（U 开头是壳，I 开头是真接口） |
| `BindIndicator()` | `BlueprintNativeEvent` —— 绑定到一份 `UIndicatorDescriptor` |
| `UnbindIndicator()` | 反向解绑 |

> 💡 **`SActorCanvas` 会在创建/回收控件时自动调用这两个函数**（见 `AddIndicatorForEntry` / `RemoveIndicatorForEntry`，用 `ImplementsInterface` 判断）。
>
> 所以你只要让标记控件实现这个接口，就能在 `BindIndicator` 里拿到 descriptor，从中读数据对象并更新显示 —— **控件本身不需要知道投影和位置逻辑**。

**说明**：这是典型的"数据与显示分离"：descriptor 管位置和规则，控件只管长什么样。
