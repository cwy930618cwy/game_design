# `LyraGameplayCueManager.cpp` 速览

> 407 行。**前 40 行定义了三种加载模式，先读这段**。

## 三种加载模式（`ELyraEditorLoadMode`）

| 模式 | 行为 |
|---|---|
| `LoadUpfront`（**当前默认**） | 一次性全加载。编辑器里加载慢，但 PIE 快、特效绝不会漏播 |
| `PreloadAsCuesAreReferenced_GameOnly` | 编辑器里不预加载（迭代快，但 PIE 可能出现"为什么没特效"） |
| `PreloadAsCuesAreReferenced` | 一律延迟加载 |

## 主要函数

| 函数 | 干嘛的 |
|---|---|
| CVar `Lyra.DumpGameplayCues` | 打印常驻/预加载/按需加载的三类 Cue 及总数 |
| `ShouldDelayLoadGameplayCues()` | `!IsRunningDedicatedServer()` —— **专服不延迟**（它根本不放特效） |
| `LoadAlwaysLoadedCues()` | 目前 `AdditionalAlwaysLoadedCueTags` 是空数组，留了 `@TODO` |
| `OnGameplayTagLoaded()` | ⭐ 加锁入队，派一个**回到游戏线程**的任务（`FGameplayCueTagThreadSynchronizeGraphTask`） |
| `HandlePostGarbageCollect()` | GC 期间不能调 `StaticFindObject`，所以延后处理 |
| `ProcessTagToPreload()` | 已加载就直接登记；否则 `RequestAsyncLoad` |
| `RegisterPreloadedCue()` | ⭐ 没有 Owner 的算"常驻"；有 Owner 的记进 `PreloadedCues` + 引用者集合 |
| `HandlePostLoadMap()` | 换地图时清掉预加载，并剔除已失效的引用者 |
| `RefreshGameplayCuePrimaryAsset()` | 把所有 Cue 路径塞进 `Client` bundle，注册成动态资产 |

> 💡 **最值得学的一点**：`OnGameplayTagLoaded` 可能来自**任意线程**（资源加载线程），所以它用锁把 Tag 入队，再派任务回游戏线程处理；又发现 GC 期间不能查对象，于是再延后到 GC 之后。这是很典型的"异步 + 线程安全 + 规避 GC"处理套路。

> ⚠️ 变量名还留着 `UFortAssetManager_*` 的前缀（Fortnite 遗留），说明这段代码是从堡垒之夜搬过来的。

**优先级**：`OnGameplayTagLoaded` → `ProcessTagToPreload` → `RegisterPreloadedCue`
