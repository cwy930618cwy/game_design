# `LyraHUD.cpp` 速览

> 75 行，五个函数。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 关掉 Tick（`bStartWithTickEnabled = false`） |
| `PreInitializeComponents()` | `AddGameFrameworkComponentReceiver(this)` —— 让 HUD 能接收来自 GameFeature 的组件扩展 |
| `BeginPlay()` | ⭐ `SendGameFrameworkComponentExtensionEvent(this, NAME_GameActorReady)`，然后 Super |
| `EndPlay()` | `RemoveGameFrameworkComponentReceiver(this)` |
| `GetDebugActorList()` | 遍历所有 ASC，把它的 Avatar（或退而求其次 Owner）加进调试列表，跳过 CDO/Archetype |

> 💡 **两处配合使用**：`PreInitializeComponents` 注册接收者 → `BeginPlay` 广播"我准备好了" → `GameFeatureAction_AddWidgets` 收到后开始往 HUD 上加界面。这条链断掉的话，HUD 上会什么都没有。

**优先级**：`BeginPlay` → `GetDebugActorList`
