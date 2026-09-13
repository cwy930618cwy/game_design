# `LyraReplicationGraph.h` 速览

> 123 行，**一个主类 + 两个自定义节点类**。作用：接管 Actor 的网络相关性判断（不用 `IsNetRelevantFor`）。

| 成员 | 干嘛的 |
|---|---|
| `LogLyraRepGraph` | 这个类专用的日志分类 |
| `: UReplicationGraph`（`config=Engine`） | 基类 + 从 `DefaultEngine.ini` 读配置 |
| `ResetGameWorldState()` | 清空并重置所有节点的缓存 |
| `InitGlobalActorClassSettings()` | ⭐ 配置"每种类该走哪个复制节点"（最核心） |
| `InitGlobalGraphNodes()` | 创建全局节点（网格 / 永远相关 / PlayerState） |
| `InitConnectionGraphNodes()` | 给每个连接创建专属节点 |
| `RouteAddNetworkActorToNodes()` | 按策略把一个 Actor 塞进对应节点 |
| `RouteRemoveNetworkActorToNodes()` | 反向：从节点移除 |
| `GridNode` | 空间网格节点（按距离相关） |
| `AlwaysRelevantNode` | 对所有连接永远相关的 Actor 列表 |
| `AlwaysRelevantStreamingLevelActors` | **按关卡名分组**的永远相关列表 |
| `PrintRepNodePolicies()` | 打印每个类的路由策略 |
| `IsSpatialized()` | 内联：`Mapping >= Spatialize_Static` 就是空间化的 |
| `ClassRepNodePolicies` | 类 → 路由策略的映射表 |
| `ExplicitlySetClasses` | 代码里明确设过信息的类（避免被默认逻辑覆盖） |

## 两个自定义节点类

| 类 | 干嘛的 |
|---|---|
| `ULyraReplicationGraphNode_AlwaysRelevant_ForConnection` | 每连接的"永远相关"列表，还负责**按客户端关卡可见性**补发 Actor |
| `ULyraReplicationGraphNode_PlayerStateFrequencyLimiter` | 限流节点：每帧只滚动复制 2 个 PlayerState |

**优先级**：`InitGlobalActorClassSettings` → `RouteAddNetworkActorToNodes` → `InitGlobalGraphNodes`
