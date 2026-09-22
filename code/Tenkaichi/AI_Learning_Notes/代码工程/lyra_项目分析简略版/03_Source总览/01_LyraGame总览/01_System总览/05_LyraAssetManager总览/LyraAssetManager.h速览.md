# `LyraAssetManager.h` 速览

> 资源加载总控。**133 行里有 52 行是两个模板函数**，其余是「加载入口 + 启动任务队列」。

| 成员 | 干嘛的 |
|---|---|
| `struct FLyraBundles` | 定义了 bundle 名 `Equipped`（装备用，client/server 分开加载就靠它） |
| `: UAssetManager` | 注释指明要在 `DefaultEngine.ini` 里把 `AssetManagerClassName` 设成它 |
| `Get()` | 静态单例访问入口 |
| `GetAsset<T>()` | 模板：把软引用同步加载成对象，可选常驻内存 |
| `GetSubclass<T>()` | 同上，走 `TSoftClassPtr` |
| `DumpLoadedAssets()` | 打印当前被它加载的所有资源 |
| `GetGameData()` / `GetDefaultPawnData()` | 取全局数据 / 兜底 PawnData |
| `GetOrLoadTypedGameData<T>()` | 内联模板：先查缓存 `GameDataMap`，没有就**阻塞式**加载 |
| `SynchronousLoadAsset()` | 底层同步加载实现 |
| `AddLoadedAsset()` | 把资源记进常驻列表（带锁） |
| `StartInitialLoading()` | 引擎启动时调用，**排队 + 执行所有 StartupJob** |
| `PreBeginPIE()` | 仅编辑器：进 PIE 前预加载 GameData |
| `LoadGameDataOfClass()` | 实际加载某个 GameData 并写回 `GameDataMap` |
| `LyraGameDataPath` / `DefaultPawnData` | `UPROPERTY(Config)`：路径在 ini 里配 |
| `StartupJobs` | `TArray<FLyraAssetManagerStartupJob>` 启动任务队列 |
| `LoadedAssets` + `LoadedAssetsCritical` | 常驻资源集合 + 线程锁 |

**优先级**：`StartInitialLoading` → `DoAllStartupJobs` → `GetAsset`
