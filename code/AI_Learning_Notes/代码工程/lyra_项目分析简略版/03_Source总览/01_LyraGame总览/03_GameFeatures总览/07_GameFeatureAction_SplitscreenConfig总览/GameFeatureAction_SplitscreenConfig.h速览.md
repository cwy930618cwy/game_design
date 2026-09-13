# `GameFeatureAction_SplitscreenConfig.h` 速览

> 控制分屏开关。菜单里显示为 "Splitscreen Config"。**用投票制**解决"多个插件都有意见"的问题。

| 成员 | 干嘛的 |
|---|---|
| `: UGameFeatureAction_WorldActionBase`（`final`） | 走世界感知流程 |
| `OnGameFeatureDeactivating()` | 撤票，最后一个撤走的恢复分屏 |
| `AddToWorld()` | 投票禁用分屏 |
| `bDisableSplitscreen` | 默认 **true**（Lyra 默认是不想分屏的） |
| `LocalDisableVotes` | 自己投过票的 Viewport 列表 |
| `GlobalDisableVotes` | ⭐ **`static`**：全进程共享，插件 A 和插件 B 的票记在一起 |

**说明**：`static` 是关键 —— 它让**不同 GameFeature 里的多个实例**能对同一个 Viewport 累计投票。
