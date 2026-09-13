# `LyraCharacterMovementComponent.cpp` 速览

> 132 行，六个短函数 + 一个 CVar。

| 函数 | 干嘛的 |
|---|---|
| CVar `LyraCharacter.GroundTraceDistance` | 向下探测的距离，默认 100000（`ECVF_Cheat`） |
| `SimulateMovement()` | ⭐ 如果加速度是网络传来的，**先存起来，跑完 Super 再还原** —— 防止模拟端把它清掉 |
| `CanAttemptJump()` | 注释写明：和原版一样**只是去掉了蹲下的检查**；加了 `IsFalling()` 以支持二段跳 |
| `GetGroundInfo()` | ⭐ 见下 |
| `SetReplicatedAcceleration()` | 置 `bHasReplicatedAcceleration = true` 并赋值 |
| `GetDeltaRotation()` / `GetMaxSpeed()` | ASC 上有 `TAG_Gameplay_MovementStopped` 就返回 0 |

## `GetGroundInfo()` 的缓存策略

```
if (没 Owner || 本帧已算过)  return 缓存;      ← 用 GFrameCounter 判断
if (MOVE_Walking)  直接用 CurrentFloor，距离 0
else               从角色位置向下打一条射线
     MOVE_NavWalking → 距离 0
     命中            → 距离 = Hit.Distance - 半高
最后记录 LastUpdateFrame = GFrameCounter
```

> 💡 **这是个很实用的优化模式**：地面信息可能一帧内被多个系统查询（相机、动画、脚步声特效），用帧号做缓存保证**一帧只做一次射线检测**。

> 💡 **`MovementStopped` 这个 Tag 的设计值得借鉴**：想让角色定住（比如被击晕、播技能中），**不需要改移动组件**，只要往 ASC 上挂一个 GameplayTag 就行。

**优先级**：`GetGroundInfo` → `SimulateMovement`
