// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiHealthSet.h"
#include "AbilitySystem/Attributes/TenkaichiAttributeSet.h"
#include "TenkaichiGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameplayEffectExtension.h"
#include "Messages/TenkaichiVerbMessage.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiHealthSet)

// 定义生命值相关的 GameplayTag 字符串值
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Damage, "Gameplay.Damage");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_DamageImmunity, "Gameplay.DamageImmunity");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_DamageSelfDestruct, "Gameplay.Damage.SelfDestruct");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_FellOutOfWorld, "Gameplay.Damage.FellOutOfWorld");
UE_DEFINE_GAMEPLAY_TAG(TAG_Tenkaichi_Damage_Message, "Tenkaichi.Damage.Message");

// 构造函数：生命值初始化为满血，并清零跟踪变量
UTenkaichiHealthSet::UTenkaichiHealthSet()
	: Health(100.0f)
	, MaxHealth(100.0f)
{
	bOutOfHealth = false;
	MaxHealthBeforeAttributeChange = 0.0f;
	HealthBeforeAttributeChange = 0.0f;
}

// 声明这两个属性的网络复制条件（无条件复制给所有人，变化时总是通知）
void UTenkaichiHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTenkaichiHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTenkaichiHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

// Health 复制回来时的回调：广播生命值变化/归零事件（但没有施加者信息）
void UTenkaichiHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTenkaichiHealthSet, Health, OldValue);

	// 调用变化回调，但不带施加者（未来可改成显式 RPC）
	// 客户端上的这些事件不应再改变属性

	const float CurrentHealth = GetHealth();
	const float EstimatedMagnitude = CurrentHealth - OldValue.GetCurrentValue();
	
	OnHealthChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);

	if (!bOutOfHealth && CurrentHealth <= 0.0f)
	{
		OnOutOfHealth.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);
	}

	bOutOfHealth = (CurrentHealth <= 0.0f);
}

// MaxHealth 复制回来时的回调：广播最大生命值变化事件
void UTenkaichiHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTenkaichiHealthSet, MaxHealth, OldValue);

	// 调用变化回调，但不带施加者（未来可改成显式 RPC）
	OnMaxHealthChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxHealth() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxHealth());
}

// 游戏效果实际执行之前：处理伤害拦截（伤害免疫/上帝模式）、并保存当前生命值
bool UTenkaichiHealthSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData &Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	// 处理修改"传入的普通伤害"
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		if (Data.EvaluatedData.Magnitude > 0.0f)
		{
			const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(TAG_Gameplay_DamageSelfDestruct);

			if (Data.Target.HasMatchingGameplayTag(TAG_Gameplay_DamageImmunity) && !bIsDamageFromSelfDestruct)
			{
				// 不扣生命值
				Data.EvaluatedData.Magnitude = 0.0f;
				return false;
			}

#if !UE_BUILD_SHIPPING
			// 检查上帝模式作弊（无限生命在下面检查）
			if (Data.Target.HasMatchingGameplayTag(TenkaichiGameplayTags::Cheat_GodMode) && !bIsDamageFromSelfDestruct)
			{
				// 不扣生命值
				Data.EvaluatedData.Magnitude = 0.0f;
				return false;
			}
#endif // #if !UE_BUILD_SHIPPING
		}
	}

	// 保存当前生命值
	HealthBeforeAttributeChange = GetHealth();
	MaxHealthBeforeAttributeChange = GetMaxHealth();

	return true;
}

// 游戏效果实际执行之后：把伤害/治疗映射成生命值变化、钳制并广播
void UTenkaichiHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(TAG_Gameplay_DamageSelfDestruct);
	float MinimumHealth = 0.0f;

#if !UE_BUILD_SHIPPING
	// 上帝模式和无限生命阻止死亡（除非是自毁）
	if (!bIsDamageFromSelfDestruct &&
		(Data.Target.HasMatchingGameplayTag(TenkaichiGameplayTags::Cheat_GodMode) || Data.Target.HasMatchingGameplayTag(TenkaichiGameplayTags::Cheat_UnlimitedHealth) ))
	{
		MinimumHealth = 1.0f;
	}
#endif // #if !UE_BUILD_SHIPPING

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// 发送标准化的动词消息，供其他系统观察
		if (Data.EvaluatedData.Magnitude > 0.0f)
		{
			FTenkaichiVerbMessage Message;
			Message.Verb = TAG_Tenkaichi_Damage_Message;
			Message.Instigator = Data.EffectSpec.GetEffectContext().GetEffectCauser();
			Message.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
			Message.Target = GetOwningActor();
			Message.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
			//@TODO: 填充上下文标签，以及任何非能力系统的来源/施加者标签
			//@TODO: 判断是敌方击杀、自杀还是友军误伤等
			Message.Magnitude = Data.EvaluatedData.Magnitude;

			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(Message.Verb, Message);
		}

		// 转换成 -Health 然后钳制
		SetHealth(FMath::Clamp(GetHealth() - GetDamage(), MinimumHealth, GetMaxHealth()));
		SetDamage(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		// 转换成 +Health 然后钳制
		SetHealth(FMath::Clamp(GetHealth() + GetHealing(), MinimumHealth, GetMaxHealth()));
		SetHealing(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// 钳制并落入下面的归零处理
		SetHealth(FMath::Clamp(GetHealth(), MinimumHealth, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		// TODO 钳制当前生命值?

		// 广播任何请求的最大生命值变化
		OnMaxHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxHealthBeforeAttributeChange, GetMaxHealth());
	}

	// 如果生命值确实变了，激活回调
	if (GetHealth() != HealthBeforeAttributeChange)
	{
		OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
	}

	if ((GetHealth() <= 0.0f) && !bOutOfHealth)
	{
		OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
	}

	// 再检查一次生命值，以防上面的某个事件改了它
	bOutOfHealth = (GetHealth() <= 0.0f);
}

// 属性的基础值即将改变之前：钳制
void UTenkaichiHealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

// 属性当前值即将改变之前：钳制
void UTenkaichiHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

// 属性当前值改变之后：处理最大生命值变化对当前生命值的影响，并复位归零标记
void UTenkaichiHealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		// 确保当前生命值不超过新的最大生命值
		if (GetHealth() > NewValue)
		{
			UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent();
			check(TenkaichiASC);

			TenkaichiASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
		}
	}

	if (bOutOfHealth && (GetHealth() > 0.0f))
	{
		bOutOfHealth = false;
	}
}

// 把属性值钳制到允许的范围（生命值 0~最大，最大生命值不低于 1）
void UTenkaichiHealthSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		// 不允许生命值变负或超过最大生命值
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// 不允许最大生命值低于 1
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}