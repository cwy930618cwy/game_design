# `LyraExperienceManagerComponent.cpp` 速览

> 468 行。开头 **7 条 `@TODO`** 直接写明了哪些地方是半成品，值得先扫一眼。

## 加载全过程（四个阶段）

| 函数 | 干了什么 |
|---|---|
| `SetCurrentExperience()` | 用 AssetManager 把 ExperienceId 解析成CDO，**两个 check** 后赋值并启动加载 |
| `StartExperienceLoad()` | ⭐ 收集 experience + 所有 ActionSet 的资产 ID；按 NetMode 决定要不要 `Client` / `Server` bundle；调 `ChangeBundleStateForPrimaryAssets` 异步加载；完成回调到下一步 |
| `OnExperienceLoadComplete()` | ⭐ 从 Experience 和 ActionSet 里收集所有 GameFeature 插件名 → 转成 URL；然后 `LoadAndActivateGameFeaturePlugin` |
| `OnGameFeaturePluginLoadComplete()` | 计数递减，归零进下一步 |
| `OnExperienceFullLoadCompleted()` | ⭐ 混沌测试延迟 → 逐个调用每个 Action 的 `Registering` / `Loading` / `Activating` → 置 Loaded → **按高/中/低三档广播委托** → 通知 `LyraSettingsLocal` |
| `EndPlay()` | 反过来对插件和 Action 做 Deactivate / Unregistering |
| `ShouldShowLoadingScreen()` | `LoadState != Loaded` 就返回 true，理由串是 `"Experience still loading"` |

## 关键细节

| 点 | 说明 |
|---|---|
| `bLoadClient` / `bLoadServer` | 按 `GetOwner()->GetNetMode()` 判断，**专服不加载 Client bundle** —— 这就是 `FLyraBundles::Equipped` 之外那两个 bundle 的用途 |
| 合并 Handle | Bundle 与 Raw 两个异步加载用一个 `CreateCombinedHandle` 合起来，避免重复回调 |
| 两个混沌 CVar | `lyra.chaos.ExperienceDelayLoad.MinSecs` / `.RandomSecs`，人为延迟加载完成，**专门用来测试加载界面的表现** |
| `ActivateListOfActions` | 对 Experience 自己的 Actions 和每个 ActionSet 的 Actions **都跑一遍** |
| 加载完才会做的事 | `ULyraSettingsLocal::Get()->OnExperienceLoaded()`（应用画质设置） |

## 顶部 7 条 TODO 透露的信息

```
体验定义本身是同步加载的（TODO 想改异步）
失败处理靠 check()，没有"加载失败"状态
Action 的各个阶段现在是"一次性全做"，没按时机分开
GameFeature 卸载目前"泄漏"（保持着启用状态）
```

**优先级**：`StartExperienceLoad` → `OnExperienceLoadComplete` → `OnExperienceFullLoadCompleted`
