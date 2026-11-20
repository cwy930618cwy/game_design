# `AsyncAction_ExperienceReady.h` 速览

> 给**蓝图**用的异步节点："等 Experience 加载完了再做这件事"。对应蓝图中的 `WaitForExperienceReady`。

| 成员 | 干嘛的 |
|---|---|
| `: UBlueprintAsyncActionBase` | 标准 UE 蓝图异步节点基类 |
| `FExperienceReadyAsyncDelegate` | 动态多播委托类型 |
| `WaitForExperienceReady()` | 静态工厂，`BlueprintInternalUseOnly="true"`（由自定义节点包装调用） |
| `Activate()` | 节点被激活时自动调用 |
| `OnReady` | `BlueprintAssignable` —— 蓝图上看到的那根执行针 |
| `Step1_HandleGameStateSet()` | 等 GameState 出现 |
| `Step2_ListenToExperienceLoading()` | 去问 ExperienceManagerComponent 加载好没 |
| `Step3_HandleExperienceLoaded()` | 收到回调 |
| `Step4_BroadcastReady()` | 广播 OnReady 并销毁自己 |
| `WorldPtr` | `TWeakObjectPtr<UWorld>`，不阻止世界被销毁 |

**说明**：四个 Step 是一条直线推进的迷你状态机。
