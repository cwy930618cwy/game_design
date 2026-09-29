# 14 — 技能"什么时候放、会不会互殴"咋实现的：每个概念都贴源码

> **定位**：给技能贴俩标签——`ActivationPolicy`（啥时候放）+ `ActivationGroup`（会不会跟别人互殴）。
>
> 这篇**一个概念接一段源码**，讲清"这个选项是干嘛的"紧接着"代码里是怎么实现的"，不搞概念和代码分家。

---

## 〇、先说这俩标签是谁的、住哪

它俩是 `ULyraGameplayAbility` 这个技能基类上**两个枚举字段**。源码（`LyraGameplayAbility.h` 第 193~199 行）：

```cpp
	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Ability Activation")
	ELyraAbilityActivationPolicy ActivationPolicy;   // 啥时候放

	// Defines the relationship between this ability activating and other abilities activating.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Ability Activation")
	ELyraAbilityActivationGroup ActivationGroup;     // 会不会跟别人互殴
```

- 每个技能在蓝图/资产里**选一下这两项**，就贴好了标签。
- 注意 `EditDefaultsOnly`——**只能改默认值**，也就是做技能时定好，运行中不变。
- 重要认知：**这俩字段只存"选项"，不写逻辑**。逻辑在下面的各函数里，靠读这个字段来决定怎么做。

---

## 一、标签 1：`ActivationPolicy` —— 技能啥时候放

### 概念（先 3 个选项秒懂）

| 值 | 大白话 | 游戏里 |
|---|---|---|
| `OnInputTriggered` | **按一下就放一次** | 闪现/单发火球：点一下，放完拉倒 |
| `WhileInputActive` | **按住才一直放** | 蓄力/连射：按住持续，松手就停 |
| `OnSpawn` | **不用按，出生自动放** | 被动光环/出生被动 |

源码（`LyraGameplayAbility.h` 第 38~49 行）：

```cpp
UENUM(BlueprintType)
enum class ELyraAbilityActivationPolicy : uint8
{
	OnInputTriggered,    // Try to activate the ability when the input is triggered.
	WhileInputActive,    // Continually try to activate the ability while the input is active.
	OnSpawn              // Try to activate the ability when an avatar is assigned.
};
```

### 它是怎么被"用"的？（核心：`ProcessAbilityInput` 每帧查它）

玩家每帧的处理在 `ULyraAbilitySystemComponent::ProcessAbilityInput`。它维护三个"输入手柄缓存"，每帧**翻技能、读 `ActivationPolicy`、决定要不要激活**。

源码（`LyraAbilitySystemComponent.cpp` 第 216~311 行，关键段）：

```cpp
void ULyraAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (HasMatchingGameplayTag(TAG_Gameplay_AbilityInputBlocked))  // 身上有"技能输入被封"标记
	{
		ClearAbilityInput();
		return;                              // 清空所有输入 → 按什么都没用（被控制/禁赛时）
	}

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	// —— ① 处理"按住"的技能（WhileInputActive）——
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)  // 还按着的手柄
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())   // 这个技能还没在跑
			{
				const ULyraGameplayAbility* LyraAbilityCDO = Cast<ULyraGameplayAbility>(AbilitySpec->Ability);
				if (LyraAbilityCDO && LyraAbilityCDO->GetActivationPolicy() == ELyraAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);  // 按住且是"按住类" → 想激活它
				}
			}
		}
	}

	// —— ② 处理"按一下"的技能（OnInputTriggered）——
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)  // 本帧刚按下的手柄
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;
				if (AbilitySpec->IsActive())    // 已经在这跑 → 把"按下"当事件发给技能
					AbilitySpecInputPressed(*AbilitySpec);
				else
				{
					const ULyraGameplayAbility* LyraAbilityCDO = Cast<ULyraGameplayAbility>(AbilitySpec->Ability);
					if (LyraAbilityCDO && LyraAbilityCDO->GetActivationPolicy() == ELyraAbilityActivationPolicy::OnInputTriggered)
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);  // 按一下且是"触发类" → 想激活它
				}
			}
		}
	}

	// —— ③ 集中统一激活 ——
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
		TryActivateAbility(AbilitySpecHandle);   // 真正去激活！

	// —— ④ 松开处理 + 清空本周期记录 ——
	// 松开时把 InputPressed=false，若技能还在跑就发"松开"事件；然后 Reset 三个数组
	...
}
```

