# `GameFeatureAction_AddWidget.h` 速览

> 往 HUD 上加界面。菜单里显示为 **"Add Widgets"**（注意文件名是单数 Widget，类名是复数 Widgets）。

## 两个配置结构体

| 结构体 | 字段 | 干嘛的 |
|---|---|---|
| `FLyraHUDLayoutRequest` | `LayoutClass`、`LayerID` | 整块布局，推到 `UI.Layer` 分类下的某个层 |
| `FLyraHUDElementEntry` | `WidgetClass`、`SlotID` | 单个小部件，塞进某个插槽 |

> 两者都用 **GameplayTag** 定位（`LayerID` / `SlotID`），所以 HUD 上加东西**不需要改任何 C++**。

## 主要成员

| 成员 | 干嘛的 |
|---|---|
| `: UGameFeatureAction_WorldActionBase`（`final`） | 走世界感知流程 |
| `Layout` | `TArray<FLyraHUDLayoutRequest>` |
| `Widgets` | `TArray<FLyraHUDElementEntry>` |
| `AddAdditionalAssetBundleData()` | 把 WidgetClass 记进 **Client** bundle（服务器不加载） |
| `IsDataValid()`（编辑器） | 校验类和 Tag 是否都填了 |
| `AddToWorld()` | 对 `ALyraHUD::StaticClass()` 注册扩展回调 |
| `HandleActorExtension()` | `ExtensionAdded` 或 **`NAME_GameActorReady`** → 加；移除事件 → 撤 |
| `AddWidgets()` / `RemoveWidgets()` | 真正加/撤 |
| `FPerActorData` | 记录这个 HUD 上加了哪些 Layout 和 ExtensionHandle |
| `ContextData` | 按 Context 分开记账 |

**优先级**：`AddWidgets` → `HandleActorExtension`
