# `GameFeatureAction_SplitscreenConfig.cpp` 速览

> 79 行。逻辑就是一张**引用计数表**。

| 函数 | 干嘛的 |
|---|---|
| `AddToWorld()` | `bDisableSplitscreen` 为真时：拿到 ViewportClient → 记入 `LocalDisableVotes` → `GlobalDisableVotes[Key]++` → **只有从 0 变 1 时**才真正 `SetForceDisableSplitscreen(true)` |
| `OnGameFeatureDeactivating()` | 倒序遍历自己的票：检查 WorldContext 属不属于本 Context（不属于就跳过；**已销毁的对象算属于**）→ 计数 <=1 就移除并恢复分屏，否则 -1 |

## 投票机制

```
插件 A 说"禁用分屏"  → 计数 1 → SetForceDisableSplitscreen(true)
插件 B 说"禁用分屏"  → 计数 2 → 什么都不做（已经禁了）
插件 A 卸载          → 计数 1 → 保持禁用
插件 B 卸载          → 计数 0 → SetForceDisableSplitscreen(false)
```

> 💡 为什么需要这个？因为「要不要分屏」可能有好几个 GameFeature 同时有意见。用计数保证**最后一个撤走的才真正恢复**，避免一个插件提前把别人要的禁用给解了。

**优先级**：`AddToWorld`
