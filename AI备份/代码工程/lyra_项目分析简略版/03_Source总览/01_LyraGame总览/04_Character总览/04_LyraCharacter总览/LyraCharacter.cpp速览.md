# `LyraCharacter.cpp` 速览

> 682 行，全目录最长。**构造函数的参数表 + 网络移动优化**是两大看点。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | ⭐ 见下 |
| `BeginPlay()` / `EndPlay()` | 注册/注销 SignificanceManager（**但注册那行是注释掉的**，见下） |
| `GetLifetimeReplicatedProps()` | `ReplicatedAcceleration` 用 `COND_SimulatedOnly`；`MyTeamID` 正常复制 |
| `PreReplication()` | ⭐ 把加速度从笛卡尔转极坐标，量化成 3 个字节 |
| `OnRep_ReplicatedAcceleration()` | 反向解压回 `FVector` 交给移动组件 |
| `OnAbilitySystemInitialized()` | 初始化 HealthComponent + 同步移动模式 Tag |
| `PossessedBy()` / `UnPossessed()` / `OnRep_Controller()` / `OnRep_PlayerState()` / `SetupPlayerInputComponent()` | ⭐ 每个都顺手调一次 `PawnExtComponent` 的对应通知 —— 这是 InitState 能被推进的原因 |
| `InitializeGameplayTags()` | 先清空所有移动模式 Tag 再设当前的（防止上个 Pawn 残留） |
| `FellOutOfWorld()` | 掉出世界 → `DamageSelfDestruct(true)` |
| `OnDeathStarted()` / `OnDeathFinished()` | 前者禁用移动碰撞；后者下一帧销毁 |
| `UninitAndDestroy()` | 权威端 `DetachFromControllerPendingDestroy` + `SetLifeSpan(0.1)`；然后解绑 ASC |
| `UpdateSharedReplication()` | ⭐ 只在**数据变了**才发，避免重复发包 |
| `FastSharedReplication_Implementation()` | 只在 `ROLE_SimulatedProxy` 处理；**回放播放时直接 return** |
| `FSharedRepMovement` 的 4 个方法 | `FillForCharacter` / `Equals` / `NetSerialize` / 构造（量化级别：保留两位小数） |

## 构造函数里定死的东西

```
把默认移动组件换成 ULyraCharacterMovementComponent
关掉 Tick（注释：尽可能不 Tick 角色）
NetCullDistanceSquared = 900000000   → 剔除距离 30000cm
胶囊体 40×90，碰撞配置 "LyraPawnCapsule" / "LyraPawnMesh"
Mesh 绕 Z 转 -90°（注释：资源是 Y 朝前导出的）
一堆移动参数（加速度 2400、摩擦、转向速率 720、蹲下半高 65...）
★ 创建并连线 3 个组件：
    PawnExtComponent  → 注册 ASC 初始化/卸载回调
    HealthComponent   → 注册 OnDeathStarted / OnDeathFinished
    CameraComponent   → 相对位置 (-300, 0, 75)
```

> ⚠️ **一处官方疏漏**：`BeginPlay` 里那行 `SignificanceManager->RegisterObject(this, ...)` 被注释掉了（留了 `@TODO`），但 `EndPlay` 里照样 `UnregisterObject(this)`。也就是说**当前版本角色实际并没有注册进 SignificanceManager**。

**优先级**：构造函数 → `PreReplication` → `PossessedBy` 系列
