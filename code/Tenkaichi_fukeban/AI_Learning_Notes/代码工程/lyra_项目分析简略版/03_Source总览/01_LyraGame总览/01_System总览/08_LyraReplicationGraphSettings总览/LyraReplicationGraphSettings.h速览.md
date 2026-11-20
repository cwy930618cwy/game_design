# `LyraReplicationGraphSettings.h` 速览

> RepGraph 的所有可调参数。**每个数值项都绑定了一个控制台变量**，所以运行时也能改。

| 成员 | 默认值 | 干嘛的 |
|---|---|---|
| `: UDeveloperSettingsBackedByCVars` | – | 会出现在项目设置的 **Game** 分类里 |
| `bDisableReplicationGraph` | **true** | ⭐ 总开关，默认 RepGraph 是**关闭**的 |
| `DefaultReplicationGraphClass` | `LyraReplicationGraph` | 用哪个 Graph 类（可换成自己的） |
| `bEnableFastSharedPath` | true | 角色移动的快速共享路径 |
| `TargetKBytesSecFastSharedPath` | 10 | FastShared 用的带宽预算 |
| `FastSharedPathCullDistPct` | 0.80 | 超过此距离比例就剔除 FastShared |
| `DestructionInfoMaxDist` | 30000 cm | 超过这个距离不复制销毁信息 |
| `SpatialGridCellSize` | 10000 cm | 网格单元大小 |
| `SpatialBiasX` / `SpatialBiasY` | -200000 | 网格"最小坐标"，出界会自动重置 |
| `bDisableSpatialRebuilds` | true | 禁止空间重建 |
| `DynamicActorFrequencyBuckets` | 3 | 动态 Actor 分几个桶，桶越多有效频率越低 |
| `ClassSettings` | 空数组 | ⭐ **逐个类**指定走哪个 `EClassRepNodeMapping` |

**说明**：真正 tuning 复制行为时，改的是这个类的字段（或在控制台输对应 CVar），而不是改代码。
