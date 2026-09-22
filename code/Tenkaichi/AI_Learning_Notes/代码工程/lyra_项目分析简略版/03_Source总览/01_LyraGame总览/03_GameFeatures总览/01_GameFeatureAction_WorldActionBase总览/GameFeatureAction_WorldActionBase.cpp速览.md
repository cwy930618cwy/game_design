# `GameFeatureAction_WorldActionBase.cpp` 速览

> 45 行，三个函数。**理解它就能理解所有 5 个子类为什么能自动生效**。

| 函数 | 干嘛的 |
|---|---|
| `OnGameFeatureActivating()` | ① 往 `FWorldDelegates::OnStartGameInstance` 挂 `HandleGameInstanceStart` ② 遍历**已存在**的世界，`Context.ShouldApplyToWorldContext()` 为真就当场 `AddToWorld` |
| `OnGameFeatureDeactivating()` | 从 `OnStartGameInstance` 摘掉委托并清表 |
| `HandleGameInstanceStart()` | 新 GameInstance 起来时，同样判断上下文后调 `AddToWorld` |

## 为什么要这么绕

```
情况 A：Action 激活时，世界已经在了  → 当场 AddToWorld
情况 B：Action 激活时，世界还没建   → 挂回调，等 GameInstance 一启动再补上
```

> 💡 这一层屏蔽掉了 "PIE 里多个世界"、"Action 先于世界激活" 这些坑，子类只写 `AddToWorld` 就够了。

> ⚠️ 注意：基类**没有**提供反向清理（没有 `RemoveFromWorld`），卸载逻辑由各子类在自己的 `OnGameFeatureDeactivating` 里做。

**优先级**：`OnGameFeatureActivating`
