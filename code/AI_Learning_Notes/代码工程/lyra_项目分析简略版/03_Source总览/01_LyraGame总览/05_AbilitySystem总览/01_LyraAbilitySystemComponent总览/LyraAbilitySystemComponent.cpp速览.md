# `LyraAbilitySystemComponent.cpp` 速览

> 529 行。**输入处理**和**激活组**是两大核心。

| 函数 | 干嘛的 |
|---|---|
| `EndPlay()` | 从 `ULyraGlobalAbilitySystem` 注销 |
| `InitAbilityActorInfo()` | ⭐ 见下 |
| `TryActivateAbilitiesOnSpawn()` | 遍历所有 Spec，调 `TryActivateAbilityOnSpawn` |
| `CancelAbilitiesByFunc()` | 遍历活跃技能，逐个问谓词；**`CanBeCanceled()` 为假就记 Error 跳过** |
| `CancelInputActivatedAbilities()` | 用上面的函数取消 `OnInputTriggered` / `WhileInputActive` 两类 |
| `AbilitySpecInputPressed/Released()` | 注释说明：**不支持 `bReplicateInputDirectly`**，改用 `InvokeReplicatedEvent` 让 `WaitInputPress` 任务能工作 |
| `AbilityInputTagPressed/Released()` | ⭐ 遍历 Spec，用 `GetDynamicSpecSourceTags().HasTagExact(InputTag)` 匹配 |
| `ProcessAbilityInput()` | ⭐ 见下 |
| `NotifyAbilityActivated()` / `NotifyAbilityEnded()` | 加减激活组计数 |
| `NotifyAbilityFailed()` | 非本地控制且技能支持网络 → 发 RPC 给客户端；否则本地处理 |
| `ApplyAbilityBlockAndCancelTags()` | ⭐ 先用 Tag 关系表**扩充** Block/Cancel 列表，再交给 Super |
| `GetAdditionalActivationTagRequirements()` | 转发给 Tag 关系表 |
| `AddDynamicTagGameplayEffect()` | 从 `LyraGameData::Get().DynamicTagGameplayEffect` 取 GE，把 Tag 塞进 `DynamicGrantedTags` |
| `IsActivationGroupBlocked()` | ⭐ 见下 |
| `AddAbilityToActivationGroup()` | 计数 +1；若是 Exclusive 组，先取消掉组里"可替换"的；`ensure(ExclusiveCount <= 1)` |

## `InitAbilityActorInfo()` 换 Pawn 时做的 4 件事

```
1. 通知所有技能实例：OnPawnAvatarSet()        （回放里实例可能缺失，代码做了判空）
2. 注册进 ULyraGlobalAbilitySystem            （注释：等到真有 Pawn 才注册，因为全局 GE 可能需要 Avatar）
3. 接上 ULyraAnimInstance::InitializeWithAbilitySystem
4. TryActivateAbilitiesOnSpawn()
```

## `ProcessAbilityInput()` 的顺序（很讲究）

```
0. 有 AbilityInputBlocked 标签 → 清空输入并 return
1. 先处理"按住"的（WhileInputActive）
2. 再处理"本帧按下"的（OnInputTriggered）
   —— 已经激活的就转发输入事件，没激活的加入待激活列表
3. 一次性统一 TryActivateAbility（注释：避免"按住"激活后又收到一次按下事件）
4. 最后处理"本帧松开"的
5. 清空按下/松开缓存（注意：InputHeld 不清空）
```

## 激活组规则

| 组 | 能否激活 | 激活时做什么 |
|---|---|---|
| `Independent` | 永远可以 | 不影响别人 |
| `Exclusive_Replaceable` | 只要没有 `Exclusive_Blocking` 在跑 | 取消掉同组的其它可替换技能 |
| `Exclusive_Blocking` | 同上 | 同上 |

> 💡 **一句话理解**：`Independent` 是被动技能，`Exclusive_Blocking` 是"大招"（会挡住别人），`Exclusive_Replaceable` 是"可被下一个打断的技能"。同时最多只能有 1 个 Exclusive 技能在跑。

**优先级**：`ProcessAbilityInput` → `InitAbilityActorInfo` → `AddAbilityToActivationGroup`
