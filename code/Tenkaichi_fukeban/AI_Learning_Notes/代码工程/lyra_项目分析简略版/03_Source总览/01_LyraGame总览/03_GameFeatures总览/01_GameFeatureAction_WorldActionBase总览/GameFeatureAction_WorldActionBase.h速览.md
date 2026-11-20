# `GameFeatureAction_WorldActionBase.h` 速览

> 所有"要往某个世界加东西"的 Action 的**基类**。它把"世界什么时候可用"这个麻烦统一处理掉，子类只管实现 `AddToWorld()`。

| 成员 | 干嘛的 |
|---|---|
| `UCLASS(Abstract)` : `UGameFeatureAction` | 抽象类，不直接用 |
| `OnGameFeatureActivating()` | 挂上"世界启动"监听 + 给已存在的世界补一次 |
| `OnGameFeatureDeactivating()` | 摘掉监听 |
| `HandleGameInstanceStart()`（private） | 新的 GameInstance 启动时触发，符合条件就 `AddToWorld` |
| `AddToWorld()`（纯虚） | ⭐ **子类唯一必须实现的** —— 真正的逻辑写在这 |
| `GameInstanceStartHandles` | `TMap<Context, FDelegateHandle>`，记住挂过的委托以便摘除 |

**说明**：子类有 5 个 —— `AddAbilities` / `AddWidgets` / `AddInputBinding` / `AddInputContextMapping` / `SplitscreenConfig`。