**对着看就懂了**：
- 每个技能手柄被记录时，系统**查它的 `ActivationPolicy`**：是 `WhileInputActive` 就从"按住数组"里捞，是 `OnInputTriggered` 就从"按下数组"里捞，捞到就送进"待激活列表"，最后统一 `TryActivateAbility`。
- **"按住就持续、松手就停"的实现**：按住时手柄一直留在 `InputHeldSpecHandles`，每帧都被捞到喂技能；**一松手手柄从数组移除，系统不再喂它 → 技能收尾**。
- `OnSpawn` 不在 `ProcessAbilityInput` 里处理，它走**另一条路**：角色拿到化身时，`TryActivateAbilitiesOnSpawn` 主动激活。源码（`LyraAbilitySystemComponent.cpp` 第 85~95 行）：

```cpp
void ULyraAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (const ULyraGameplayAbility* LyraAbilityCDO = Cast<ULyraGameplayAbility>(AbilitySpec.Ability))
			LyraAbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
	}
}
```

> **小结 `ActivationPolicy`**：它管"自己啥时候动"。实现上 = 系统每帧在 `ProcessAbilityInput` 里读它、把技能分门别类送进"待激活队列"；`OnSpawn` 例外，走出生时自动激活那一路。

---

## 二、标签 2：`ActivationGroup` —— 会不会跟别人互殴

### 概念（先 3 种角色秒懂）

| 值 | 大白话 | 游戏里 |
|---|---|---|
| `Independent` | **独行侠**：谁也管不着我，我也不管别人 | 走路/移动 |
| `Exclusive_Replaceable` | **独占但怂**：我放着，来个更横的我让路 | 普攻/小招 |
| `Exclusive_Blocking` | **独占且横**：我一放，别的独占全让路 | 大招/处决 |

源码（`LyraGameplayAbility.h` 第 57~70 行）：

```cpp
UENUM(BlueprintType)
enum class ELyraAbilityActivationGroup : uint8
{
	Independent,           // Ability runs independently of all other abilities.
	Exclusive_Replaceable, // Ability is canceled and replaced by other exclusive abilities.
	Exclusive_Blocking,    // Ability blocks all other exclusive abilities from activating.
	MAX
};
```

### 底层"账本"：`ActivationGroupCounts`

互殴判断靠 ASC 内部一个**计数数组**当账本，记着"每种分组现在有几个技能在跑"。源码（`LyraAbilitySystemComponent.h` 第 106~107 行）：

```cpp
	// Number of abilities running in each activation group.
	int32 ActivationGroupCounts[(uint8)ELyraAbilityActivationGroup::MAX];
```

`MAX` 就是枚举第 3 个值，所以这是一个长度 3 的数组：`[Independent数, Replaceable数, Blocking数]`。

### 实现①：谁来"记/销"账本 —— 技能激活/结束时（`NotifyAbilityActivated` / `NotifyAbilityEnded`）

官方 ASC 在技能激活成功/结束时都会回调通知，Lyra 借这两个通知维护账本。源码（`LyraAbilitySystemComponent.cpp` 第 320~354 行）：

```cpp
void ULyraAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);
	if (ULyraGameplayAbility* LyraAbility = Cast<ULyraGameplayAbility>(Ability))
	{
		AddAbilityToActivationGroup(LyraAbility->GetActivationGroup(), LyraAbility);  // 激活成功 → 账本 +1
	}
}

void ULyraAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);
	if (ULyraGameplayAbility* LyraAbility = Cast<ULyraGameplayAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(LyraAbility->GetActivationGroup(), LyraAbility);  // 结束 → 账本 -1
	}
}
```

`AddAbilityToActivationGroup` / `RemoveAbilityFromActivationGroup` 里就是数组成员的 `++` / `--`（`LyraAbilitySystemComponent.cpp` 第 432~470 行）：

```cpp
void ULyraAbilitySystemComponent::AddAbilityToActivationGroup(ELyraAbilityActivationGroup Group, ULyraGameplayAbility* LyraAbility)
{
	check(LyraAbility);
	check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);
	ActivationGroupCounts[(uint8)Group]++;          // 我这个分组在跑的数量 +1

	switch (Group)
	{
	case ELyraAbilityActivationGroup::Independent:
		break;                                       // 独行侠：不取消任何人
	case ELyraAbilityActivationGroup::Exclusive_Replaceable:
	case ELyraAbilityActivationGroup::Exclusive_Blocking:
		// 新来个"独占" → 把场上的 Exclusive_Replaceable（怂的）全部顶掉
		CancelActivationGroupAbilities(ELyraAbilityActivationGroup::Exclusive_Replaceable, LyraAbility, /*bReplicate*/false);
		break;
	}
}
```

> 看到没：**`AddAbilityToActivationGroup` 不只记账，还负责"顶"**——新独占技能一进来，立刻 cancel 掉场上的 `Exclusive_Replaceable`。这就是"你放普攻，突然开大 → 普攻被顶掉"的实现。

### 实现②：怎么"挡" —— 激活前查 `IsActivationGroupBlocked`

