# `LyraGameplayAbility.cpp` 速览

> 546 行，**Lyra 里所有技能共用的行为都在这**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | ⭐ 定死 4 个策略：`ReplicateNo` / `InstancedPerActor` / `LocalPredicted` / `ClientOrServer`；默认 `OnInputTriggered` + `Independent` |
| `NativeOnAbilityFailedToActivate()` | ⭐ 遍历失败 Tag，查两张表（文字/蒙太奇），广播到消息总线 |
| `CanActivateAbility()` | Super 之后，额外检查**激活组是否被挡**（挡了就打 `Ability_ActivateFail_ActivationGroup`） |
| `SetCanBeCanceled()` | ⚠️ `Exclusive_Replaceable` 的技能**不允许**设成不可取消，会 Error 并拒绝 |
| `OnGiveAbility()` / `OnRemoveAbility()` | 触发蓝图事件；授予时顺带 `TryActivateAbilityOnSpawn` |
| `EndAbility()` | 先清相机模式再 Super |
| `CheckCost()` / `ApplyCost()` | ⭐ 见下 |
| `MakeEffectContext()` | ⭐ 把 `AbilitySource` / `Instigator` / `EffectCauser` / `SourceObject` 写进 Lyra 的 GE 上下文 |
| `ApplyAbilityTagsToGameplayEffectSpec()` | ⭐ 把命中处的**物理材质 Tag** 追加到 TargetTags（打中什么材质，GE 能感知） |
| `DoesAbilitySatisfyTagRequirements()` | 重写版：用 ASC 展开 Tag 关系；**死人被挡时额外打 `Ability_ActivateFail_IsDead`** |
| `TryActivateAbilityOnSpawn()` | 只在 `OnSpawn` 策略 + 未激活 + Avatar 未 tearoff/未将死 时才试 |
| `CanChangeActivationGroup()` / `ChangeActivationGroup()` | 换组要通过 ASC 检查；不可取消的技能不能变成 Replaceable |
| `SetCameraMode()` / `ClearCameraMode()` | 转交 `ULyraHeroComponent` |

## `ApplyCost()` 的一个巧妙之处

```
先判断"这一击有没有命中"（只在权威端、用 ASC->GetAbilityTargetData 查 HitResult）
只对标记了 ShouldOnlyApplyCostOnHit() 的消耗做这个判断
没命中 → 跳过这类消耗
```
> 💡 这意味着**"命中才扣弹药"这类规则不用写代码**，只要在消耗上勾一个开关。

## 顶部的宏

```
ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(...)
```
> 几乎所有对实例操作的函数开头都用它 —— **Lyra 强制要求技能必须是 InstancedPerActor**，非实例化的技能会被断言拦下（注释说 NonInstanced 正因易用性问题被弃用）。

**优先级**：`CheckCost`/`ApplyCost` → `MakeEffectContext` → `CanActivateAbility`
