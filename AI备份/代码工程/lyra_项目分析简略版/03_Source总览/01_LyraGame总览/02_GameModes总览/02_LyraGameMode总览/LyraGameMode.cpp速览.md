# `LyraGameMode.cpp` 速览

> 523 行。**最大的价值在于"用哪个 Experience"的优先级链，和 Pawn 生成的那几步**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 一次性把 GameState / Session / PlayerController / PlayerState / Pawn / HUD 全换成 Lyra 版 |
| `GetPawnDataForController()` | ⭐ 三级回退：PlayerState 上的 → Experience 的 `DefaultPawnData` → AssetManager 的兜底值 |
| `InitGame()` | `SetTimerForNextTick` 调 `HandleMatchAssignmentIfNotExpectingOne`（**注释：等启动设置有时间初始化**） |
| `HandleMatchAssignmentIfNotExpectingOne()` | ⭐ 按优先级挑 Experience，见下表 |
| `TryDedicatedServerLogin()` | 只在**专服 + 默认地图**时生效，走 `UCommonUserSubsystem::TryToLoginForOnlinePlay(0)` |
| `HostDedicatedServerMatch()` | 按命令行 `UserExperience=` / `Playlist=` 或默认值找 UserFacingExperience → `CreateHostingRequest` → `HostSession` |
| `OnMatchAssignmentGiven()` | 调 `ExperienceComponent->SetCurrentExperience(ExperienceId)` |
| `OnExperienceLoaded()` | 给已存在但没有 Pawn 的 PC 调 `RestartPlayer` |
| `SpawnDefaultPawnAtTransform_Implementation()` | ⭐ `bDeferConstruction` 延迟构造 → 生成后立刻 `PawnExtComp->SetPawnData(PawnData)` → `FinishSpawning` |
| `ChoosePlayerStart` / `FinishRestartPlayer` / `ControllerCanRestart` | 全都转发给 `LyraPlayerSpawningManagerComponent` |
| `ShouldSpawnAtStartSpot()` | 返回 false，注释：出生点一律交给 SpawningManager |
| `HandleStartingNewPlayer_Implementation()` | **Experience 没加载就不让玩家进来** |
| `FailedToRestartPlayer()` | 有 PawnClass 且还能重开，就下一帧再试一次 |

## Experience 的选择优先级（高 → 低）

```
Matchmaking 分配
URL Options  (?Experience=xxx)
Developer Settings（仅 PIE）
命令行 (-Experience=xxx)
World Settings 上的 DefaultGameplayExperience
专服流程
兜底：B_LyraDefaultExperience
```

> 💡 **这条链解释了"为什么我改了地图上的 Experience 却没生效"** —— 上面几档任意一个命中，World Settings 就被跳过了。PIE 里尤其容易撞上 **Developer Settings 的 ExperienceOverride**。

> 💡 `SpawnDefaultPawnAtTransform` 里 **先生成、再 SetPawnData、最后 FinishSpawning** 是有意为之：PawnData 必须在角色 BeginPlay 之前就位，这正好接上 `LyraPawnExtensionComponent` 的 InitState 流程。

**优先级**：`HandleMatchAssignmentIfNotExpectingOne` → `SpawnDefaultPawnAtTransform_Implementation` → `GetPawnDataForController`
