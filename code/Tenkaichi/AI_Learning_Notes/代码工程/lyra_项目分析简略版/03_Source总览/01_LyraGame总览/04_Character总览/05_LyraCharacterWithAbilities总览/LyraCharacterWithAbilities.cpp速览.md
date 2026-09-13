# `LyraCharacterWithAbilities.cpp` 速览

> 40 行，三个函数 —— **全目录最简洁的实现**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 建 ASC（`SetIsReplicated(true)` + `Mixed` 复制模式）；建 HealthSet 和 CombatSet；`SetNetUpdateFrequency(100.0f)` |
| `PostInitializeComponents()` | `AbilitySystemComponent->InitAbilityActorInfo(this, this)` |
| `GetAbilitySystemComponent()` | 直接返回自己的 ASC |

> 💡 **为什么 NetUpdateFrequency 要设成 100？** 源码注释写了：`AbilitySystemComponent needs to be updated at a high frequency.` —— ASC 在自己身上时，复制频率也要跟着提高。

> 💡 **为什么要手动持有 HealthSet / CombatSet 的引用？** 源码注释解释得很清楚：这两个属性集是由 `AbilitySystemComponent::InitializeComponent` 自动检测发现的，**如果不在这里留一个 UPROPERTY 引用，它们会在被发现之前就被 GC 回收掉**。
