# `IndicatorDescriptor.h` 速览

> 一个标记的**全部配置**。**这个头文件有 240 行，绝大部分是 Getter/Setter**。

| 分组 | 成员 |
|---|---|
| 绑定什么 | `DataObject`（任意数据）、`Component`（场景组件）、`ComponentSocketName`（插槽）、`IndicatorWidgetClass`（用哪个控件） |
| 自动移除 | `bAutoRemoveWhenIndicatorComponentIsNull` + `CanAutomaticallyRemove()` |
| 可见性 | `bVisible` + `GetIsVisible()`（**组件无效时也算不可见**） |
| 投影方式 | `EActorCanvasProjectionMode`：`ComponentPoint` / `ComponentBoundingBox` / `ComponentScreenBoundingBox` / `ActorBoundingBox` / `ActorScreenBoundingBox` |
| 对齐 | `HAlignment` / `VAlignment` |
| 贴边 | `bClampToScreen`（超出屏幕就贴边）+ `bShowClampToScreenArrow`（显示指示箭头） |
| 偏移 | `WorldPositionOffset`（世界空间）/ `ScreenSpaceOffset`（屏幕空间）/ `BoundingBoxAnchor`（包围盒锚点 0~1） |
| 排序 | `Priority`（在按深度排序之后再用它排） |
| 生命周期 | `SetIndicatorManagerComponent()`（**只能设一次**）、`UnregisterIndicator()` |
| 内部 | `IndicatorWidget` / `Content` / `CanvasHost`（`SActorCanvas` 是 friend） |

## 5 种投影模式

| 模式 | 用哪个点 |
|---|---|
| `ComponentPoint` | 组件位置（或插槽位置） |
| `ComponentBoundingBox` | 组件包围盒，再按 `BoundingBoxAnchor` 取点 |
| `ComponentScreenBoundingBox` | 投影组件包围盒在屏幕上的矩形 |
| `ActorBoundingBox` | 整个 Actor 的包围盒 |
| `ActorScreenBoundingBox` | 整个 Actor 投影后的屏幕矩形 |

> 💡 类注释建议：**你的标记控件最好实现 `IActorIndicatorWidget` 接口**，这样它能"绑定"到这份数据上。

**优先级**：投影模式 → `CanAutomaticallyRemove`
