# `LyraAbilitySystemComponent.h` 速览

> ⭐ 整个 GAS 在 Lyra 里的**中枢**。它在原生 ASC 上加三样东西：**Tag 驱动的输入**、**激活组**、**Tag 关系表**。

| 成员 | 干嘛的 |
|---|---|
| `: UAbilitySystemComponent` | 标准继承 |
| `TAG_Gameplay_AbilityInputBlocked` | 全局 Tag，有它就**清空所有输入**（暂停、UI 打开时用） |
| `EndPlay()` | 从全局系统里注销自己 |
| `InitAbilityActorInfo()` | ⭐ 换了新 Pawn 时通知所有技能、注册全局系统、接动画实例 |
| `CancelAbilitiesByFunc()` | 按自定义条件批量取消技能 |
| `CancelInputActivatedAbilities()` | 取消所有"输入触发/输入保持"型技能 |
| `AbilityInputTagPressed()` / `AbilityInputTagReleased()` | ⭐ **按 GameplayTag** 收输入（不是按按键） |
| `ProcessAbilityInput()` | ⭐ 每帧把缓存的输入变成技能激活 |
| `ClearAbilityInput()` | 清空输入缓存 |
| `IsActivationGroupBlocked()` / `AddAbilityToActivationGroup()` / `RemoveAbilityFromActivationGroup()` / `CancelActivationGroupAbilities()` | ⭐ 激活组管理 |
| `AddDynamicTagGameplayEffect()` / `RemoveDynamicTagGameplayEffect()` | 用一个 GE 动态增删 Tag |
| `GetAbilityTargetData()` | 按句柄取目标数据 |
| `SetTagRelationshipMapping()` | 设 Tag 关系表（来自 PawnData） |
| `GetAdditionalActivationTagRequirements()` | 补上关系表里的必需/禁止 Tag |
| `TryActivateAbilitiesOnSpawn()` | 激活那些"生成即触发"的技能 |
| 重写的 5 个通知 | `NotifyAbilityActivated` / `NotifyAbilityFailed` / `NotifyAbilityEnded` / `ApplyAbilityBlockAndCancelTags` / `HandleChangeAbilityCanBeCanceled` |
| `ClientNotifyAbilityFailed()` | `Client, Unreliable` RPC，把失败通知到客户端 |
| 三个输入句柄数组 | `InputPressedSpecHandles` / `InputReleasedSpecHandles` / `InputHeldSpecHandles` |
| `ActivationGroupCounts[]` | 每组正在跑几个技能 |

**优先级**：`ProcessAbilityInput` → `NotifyAbilityActivated` → `AddAbilityToActivationGroup`
