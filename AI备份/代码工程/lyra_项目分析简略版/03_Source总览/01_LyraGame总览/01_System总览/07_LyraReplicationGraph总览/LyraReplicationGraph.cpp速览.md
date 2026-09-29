# `LyraReplicationGraph.cpp` 速览

> 990 行，**前 69 行是官方写的设计说明注释**，非常值得先读。

| 函数 / 块 | 干嘛的 |
|---|---|
| 顶部大注释 | 讲解 4 类节点的分工 + **怎么调试**（不看这里基本看不懂代码） |
| `namespace Lyra::RepGraph` 的 10 个 CVar | `Lyra.RepGraph.CellSize`、`SpatialBiasX/Y`、`DynamicActorFrequencyBuckets`、`EnableFastSharedPath` 等 |
| `ConditionalCreateReplicationDriver()` | 挂到 `CreateReplicationDriverDelegate`；**settings 里开关关着就返回 nullptr** |
| `ULyraReplicationGraph()` | 构造函数里绑定上面那个委托 |
| `GetClassNodeMapping()` | 自动推断策略：CDO 上 `bAlwaysRelevant` / `bOnlyRelevantToOwner` / `bNetUseOwnerRelevancy` 三个标志决定走哪条路 |
| `InitGlobalActorClassSettings()` | ⭐ 设惰性初始化回调 → 应用 settings 里的 `ClassSettings` → 遍历所有可复制类登记策略 → 给 `ACharacter` 配 FastShared 与量化 → 配 RPC 多播开通道策略 |
| `InitGlobalGraphNodes()` | 建 GridNode（10m 格）+ AlwaysRelevantNode + PlayerState 限流节点 |
| `InitConnectionGraphNodes()` | 给连接建专属节点，并挂上"客户端关卡可见性"回调 |
| `RouteAdd/RemoveNetworkActorToNodes()` | 按 `EClassRepNodeMapping` 分派到对应节点 |
| `AlwaysRelevant_ForConnection::GatherActorListsForConnection()` | 每帧重建列表：Viewer + ViewTarget + Pawn + PlayerState（**15 行；PS 做 50% 节流**） |
| `PlayerStateFrequencyLimiter::PrepareForReplication()` | 每帧遍历所有 PlayerState 装桶，每桶 `TargetActorsPerFrame`(2) 个 |
| `PrintRepNodePolicies()` + 2 个控制台命令 | `Lyra.RepGraph.PrintRouting`、`Lyra.RepGraph.FrequencyBuckets` |

## 官方注释里给的调试命令

```
Net.RepGraph.PrintGraph           打印整张图和每个节点
Net.RepGraph.PrintGraph class     按类分组
Net.RepGraph.PrintAll <Frames> <Conn>   打印某连接 N 帧的收集过程
Lyra.RepGraph.PrintRouting        打印每个类的路由策略
```

> 💡 **调试复制问题第一步**：把 `LyraReplicationGraphSettings` 里的 `bDisableReplicationGraph`（默认 **true**，即默认关闭）切成 false，先确认问题是不是 RepGraph 引入的。

**优先级**：`InitGlobalActorClassSettings` → `GetClassNodeMapping` → `RouteAddNetworkActorToNodes`
