# `LyraCharacter.h` 速览

> 角色基类。**实现了 4 个接口**，身上挂 3 个组件，还负责网络移动优化和死亡流程。

## 两个网络优化用的结构体

| 结构体 | 干嘛的 |
|---|---|
| `FLyraReplicatedAcceleration` | 量化后的加速度：`AccelXYRadians` / `AccelXYMagnitude` / `AccelZ`（各 1 字节） |
| `FSharedRepMovement` | ⭐ 跳帧时补发的移动快照：位置旋转速度 + 时间戳 + 移动模式 + 跳跃/蹲标志 |

> 后者开了 `WithNetSharedSerialization` —— 一次序列化**复用给所有连接**，省 CPU。

## 主要成员

| 成员 | 干嘛的 |
|---|---|
| `: AModularCharacter` + 4 个接口 | `IAbilitySystemInterface` / `IGameplayCueInterface` / `IGameplayTagAssetInterface` / `ILyraTeamAgentInterface` |
| ⭐ 3 个组件 | `PawnExtComponent`、`HealthComponent`、`CameraComponent` |
| `GetAbilitySystemComponent()` | 转发给 `PawnExtComponent`（**ASC 不在这个类上**） |
| `OnAbilitySystemInitialized()` / `Uninitialized()` | 接上/断开 HealthComponent |
| `FastSharedReplication()` | `NetMulticast, unreliable` —— 跳帧补发 |
| `UpdateSharedReplication()` | 判断要不要补发 |
| `OnDeathStarted()` / `OnDeathFinished()` | 死亡开始/结束（由 HealthComponent 驱动） |
| `DisableMovementAndCollision()` / `DestroyDueToDeath()` / `UninitAndDestroy()` | 死亡收尾 |
| `K2_OnDeathFinished()` | `BlueprintImplementableEvent`，给蓝图用 |
| `InitializeGameplayTags()` / `SetMovementModeTag()` | 把移动模式同步成 GameplayTag |
| `ToggleCrouch()` / `OnStartCrouch()` / `OnEndCrouch()` | 蹲下相关（会打 `Status_Crouching` 标签） |
| `CanJumpInternal_Implementation()` | 去掉原版对蹲下的检查 |
| `MyTeamID` + `OnRep_MyTeamID` + `DetermineNewTeamAfterPossessionEnds()` | 队伍归属 |
| `ReplicatedAcceleration` + `OnRep_ReplicatedAcceleration` | 压缩/解压加速度 |

**优先级**：3 个组件 → `OnAbilitySystemInitialized` → `UpdateSharedReplication`
