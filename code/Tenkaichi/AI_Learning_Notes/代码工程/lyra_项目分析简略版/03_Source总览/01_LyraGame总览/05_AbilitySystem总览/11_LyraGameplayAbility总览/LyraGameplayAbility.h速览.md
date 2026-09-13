# `LyraGameplayAbility.h` 速览

> ⭐ **所有 Lyra 技能的基类**（`UCLASS(Abstract)`）。它给原生 `UGameplayAbility` 加了：激活策略、激活组、额外消耗、失败反馈、相机接管。

## 两个核心枚举

| 枚举 | 取值 | 含义 |
|---|---|---|
| `ELyraAbilityActivationPolicy` | `OnInputTriggered` / `WhileInputActive` / `OnSpawn` | 技能**什么时候**激活 |
| `ELyraAbilityActivationGroup` | `Independent` / `Exclusive_Replaceable` / `Exclusive_Blocking` | 技能**与其它技能**的关系 |

## 主要成员

| 成员 | 干嘛的 |
|---|---|
| `GetLyraAbilitySystemComponentFromActorInfo()` 等 5 个 Getter | 便捷取 ASC / PC / Controller / Character / HeroComponent |
| `TryActivateAbilityOnSpawn()` | 策略是 `OnSpawn` 时尝试立即激活 |
| `CanChangeActivationGroup()` / `ChangeActivationGroup()` | 运行时改激活组 |
| `SetCameraMode()` / `ClearCameraMode()` | ⭐ 技能临时接管相机 |
| `OnAbilityFailedToActivate()` | 失败时调 Native + 蓝图两个回调 |
| `NativeOnAbilityFailedToActivate()` / `ScriptOnAbilityFailedToActivate()` | 失败反馈的两个实现点 |
| 重写的 10 个 GAS 虚函数 | 含 `CheckCost` / `ApplyCost` / `MakeEffectContext` / `DoesAbilitySatisfyTagRequirements` 等 |
| `OnPawnAvatarSet()` | 换了 Pawn 时通知 |
| `GetAbilitySource()` | 追溯伤害来源 |
| `K2_OnAbilityAdded` / `K2_OnAbilityRemoved` / `K2_OnPawnAvatarSet` | 三个 `BlueprintImplementableEvent` |
| `ActivationPolicy` / `ActivationGroup` | 上面两个枚举的配置字段 |
| `AdditionalCosts` | ⭐ `Instanced` 的消耗列表（弹药、法力等） |
| `FailureTagToUserFacingMessages` | 失败 Tag → 给玩家看的提示文字 |
| `FailureTagToAnimMontage` | 失败 Tag → 播放的蒙太奇（如"没弹药"咔哒动作） |
| `FLyraAbilityMontageFailureMessage` | 失败消息结构体 |

**优先级**：`CanActivateAbility` → `ApplyCost` → `MakeEffectContext`
