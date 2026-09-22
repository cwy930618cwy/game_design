# `LyraGameState.cpp` 速览

> 145 行，多数函数很短。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 开 Tick；创建两个 subobject（ASC 和 ExperienceManagerComponent）；ASC 设成 `Mixed` 复制模式 |
| `PostInitializeComponents()` | `AbilitySystemComponent->InitAbilityActorInfo(this, this)` —— **Owner 和 Avatar 都指向 GameState 自己** |
| `Tick()` | 有 authority 时把引擎全局变量 `GAverageFPS` 抄到 `ServerFPS` 上复制下去 |
| `AddPlayerState()` | 只调 Super（目前没加东西） |
| `RemovePlayerState()` | 只调 Super；**注释指出它在 `AGameModeBase` 下压根不会被调用** |
| `SeamlessTravelTransitionCheckpoint()` | ⭐ 倒序遍历 PlayerArray，**移除所有机器人和 Inactive 的 PlayerState** |
| `GetLifetimeReplicatedProps()` | `ServerFPS` 正常复制；`RecorderPlayerState` 用 **`COND_ReplayOnly`**（只在回放里同步） |
| `MulticastMessageToClients_Implementation()` | 在客户端把 `FLyraVerbMessage` 转成 `UGameplayMessageSubsystem` 广播 |
| `MulticastReliableMessageToClients_Implementation()` | 直接转调上面那个 |
| `SetRecorderPlayerState()` | ⭐ 只允许设一次，重复调用记 Warning；设完**手动手动调一次** `OnRep` |
| `OnRep_RecorderPlayerState()` | 广播 `OnRecorderPlayerStateChangedEvent` |

> 💡 **为什么 `ServerFPS` 每帧复制？** 性能面板要能在客户端显示服务器帧率，Lyra 的做法是拿一个 float 走属性复制，而不是发 RPC。

**优先级**：`SeamlessTravelTransitionCheckpoint` → 构造函数 → `MulticastMessageToClients_Implementation`
