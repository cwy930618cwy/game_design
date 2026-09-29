# `LyraDamageExecution.cpp` 速览

> 141 行。**想改伤害公式就改这个文件。**

| 步骤 | 干嘛的 |
|---|---|
| `FDamageStatics` | 用静态单例缓存 `BaseDamage` 的属性捕获定义（避免每次构造） |
| 构造函数 | 登记要捕获 `ULyraCombatSet::BaseDamage`（**从 Source 侧捕获**） |
| `Execute_Implementation()` | ⭐ 整个实现包在 `#if WITH_SERVER_CODE` 里 —— **客户端根本不算伤害** |

## 最终公式

```
DamageDone = Max(
    BaseDamage
  × DistanceAttenuation          ← 距离衰减（来自 ILyraAbilitySourceInterface）
  × PhysicalMaterialAttenuation  ← 材质衰减（打中石头/木头）
  × DamageInteractionAllowedMultiplier   ← 队伍规则（0 或 1）
  , 0)
```

然后 `AddOutputModifier` 加到 `ULyraHealthSet::GetDamageAttribute()` 上（元属性，后面由 HealthSet 转成掉血）。

## 几个细节

| 点 | 说明 |
|---|---|
| 距离怎么算 | 优先用 GE 上下文的 `Origin`；没有就用 `EffectCauser` 的位置；都算不出就 `WORLD_MAX` 并打 Error |
| 没命中结果时 | 退化成"直接打目标的 Avatar"，位置用 Actor 位置 —— 注释说明：**非指向性的 GE 本来就带不上 HitResult** |
| 队伍规则 | 通过 `ULyraTeamSubsystem::CanCauseDamage` 判定，返回 0 就完全没伤害（友军免伤） |
| 距离衰减会钳到 >= 0 | `FMath::Max(DistanceAttenuation, 0.0f)` —— 防止负数 |

> 💡 **这个设计把所有"伤害修正"都变成了乘数**：距离、材质、队伍各出一个系数相乘。想加新的修正维度（比如背刺加成），只要再加一个乘数即可。

**优先级**：`Execute_Implementation`（唯一函数）
