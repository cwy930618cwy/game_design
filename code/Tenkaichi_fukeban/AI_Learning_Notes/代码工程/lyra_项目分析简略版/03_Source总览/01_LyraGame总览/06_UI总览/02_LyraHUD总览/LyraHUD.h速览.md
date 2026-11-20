# `LyraHUD.h` 速览

> HUD 的 Actor。**类注释直接告诉你：基本不用改它**。

> "you typically do not need to extend or modify this class, instead you would use an 'Add Widget' action in your experience to add a HUD layout and widgets to it. **This class exists primarily for debug rendering**"

| 成员 | 干嘛的 |
|---|---|
| `: AHUD`（`Config = Game`） | 标准 HUD 继承 |
| `PreInitializeComponents()` | 注册成 GameFramework 组件接收者 |
| `BeginPlay()` / `EndPlay()` | 发送/移除 `NAME_GameActorReady` 事件 |
| `GetDebugActorList()` | ⭐ 把所有带 ASC 的 Actor 加进调试列表 |

> 💡 **`BeginPlay` 里那行 `SendGameFrameworkComponentExtensionEvent(NAME_GameActorReady)` 很关键** —— 这正是 `GameFeatureAction_AddWidgets` 等待的那个事件。HUD 一就绪，界面才会被加进来。

**优先级**：`GetDebugActorList` → `BeginPlay`
