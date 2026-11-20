# `LyraFrontendStateComponent.cpp` 速览

> 244 行。**这是理解 Lyra 主菜单启动流程的关键文件**。

| 函数 | 干嘛的 |
|---|---|
| `BeginPlay()` | 用 `_HighPriority` 注册 Experience 回调（注释说明：同生命周期所以不用手动解绑） |
| `ShouldShowLoadingScreen()` | ⭐ 显示时**把当前步骤名填进 OutReason**，方便调试加载卡在哪一步 |
| `OnExperienceLoaded()` | ⭐ 用 `FControlFlowStatics::Create` 串起四步并 `ExecuteFlow()` |
| `FlowStep_WaitForUserInitialization()` | ⭐ 见下 |
| `FlowStep_TryShowPressStartScreen()` | ⭐ 见下 |
| `OnUserInitialized()` | 成功/失败**都继续流程**（注释：失败本该去错误界面） |
| `FlowStep_TryJoinRequestedSession()` | 有待加入会话就加入；成功 → `CancelFlow()`（不去主菜单了）；失败 → 继续 |
| `FlowStep_TryShowMainScreen()` | 推入主菜单界面，并把 `bShouldShowLoadingScreen` 置 false |

## Step 1 的两个细节

```
① 判断是不是"硬断线"：GameMode 的 OptionsString 里有 "closed"
   → 是才 ResetUserState()（注释：只在硬断线时重置用户）
② 会话状态无论如何都 CleanUpSessions()
```

## Step 2 的三个分支

```
① 第一个玩家已经登录了 → 直接跳过 PressStart
② 平台不需要 PressStart（ShouldWaitForStartInput 为 false）
   → 直接自动登录，等 OnUserInitialized 回调
③ 需要 → 推入 PressStart 界面，等它 Deactivate 后继续
```

> 💡 **`bSuspendInputUntilComplete = true`** —— 异步推界面期间挂起输入，防止玩家在加载中乱按。

> ⚠️ 两处 TODO：一是引擎的断线流程不够明确（靠 `closed` 选项猜），二是加入会话后没确认 ServerTravel 是否完成。

**优先级**：`OnExperienceLoaded` → `FlowStep_TryShowPressStartScreen`