要让"大招放着时，别的独占技能开不了"，得在**激活前**拦住。Lyra 在技能的 `CanActivateAbility` 里加了一步检查。源码（`LyraGameplayAbility.cpp` 第 136~160 行）：

```cpp
bool ULyraGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, ...) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
		return false;
	if (!Super::CanActivateAbility(...))          // 先走官方基础检查（冷却、Tag、能否主动等）
		return false;

	ULyraAbilitySystemComponent* LyraASC = CastChecked<ULyraAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (LyraASC->IsActivationGroupBlocked(ActivationGroup))   // ← 问系统"我这个分组被挡了吗"
	{
		if (OptionalRelevantTags)
			OptionalRelevantTags->AddTag(LyraGameplayTags::Ability_ActivateFail_ActivationGroup);
		return false;                             // 被挡 → 激活失败，放不出来
	}
	return true;
}
```

而"被挡吗"是这么算的（`LyraAbilitySystemComponent.cpp` 第 407~430 行）：

```cpp
bool ULyraAbilitySystemComponent::IsActivationGroupBlocked(ELyraAbilityActivationGroup Group) const
{
	bool bBlocked = false;
	switch (Group)
	{
	case ELyraAbilityActivationGroup::Independent:
		bBlocked = false;   // 独行侠永不被挡
		break;
	case ELyraAbilityActivationGroup::Exclusive_Replaceable:
	case ELyraAbilityActivationGroup::Exclusive_Blocking:
		// 独占类：只要账本里 Exclusive_Blocking 的数量 > 0（有大招在跑），就被挡
		bBlocked = (ActivationGroupCounts[(uint8)ELyraAbilityActivationGroup::Exclusive_Blocking] > 0);
		break;
	}
	return bBlocked;
}
```

**对着看就懂了**：
- **"挡"** = 大招激活后账本里 `Exclusive_Blocking` 计数 ≥1；此后任何 `Exclusive_*` 技能去激活，`CanActivateAbility` 调 `IsActivationGroupBlocked` 查到 ≥1 → 返回 false → 放不出来。
- 为什么 `Independent` 永不被挡？因为它 case 里直接 `bBlocked = false`——走路永远可以。
- **先"挡"新的不让进（CanActivateAbility），再"顶"旧怂的让路（AddAbilityToActivationGroup）**——一进一出，保证场上同一时刻最多一个 `Exclusive` 在跑。

---

## 三、整条链路的完整串场

把两个标签对应的函数拼起来，就是你从"按键盘"到"技能放出来/放不出来"的全过程：

```
你按技能键
  → AbilityInputTagPressed：技能绑了这个按键Tag？→ 手柄记进数组
  → 每帧 ProcessAbilityInput：
       读 ActivationPolicy —— WhileInputActive 从按住数组捞 / OnInputTriggered 从按下数组捞
       → 送进"待激活队列"
  → TryActivateAbility：
       技能先问 CanActivateAbility：
          读 ActivationGroup —— 走官方检查 + IsActivationGroupBlocked 看自己分组被挡没
          被挡 → 直接失败，放不出来
       没被挡 → 激活成功
  → NotifyAbilityActivated → AddAbilityToActivationGroup：
        账本 +1；若是 Exclusive 新进来 → cancel 掉场上的 Exclusive_Replaceable（顶掉旧招）
  → 你一松手 / 技能放完 → NotifyAbilityEnded → 账本 -1 → 别的独占又能放了
```

**两个标签各自负责的段落**：
- `ActivationPolicy` → 在 `ProcessAbilityInput` 被读 → 决定"**何时**进待激活队列"。
- `ActivationGroup` → 在 `CanActivateAbility`（挡）和 `AddAbilityToActivationGroup`（顶）被读 → 决定"**能不能激活、激活时顶不顶别人**"。

---

## 四、三个"所以"直接对号入座

- **"大招放的时候为什么普攻打不出来？"** → 大招在账本把 `Exclusive_Blocking` 加到 ≥1；普攻激活前 `CanActivateAbility → IsActivationGroupBlocked` 查到 ≥1 → 被拒。见本文"实现②"。
- **"按住蓄力，为什么松手就停？"** → 它是 `WhileInputActive`，手柄留在 `InputHeldSpecHandles` 才被每帧喂；一松手手柄移除，`ProcessAbilityInput` 不再喂 → 技能收尾。见"标签1实现"。
- **"这俩标签到底是数据还是逻辑？"** → 只是技能上两个枚举**字段**（存数据）；执行它们靠 ASC 的 `ProcessAbilityInput` / `CanActivateAbility` / `AddAbilityToActivationGroup`。**标签是开关，ASC 是执行开关的人。**
