# `AsyncAction_ExperienceReady.cpp` 速览

> 96 行，四步状态机逐一看。

| 函数 | 干嘛的 |
|---|---|
| `WaitForExperienceReady()` | 建出 Action 对象，存弱引用 World，`RegisterWithGameInstance(World)` |
| `Activate()` | GameState 已存在 → 直接 Step2；否则挂到 `World->GameStateSetEvent` 等 Step1；**世界都没了就 `SetReadyToDestroy()`** |
| `Step1_HandleGameStateSet()` | 摘掉回调，进 Step2 |
| `Step2_ListenToExperienceLoading()` | 问 ExperienceManagerComponent：已加载 → 进 Step4；否则注册 `CallOrRegister_OnExperienceLoaded` 等 Step3 |
| `Step3_HandleExperienceLoaded()` | 直接进 Step4 |
| `Step4_BroadcastReady()` | 广播 `OnReady` 然后 `SetReadyToDestroy()` |

> 💡 **一个刻意的设计**：即使 Experience **已经**加载好了，Step2 仍然**故意延迟一帧**才广播。源码注释写得很清楚 —— 防止有人写出"依赖下一帧就一定成立"的代码，把竞争条件提前暴露出来。

**优先级**：`Step2_ListenToExperienceLoading`
