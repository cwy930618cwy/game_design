# `LyraReplicationGraphTypes.h` 速览

> **只有 `.h`，没有 `.cpp`**。定义 RepGraph 的"枚举 + 配置结构"，被 `LyraReplicationGraph.h` 和 `...Settings.h` 共用。

## `EClassRepNodeMapping`：一个类该路由到哪

| 枚举值 | 含义 |
|---|---|
| `NotRouted` | 不进任何节点（有特殊节点单独处理，如 PlayerState 限流器） |
| `RelevantAllConnections` | 对所有连接都相关的节点 |
| `Spatialize_Static` | 进网格，不动、不用每帧更新 |
| `Spatialize_Dynamic` | 进网格，经常动、每帧更新 |
| `Spatialize_Dormancy` | 进网格，休眠时当静态，唤醒后当动态 |

> 💡 注释里划了一条线：**下面三个才算"空间化"** —— 代码里用 `Mapping >= Spatialize_Static` 判断，所以枚举顺序不能乱调。

## `FRepGraphActorClassSettings`：单个类的配置项

| 成员 | 干嘛的 |
|---|---|
| `ActorClass` | 目标类（`FSoftClassPath`，蓝图也可以） |
| `bAddClassRepInfoToMap` | 是否登记进类 → 策略映射表 |
| `ClassNodeMapping` | 用哪个路由策略 |
| `bAddToRPC_Multicast_OpenChannelForClassMap` | 是否要为多播 RPC 开通道 |
| `bRPC_Multicast_OpenChannelForClass` | 开还是不开 |
| `GetStaticActorClass()` | 把软路径解析成 UClass（C++ 类走 FindObject，蓝图走 StaticLoadObject） |

**说明**：这个结构让「让某个类不再被距离剔除」变成**在设置里配一行**，不用改 `LyraReplicationGraph.cpp`。
