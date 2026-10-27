// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/TenkaichiHealthComponent.h"

#include "AbilitySystem/Attributes/TenkaichiAttributeSet.h"
#include "TenkaichiLogChannels.h"
// #include "System/TenkaichiAssetManager.h"   // ⚠️ P14 未就位，暂时挂起（见 DamageSelfDestruct）
#include "System/TenkaichiGameData.h"
#include "TenkaichiGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/TenkaichiHealthSet.h"
#include "Messages/TenkaichiVerbMessage.h"
#include "Messages/TenkaichiVerbMessageHelpers.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiHealthComponent)

// 定义"淘汰消息"这个游戏标签，作为击杀播报的频道
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Tenkaichi_Elimination_Message, "Tenkaichi.Elimination.Message");


// 构造函数：关闭 Tick、开启网络复制，并把三个字段置空
UTenkaichiHealthComponent::UTenkaichiHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	AbilitySystemComponent = nullptr;
	HealthSet = nullptr;
	DeathState = ETenkaichiDeathState::NotDead;
}

void UTenkaichiHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTenkaichiHealthComponent, DeathState);
}

// 组件注销时自动反初始化，避免残留引用
void UTenkaichiHealthComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();

	Super::OnUnregister();
}

// 使用给定的能力系统组件来初始化本组件
void UTenkaichiHealthComponent::InitializeWithAbilitySystem(UTenkaichiAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		UE_LOG(LogTenkaichi, Error, TEXT("TenkaichiHealthComponent: Health component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTenkaichi, Error, TEXT("TenkaichiHealthComponent: Cannot initialize health component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	HealthSet = AbilitySystemComponent->GetSet<UTenkaichiHealthSet>();
	if (!HealthSet)
	{
		UE_LOG(LogTenkaichi, Error, TEXT("TenkaichiHealthComponent: Cannot initialize health component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	// 注册监听属性变化
	HealthSet->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);
	HealthSet->OnMaxHealthChanged.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);

	// 临时：把属性重置为默认值。将来会改由一张配置表驱动。
	AbilitySystemComponent->SetNumericAttributeBase(UTenkaichiHealthSet::GetHealthAttribute(), HealthSet->GetMaxHealth());

	ClearGameplayTags();

	OnHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
	OnMaxHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
}

// 反初始化本组件，清空对能力系统的所有引用
void UTenkaichiHealthComponent::UninitializeFromAbilitySystem()
{
	ClearGameplayTags();

	if (HealthSet)
	{
		HealthSet->OnHealthChanged.RemoveAll(this);
		HealthSet->OnMaxHealthChanged.RemoveAll(this);
		HealthSet->OnOutOfHealth.RemoveAll(this);
	}

	HealthSet = nullptr;
	AbilitySystemComponent = nullptr;
}

// 清空本组件施加的所有游戏标签
void UTenkaichiHealthComponent::ClearGameplayTags()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(TenkaichiGameplayTags::Status_Death_Dying, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(TenkaichiGameplayTags::Status_Death_Dead, 0);
	}
}

// 返回当前的血量值
float UTenkaichiHealthComponent::GetHealth() const
{
	return (HealthSet ? HealthSet->GetHealth() : 0.0f);
}

// 返回当前的最大血量值
float UTenkaichiHealthComponent::GetMaxHealth() const
{
	return (HealthSet ? HealthSet->GetMaxHealth() : 0.0f);
}

// 返回当前血量归一化后的值，范围 [0.0, 1.0]
float UTenkaichiHealthComponent::GetHealthNormalized() const
{
	if (HealthSet)
	{
		const float Health = HealthSet->GetHealth();
		const float MaxHealth = HealthSet->GetMaxHealth();

		return ((MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f);
	}

	return 0.0f;
}

// 血量属性变化时的内部处理：转发为 OnHealthChanged 广播
void UTenkaichiHealthComponent::HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

// 最大血量属性变化时的内部处理：转发为 OnMaxHealthChanged 广播
void UTenkaichiHealthComponent::HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

// 血量耗尽时的内部处理：广播死亡游戏事件与击杀消息
void UTenkaichiHealthComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent && DamageEffectSpec)
	{
		// 通过拥有者的能力系统发送 "GameplayEvent.Death" 游戏事件。这可以用来触发一个死亡技能。
		{
			FGameplayEventData Payload;
			Payload.EventTag = TenkaichiGameplayTags::GameplayEvent_Death;
			Payload.Instigator = DamageInstigator;
			Payload.Target = AbilitySystemComponent->GetAvatarActor();
			Payload.OptionalObject = DamageEffectSpec->Def;
			Payload.ContextHandle = DamageEffectSpec->GetEffectContext();
			Payload.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = DamageMagnitude;

			FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
			AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
		}

		// 发送一条标准化的动词消息，供其他系统监听
		{
			FTenkaichiVerbMessage Message;
			Message.Verb = TAG_Tenkaichi_Elimination_Message;
			Message.Instigator = DamageInstigator;
			Message.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Message.Target = UTenkaichiVerbMessageHelpers::GetPlayerStateFromObject(AbilitySystemComponent->GetAvatarActor());
			Message.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
			//@TODO: 补全上下文标签，以及任何非能力系统来源的施加者/目标标签
			//@TODO: 判断这是不是敌方击杀、自尽、队友击杀等等

			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(Message.Verb, Message);
		}

		//@TODO: 助攻消息（可以从别处统计的伤害量算出来）？
	}

#endif // #if WITH_SERVER_CODE
}

// 死亡状态从服务器复制到客户端时的回调
void UTenkaichiHealthComponent::OnRep_DeathState(ETenkaichiDeathState OldDeathState)
{
	const ETenkaichiDeathState NewDeathState = DeathState;

	// 暂时回退死亡状态，因为我们依赖 StartDeath 和 FinishDeath 来改变它
	DeathState = OldDeathState;

	if (OldDeathState > NewDeathState)
	{
		// 服务器试图把我们设回去，但我们本地已经预测得比服务器更远了
		UE_LOG(LogTenkaichi, Warning, TEXT("TenkaichiHealthComponent: Predicted past server death state [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		return;
	}

	if (OldDeathState == ETenkaichiDeathState::NotDead)
	{
		if (NewDeathState == ETenkaichiDeathState::DeathStarted)
		{
			StartDeath();
		}
		else if (NewDeathState == ETenkaichiDeathState::DeathFinished)
		{
			StartDeath();
			FinishDeath();
		}
		else
		{
			UE_LOG(LogTenkaichi, Error, TEXT("TenkaichiHealthComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}
	else if (OldDeathState == ETenkaichiDeathState::DeathStarted)
	{
		if (NewDeathState == ETenkaichiDeathState::DeathFinished)
		{
			FinishDeath();
		}
		else
		{
			UE_LOG(LogTenkaichi, Error, TEXT("TenkaichiHealthComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}

	ensureMsgf((DeathState == NewDeathState), TEXT("TenkaichiHealthComponent: Death transition failed [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
}

// 为拥有者开启死亡流程
void UTenkaichiHealthComponent::StartDeath()
{
	if (DeathState != ETenkaichiDeathState::NotDead)
	{
		return;
	}

	DeathState = ETenkaichiDeathState::DeathStarted;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(TenkaichiGameplayTags::Status_Death_Dying, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathStarted.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

// 为拥有者结束死亡流程
void UTenkaichiHealthComponent::FinishDeath()
{
	if (DeathState != ETenkaichiDeathState::DeathStarted)
	{
		return;
	}

	DeathState = ETenkaichiDeathState::DeathFinished;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(TenkaichiGameplayTags::Status_Death_Dead, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathFinished.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

// 施加足以杀死拥有者的伤害
void UTenkaichiHealthComponent::DamageSelfDestruct(bool bFellOutOfWorld)
{
#if 0   // ⚠️ 注释挂起：依赖 P14 UTenkaichiAssetManager（注释备忘录 ㉘），等 P14 就位后解开
	if ((DeathState == ETenkaichiDeathState::NotDead) && AbilitySystemComponent)
	{
		const TSubclassOf<UGameplayEffect> DamageGE = UTenkaichiAssetManager::GetSubclass(UTenkaichiGameData::Get().DamageGameplayEffect_SetByCaller);
		if (!DamageGE)
		{
			UE_LOG(LogTenkaichi, Error, TEXT("TenkaichiHealthComponent: DamageSelfDestruct failed for owner [%s]. Unable to find gameplay effect [%s]."), *GetNameSafe(GetOwner()), *UTenkaichiGameData::Get().DamageGameplayEffect_SetByCaller.GetAssetName());
			return;
		}

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageGE, 1.0f, AbilitySystemComponent->MakeEffectContext());
		FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

		if (!Spec)
		{
			UE_LOG(LogTenkaichi, Error, TEXT("TenkaichiHealthComponent: DamageSelfDestruct failed for owner [%s]. Unable to make outgoing spec for [%s]."), *GetNameSafe(GetOwner()), *GetNameSafe(DamageGE));
			return;
		}

		Spec->AddDynamicAssetTag(TAG_Gameplay_DamageSelfDestruct);

		if (bFellOutOfWorld)
		{
			Spec->AddDynamicAssetTag(TAG_Gameplay_FellOutOfWorld);
		}

		const float DamageAmount = GetMaxHealth();

		Spec->SetSetByCallerMagnitude(TenkaichiGameplayTags::SetByCaller_Damage, DamageAmount);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec);
	}
#endif  // ⚠️ 注释挂起结束（㉘）
}