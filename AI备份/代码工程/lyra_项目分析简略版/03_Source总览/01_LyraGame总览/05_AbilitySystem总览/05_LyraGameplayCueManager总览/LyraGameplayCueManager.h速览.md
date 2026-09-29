# `LyraGameplayCueManager.h` 速览

> GameplayCue（特效/音效）的加载管理。**核心思路：客户端延迟加载，用到才加载**。

| 成员 | 干嘛的 |
|---|---|
| `: UGameplayCueManager` | 标准继承 |
| `Get()` | 从 `UAbilitySystemGlobals` 里取出本管理器 |
| `OnCreated()` | 挂上延迟加载需要的委托 |
| `ShouldAsyncLoadRuntimeObjectLibraries()` / `ShouldSyncLoadMissingGameplayCues()` / `ShouldAsyncLoadMissingGameplayCues()` | 三个加载策略开关 |
| `DumpGameplayCues()` | 控制台命令，打印所有已加载的 Cue |
| `LoadAlwaysLoadedCues()` | ⭐ 加载那些"必须常驻"的 Cue |
| `RefreshGameplayCuePrimaryAsset()` | 重建 Cue 的 PrimaryAsset（数量变了要调） |
| `OnGameplayTagLoaded()` | 有新 Tag 加载时，安排一次异步处理 |
| `HandlePostGarbageCollect()` | GC 期间不能查对象，等 GC 完再处理 |
| `ProcessLoadedTags()` / `ProcessTagToPreload()` / `OnPreloadCueComplete()` / `RegisterPreloadedCue()` | 预加载流水线 |
| `HandlePostLoadMap()` | 换地图时清掉预加载的 Cue |
| `UpdateDelayLoadDelegateListeners()` | 按当前模式挂/摘委托 |
| `ShouldDelayLoadGameplayCues()` | ⭐ 只有客户端才延迟加载 |
| `PreloadedCues` / `AlwaysLoadedCues` | 两类 Cue 集合 |
| `PreloadedCueReferencers` | ⭐ 记录"谁引用了这个 Cue"，没人引用就释放 |
| `LoadedGameplayTagsToProcess` + `...CS` | 待处理队列 + 锁（**会从其它线程写入**） |

**优先级**：`ShouldDelayLoadGameplayCues` → `ProcessTagToPreload` → `RegisterPreloadedCue`
