# `GameFeatureAction_AddWidget.cpp` 速览

> 194 行。加界面分**两条路**：Layout 走 CommonUI 的层，Widgets 走 UIExtension 插槽。

| 函数 | 干嘛的 |
|---|---|
| `OnGameFeatureDeactivating()` | 先 Super 再 `Reset` |
| `AddAdditionalAssetBundleData()`（编辑器） | 把每个 `WidgetClass` 加进 `LoadStateClient` bundle |
| `IsDataValid()`（编辑器） | Layout：类为空 / LayerID 无效；Widgets：类为空 / SlotID 无效 |
| `AddToWorld()` | 对 `ALyraHUD` 注册扩展回调 |
| `Reset()` | 清空请求 + 逐个 `Handle.Unregister()` |
| `HandleActorExtension()` | 移除事件 → `RemoveWidgets`；`ExtensionAdded` / `NAME_GameActorReady` → `AddWidgets` |
| `AddWidgets()` | ⭐ 核心，见下 |
| `RemoveWidgets()` | Layout 逐个 `DeactivateWidget()`；句柄逐个 `Unregister()` |

## `AddWidgets()` 的两条路

```
① Layout   → UCommonUIExtensions::PushContentToLayer_ForPlayer(LocalPlayer, LayerID, 布局类)
② Widgets  → UUIExtensionSubsystem::RegisterExtensionAsWidgetForContext(SlotID, LocalPlayer, 控件类, -1)
```

> 💡 **两个提前返回的条件**，排查"HUD 没出现"时先看这两个：
> - `HUD->GetOwningPlayerController()` 为空 → 直接 return
> - 拿不到 `ULocalPlayer`（不是本地玩家控制）→ 不加

> ⚠️ `RemoveWidgets` 里有一句防御性检查：注释提到**客户端上可能同时存在多个 HUD Actor**，所以只对当初登记过的那个做反注册。

**优先级**：`AddWidgets`
