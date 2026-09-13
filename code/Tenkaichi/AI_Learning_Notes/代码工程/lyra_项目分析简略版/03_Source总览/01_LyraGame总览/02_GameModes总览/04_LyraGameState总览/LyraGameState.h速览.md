# `LyraGameState.h` 速览

> GameState 在 Lyra 里承担了三件事：**挂 ExperienceManager**、**拥有一个全局 ASC**、**广播消息给所有客户端**。

| 成员 | 干嘛的 |
|---|---|
| `: AModularGameStateBase` + `IAbilitySystemInterface` | 组件化 + 自己就是 ASC 宿主 |
| `PreInitializeComponents` / `PostInitializeComponents` / `EndPlay` / `Tick` | 生命周期；**开了 Tick 且默认启用** |
| `AddPlayerState` / `RemovePlayerState` | 玩家加入/离开 |
| `SeamlessTravelTransitionCheckpoint()` | 过场时清理机器人和非活跃 PlayerState |
| `GetAbilitySystemComponent()` | 实现 `IAbilitySystemInterface` |
| `GetLyraAbilitySystemComponent()` | 内联，拿强类型的那个 ASC |
| `MulticastMessageToClients()` | `NetMulticast, Unreliable` —— 丢得起的通知（击杀播报等） |
| `MulticastReliableMessageToClients()` | `NetMulticast, Reliable` —— 丢不起的通知 |
| `GetServerFPS()` | 拿服务器帧率 |
| `SetRecorderPlayerState()` / `GetRecorderPlayerState()` | 回放：记录"这场录像是谁录的" |
| `OnRecorderPlayerStateChangedEvent` | 回放视角切换时的广播 |

## 私有 / 受保护成员

| 成员 | 干嘛的 |
|---|---|
| `ExperienceManagerComponent` | ⭐ **Experience 加载的总控就挂在这儿** |
| `AbilitySystemComponent` | 全局 ASC，注释说明主要用于 **GameplayCue** |
| `ServerFPS` | `Replicated`，每帧从服务端同步下来 |
| `RecorderPlayerState` | `ReplicatedUsing=OnRep`，`Transient` |

**优先级**：`ExperienceManagerComponent` → 两个 `Multicast*` 消息函数
