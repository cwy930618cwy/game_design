# `LyraGameMode.h` 速览

> 注意它继承自 **`AModularGameModeBase`**（不是 `AGameModeBase`），且方法 override 数量远超普通 GameMode —— 因为它要把"出生点选择""重开""暂停"都转交给别的组件。

| 成员 | 干嘛的 |
|---|---|
| `FOnLyraGameModePlayerInitialized` | 玩家（含机器人）初始化完成的多播委托，seamless travel 后也会触发 |
| `: AModularGameModeBase` | 来自 ModularGameplayActors 插件，支持由组件接管 GameMode 逻辑 |
| `GetPawnDataForController()` | ⭐ 查该 Controller 该用哪份 PawnData |
| `InitGame()` | 延迟一帧去做 Experience 选择 |
| `GetDefaultPawnClassForController_Implementation()` | 由 PawnData 决定生成什么 Pawn |
| `SpawnDefaultPawnAtTransform_Implementation()` | ⭐ 生成 Pawn 时顺手把 PawnData 塞给 `LyraPawnExtensionComponent` |
| `ShouldSpawnAtStartSpot()` | 恒返回 false（交给 SpawningManager） |
| `HandleStartingNewPlayer_Implementation()` | Experience 没加载就**不处理新玩家** |
| `ChoosePlayerStart_Implementation()` / `FinishRestartPlayer()` | 都转发给 `LyraPlayerSpawningManagerComponent` |
| `PlayerCanRestart_Implementation()` / `ControllerCanRestart()` | 后者是给机器人用的通用版 |
| `InitGameState()` | 注册 Experience 加载完成回调 |
| `UpdatePlayerStartSpot()` | 什么都不做（此时队伍还没分配） |
| `GenericPlayerInitialization()` | 广播 `OnGameModePlayerInitialized` |
| `FailedToRestartPlayer()` | 失败后请求下一帧重试 |
| `RequestPlayerRestartNextFrame()` | 蓝图可调的"下一帧重生" |

## protected：决定用哪个 Experience 的一串函数

| 成员 | 干嘛的 |
|---|---|
| `OnExperienceLoaded()` | Experience 就绪后给还没 Pawn 的玩家补一次重生 |
| `IsExperienceLoaded()` | 转发给 ExperienceManagerComponent |
| `OnMatchAssignmentGiven()` | 真正把 ExperienceId 交给 ExperienceManagerComponent |
| `HandleMatchAssignmentIfNotExpectingOne()` | ⭐ 决定"用哪个 Experience"的全部逻辑在这 |
| `TryDedicatedServerLogin()` | 专服场景下的在线登录 |
| `HostDedicatedServerMatch()` | 登录成功后开服 |
| `OnUserInitializedForDedicatedServer()` | 登录回调 |

**优先级**：`HandleMatchAssignmentIfNotExpectingOne` → `SpawnDefaultPawnAtTransform_Implementation`
