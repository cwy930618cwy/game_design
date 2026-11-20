# `LyraExperienceManagerComponent.h` 速览

> **整个 Lyra 的灵魂**。挂在一个 GameStateComponent 上，负责把一份 Experience 数据变成真正跑起来的游戏。

## `ELyraExperienceLoadState`：加载走到哪一步了

```
Unloaded → Loading → LoadingGameFeatures → LoadingChaosTestingDelay → ExecutingActions → Loaded
                                                                                          ↓
                                                                                     Deactivating
```

## 主要成员

| 成员 | 干嘛的 |
|---|---|
| `: UGameStateComponent` + `ILoadingProcessInterface` | 挂在 GameState 上，且能告诉引擎"我还在加载" |
| `FOnLyraExperienceLoaded` | 加载完成的多播委托（带当前 Experience 参数） |
| `SetCurrentExperience()` | ⭐ **入口**：指定要加载哪份 Experience（只在服务端调用） |
| `CallOrRegister_OnExperienceLoaded_HighPriority()` | 已加载就立即回调，否则排队（**最高优先级**那档） |
| `CallOrRegister_OnExperienceLoaded()` | 同上，普通档 |
| `CallOrRegister_OnExperienceLoaded_LowPriority()` | 同上，最低档（如生成机器人） |
| `GetCurrentExperienceChecked()` | 取当前 Experience，**没加载完会 assert** |
| `IsExperienceLoaded()` | 是否已完整加载 |
| `EndPlay()` | 收尾：反向执行 Deactivating |
| `ShouldShowLoadingScreen()` | 加载期间持续返回 true → **这就是 Lyra 加载界面不消失的原因** |

## 私有成员

| 成员 | 干嘛的 |
|---|---|
| `OnRep_CurrentExperience()` | ⭐ 客户端靠这个复制回调**触发自己的加载流程** |
| `StartExperienceLoad()` | 第一步：加载资源 bundle |
| `OnExperienceLoadComplete()` | 第二步：找齐 GameFeature 插件 URL |
| `OnGameFeaturePluginLoadComplete()` | 计数 -1，归零就进下一步 |
| `OnExperienceFullLoadCompleted()` | 第三步：按顺序执行所有 Action |
| `OnActionDeactivationCompleted()` / `OnAllActionsDeactivated()` | 卸载时的计数与收尾 |
| `CurrentExperience` | `ReplicatedUsing=OnRep_CurrentExperience` |
| `LoadState` | 当前处于上面哪个状态 |
| `NumGameFeaturePluginsLoading` / `GameFeaturePluginURLs` | 插件加载计数与列表 |
| `NumObservedPausers` / `NumExpectedPausers` | 卸载阶段等待异步完成的计数 |
| 三个 `OnExperienceLoaded_*` 委托成员 | 对应上面三档回调队列 |

> 💡 **为什么要有三档优先级？** 因为"系统层要先就位（如组件注册），玩法层才能开工，机器人最后再生成"。这是 Lyra 用来控制初始化顺序的手段。

**优先级**：`SetCurrentExperience` → `StartExperienceLoad` → `OnExperienceFullLoadCompleted`
