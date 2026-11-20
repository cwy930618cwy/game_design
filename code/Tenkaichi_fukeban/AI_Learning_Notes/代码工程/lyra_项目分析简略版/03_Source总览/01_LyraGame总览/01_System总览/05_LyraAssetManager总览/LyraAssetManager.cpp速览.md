# `LyraAssetManager.cpp` 速览

> 274 行。核心是**一串「启动工位」怎么排队执行**。

| 函数 / 块 | 干嘛的 |
|---|---|
| `STARTUP_JOB()` 宏 | 往队列里塞一个任务，`_WEIGHTED` 版可带权重 |
| `Get()` | 从 `GEngine->AssetManager` 取，类型不对直接 **Fatal**（提示 ini 没配） |
| `SynchronousLoadAsset()` | AssetManager 就绪走 StreamableManager，没就绪兜底用 `LoadObject` |
| `ShouldLogAssetLoads()` | 看命令行有没有 `-LogAssetLoads` |
| `StartInitialLoading()` | 只排两个任务：`InitializeGameplayCueManager`（权重 1）、`GetGameData`（权重 **25**） |
| `InitializeGameplayCueManager()` | 调用 `LyraGameplayCueManager::LoadAlwaysLoadedCues()` |
| `DoAllStartupJobs()` | ⭐ 服务器直接跑完；客户端按**权重累加**算百分比喂给 `UpdateInitialGameContentLoadPercent` |
| `UpdateInitialGameContentLoadPercent()` | **空实现**，注释说可以接到启动加载界面 |
| `LoadGameDataOfClass()` | 加载失败直接 **Fatal**（注释：这不可恢复） |
| `PreBeginPIE()` | PIE 前先把 GameData 加载好 |

## 启动流水线

```
StartInitialLoading()
   ├─ Super::StartInitialLoading()   先做资源扫描（必须）
   ├─ [job 1] InitializeGameplayCueManager
   ├─ [job 25] GetGameData
   └─ DoAllStartupJobs()             按权重算总进度
```

> 💡 **这是 Lyra 里最容易"地图卡住不进"的排查点**：GameData 加载失败是 Fatal，而 `GetGameData` 权重占 25/26，所以卡在 96% 通常是它。

**优先级**：`DoAllStartupJobs` → `Get` → `LoadGameDataOfClass`
