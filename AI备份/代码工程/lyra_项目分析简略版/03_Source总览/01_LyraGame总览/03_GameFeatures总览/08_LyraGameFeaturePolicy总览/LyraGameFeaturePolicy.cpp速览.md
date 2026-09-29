# `LyraGameFeaturePolicy.cpp` 速览

> 160 行。**前半是策略，后半是两个观察者的实现**。

| 函数 | 干嘛的 |
|---|---|
| `Get()` | `UGameFeaturesSubsystem::Get().GetPolicy<ULyraGameFeaturePolicy>()` |
| `InitGameFeatureManager()` | 建 `ULyraGameFeature_HotfixManager` 和 `ULyraGameFeature_AddGameplayCuePaths`，`AddObserver` 后调 Super |
| `ShutdownGameFeatureManager()` | 先 Super，再逐个 RemoveObserver 并清空 |
| `GetGameFeatureLoadingMode()` | ⭐ `bLoadClientData = !IsRunningDedicatedServer()`；`bLoadServerData = !IsRunningClientOnly()` |
| `GetPreloadAssetListForGameFeature` / `IsPluginAllowed` / `GetPreloadBundleStateForGameFeature` | **都只调 Super**（Lyra 没改，是留的扩展点） |

## 观察者 1：热更新

```
OnGameFeatureLoading → ULyraHotfixManager::RequestPatchAssetsFromIniFiles()
```
> 💡 意思是：**每次有 GameFeature 插件加载，就去拉一次热更新补丁**。这是 Lyra 把热更新和插件系统绑在一起的地方。

## 观察者 2：GameplayCue 路径

```
OnGameFeatureRegistering
  → 遍历插件的所有 Action，找出 AddGameplayCuePath
  → FixPluginPackagePath() 修正路径（把 /Game/... 换成 /插件名/...）
  → GCM->AddGameplayCueNotifyPath(...)
  → InitializeRuntimeObjectLibrary() 重建
  → Cue 数量变了就 RefreshGameplayCuePrimaryAsset()

OnGameFeatureUnregistering → 反向 RemoveGameplayCueNotifyPath
```

> 💡 **这补上了 `GameFeatureAction_AddGameplayCuePath.cpp` 里"缺失"的部分** —— 那个 Action 只有数据没有逻辑，真正干活的是这里。
>
> 而且注意时机：它发生在 **Registering**（比 Activating 更早），所以 Cue 在游戏真正开始前就已就位。

**优先级**：`GetGameFeatureLoadingMode` → `InitGameFeatureManager` → 观察者 2
