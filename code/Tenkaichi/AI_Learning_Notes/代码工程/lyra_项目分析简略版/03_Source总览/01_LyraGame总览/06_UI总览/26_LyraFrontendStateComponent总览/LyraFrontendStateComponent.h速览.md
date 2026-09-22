# `LyraFrontendStateComponent.h` 速览

> ⭐ **主菜单（前端）的流程控制器**。用一个**控制流（ControlFlow）**把"等登录 → 按 Start → 加入会话 → 显示主菜单"串起来。

| 成员 | 干嘛的 |
|---|---|
| `: UGameStateComponent` + `ILoadingProcessInterface` | 挂 GameState 上，且能控制加载界面 |
| `BeginPlay()` | 用 **HighPriority** 档注册 Experience 加载完成回调 |
| `ShouldShowLoadingScreen()` | ⭐ 流程没走完就继续显示加载界面 |
| `OnExperienceLoaded()` | ⭐ 建并执行那条控制流 |
| 四个 `FlowStep_*` | 见 cpp |
| `OnUserInitialized()` | 登录完成回调 |
| `PressStartScreenClass` / `MainScreenClass` | 两个界面的软引用 |
| `FrontEndFlow` | 控制流对象 |
| `InProgressPressStartScreen` | 正在等待的 PressStart 步骤 |
| `OnJoinSessionCompleteEventHandle` | 加入会话的回调句柄 |
| `bShouldShowLoadingScreen` | 是否还在显示加载界面 |

## 四步流程

```
1. Wait For User Initialization   等用户登录初始化
2. Try Show Press Start Screen     "按 Start" 界面（主机必需）
3. Try Join Requested Session      如果有待加入的会话就加入
4. Try Show Main Screen            显示主菜单
```

> 💡 **用 ControlFlow 而不是自己写状态机**，是 Lyra 的一个明显风格 —— 把"一串异步步骤"表达成队列，每步拿一个 `SubFlow`，完成就 `ContinueFlow()`，要中断就 `CancelFlow()`。

**优先级**：`OnExperienceLoaded` → 四个 `FlowStep_*`
