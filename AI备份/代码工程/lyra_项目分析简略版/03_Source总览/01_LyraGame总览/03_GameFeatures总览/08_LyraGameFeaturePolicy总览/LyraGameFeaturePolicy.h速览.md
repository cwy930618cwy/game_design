# `LyraGameFeaturePolicy.h` 速览

> 决定 GameFeature 插件**怎么加载**的策略类。一个头文件里放了 **3 个类**。

| 类 | 干嘛的 |
|---|---|
| `ULyraGameFeaturePolicy` | 主策略，`: UDefaultGameFeaturesProjectPolicies` |
| `ULyraGameFeature_HotfixManager` | 观察者：插件加载时触发热更新 |
| `ULyraGameFeature_AddGameplayCuePaths` | 观察者：插件注册/注销时增删 GameplayCue 目录 |

## `ULyraGameFeaturePolicy` 成员

| 成员 | 干嘛的 |
|---|---|
| `UCLASS(Config = Game)` | 能从 `DefaultGame.ini` 读配置 |
| `Get()` | 从 `UGameFeaturesSubsystem` 里取出本策略 |
| `InitGameFeatureManager()` | ⭐ 建两个观察者并注册进子系统 |
| `ShutdownGameFeatureManager()` | 反注册并清空 |
| `GetPreloadAssetListForGameFeature()` | 要预加载哪些资产（当前只调 Super） |
| `IsPluginAllowed()` | 这个插件允不允许加载（当前只调 Super） |
| `GetPreloadBundleStateForGameFeature()` | 预加载哪些 bundle（当前只调 Super） |
| `GetGameFeatureLoadingMode()` | ⭐ 决定加载 Client 还是 Server 数据 |
| `Observers` | 两个观察者对象，`Transient` |

## 两个观察者

| 观察者 | 监听的阶段 |
|---|---|
| `ULyraGameFeature_HotfixManager` | `OnGameFeatureLoading` |
| `ULyraGameFeature_AddGameplayCuePaths` | `OnGameFeatureRegistering` / `OnGameFeatureUnregistering` |

**优先级**：`InitGameFeatureManager` → `GetGameFeatureLoadingMode`
