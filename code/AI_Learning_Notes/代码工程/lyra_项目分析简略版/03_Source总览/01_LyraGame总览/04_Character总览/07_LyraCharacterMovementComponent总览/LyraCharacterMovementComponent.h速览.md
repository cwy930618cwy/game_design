# `LyraCharacterMovementComponent.h` 速览

> 在原生移动组件上加三样东西：**地面信息缓存**、**复制来的加速度**、**用 GameplayTag 冻结移动**。

| 成员 | 干嘛的 |
|---|---|
| `TAG_Gameplay_MovementStopped` | 全局 Tag `Gameplay.MovementStopped`，有它就动不了 |
| `FLyraCharacterGroundInfo` | 地面信息：`LastUpdateFrame` / `GroundHitResult` / `GroundDistance` |
| `: UCharacterMovementComponent`（`Config = Game`） | 标准继承 |
| `SimulateMovement()` | 保护"复制来的加速度"不被覆盖 |
| `CanAttemptJump()` | 放宽跳跃条件 |
| `GetGroundInfo()` | ⭐ 带缓存的地面查询 |
| `SetReplicatedAcceleration()` | 接收网络传来的加速度 |
| `GetDeltaRotation()` / `GetMaxSpeed()` | ⭐ 有 `MovementStopped` 标签时都返回 0 |
| `CachedGroundInfo` | 缓存（注释警告：别直接访问它） |
| `bHasReplicatedAcceleration` | 这一帧的加速度是不是网络传来的（`Transient`） |

**优先级**：`GetGroundInfo` → `GetMaxSpeed` / `GetDeltaRotation`
