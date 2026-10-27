// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiDamageExecution.h"
#include "AbilitySystem/Attributes/TenkaichiHealthSet.h"
#include "AbilitySystem/Attributes/TenkaichiCombatSet.h"
#include "AbilitySystem/TenkaichiAbilitySourceInterface.h"
#include "Engine/World.h"
#include "TenkaichiLogChannels.h"
#include "Teams/TenkaichiTeamSubsystem.h"

// ✅ P05.5 已就位，include 正常放开（函数体仍挂起，原因见下）
#include "AbilitySystem/TenkaichiGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiDamageExecution)

// 伤害计算的静态数据：把"要抓取的属性"提前算好存起来
struct FDamageStatics
{
	// 基础伤害属性的捕获定义（从源对象 CombatSet 抓取）
	FGameplayEffectAttributeCaptureDefinition BaseDamageDef;

	// 构造函数：用 CombatSet 的 BaseDamage 属性 + 源来源 + 可选 构建捕获定义
	FDamageStatics()
	{
		BaseDamageDef = FGameplayEffectAttributeCaptureDefinition(UTenkaichiCombatSet::GetBaseDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

// 返回伤害计算的静态单例（第一次调用时构造，之后复用）
static FDamageStatics& DamageStatics()
{
	static FDamageStatics Statics;
	return Statics;
}


// 构造函数：往捕获列表注册基础伤害属性，GAS 执行时就会帮我们抓取它
UTenkaichiDamageExecution::UTenkaichiDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().BaseDamageDef);
}

// 执行计算的核心虚函数：GE 施加时被调用，算出这次打多少伤害
void UTenkaichiDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if 0	// ⚠️【注释挂起】P05.5 EffectContext 已就位 ✅；当前阻塞原因仅剩：TeamSubsystem.cpp 整体 #if 0（CanCauseDamage 链接不到）→ 待 ④ 解开后放开本段
#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FTenkaichiGameplayEffectContext* TypedContext = FTenkaichiGameplayEffectContext::ExtractEffectContext(Spec.GetContext());
	check(TypedContext);

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// 评估参数：带上源/目标标签，让属性聚合能正确计算
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	// 从捕获里取出基础伤害值
	float BaseDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BaseDamageDef, EvaluateParameters, BaseDamage);

	// 从 EffectContext 里取"施加者"和"命中结果"
	const AActor* EffectCauser = TypedContext->GetEffectCauser();
	const FHitResult* HitActorResult = TypedContext->GetHitResult();

	AActor* HitActor = nullptr;
	FVector ImpactLocation = FVector::ZeroVector;
	FVector ImpactNormal = FVector::ZeroVector;
	FVector StartTrace = FVector::ZeroVector;
	FVector EndTrace = FVector::ZeroVector;

	// 命中 Actor、表面、区域、距离的计算都依赖"这次计算有没有命中结果"
	// 直接施加、没有经过瞄准的 GE 总是没有命中结果，必须退回到默认信息
	if (HitActorResult)
	{
		const FHitResult& CurHitResult = *HitActorResult;
		HitActor = CurHitResult.HitObjectHandle.FetchActor();
		if (HitActor)
		{
			ImpactLocation = CurHitResult.ImpactPoint;
			ImpactNormal = CurHitResult.ImpactNormal;
			StartTrace = CurHitResult.TraceStart;
			EndTrace = CurHitResult.TraceEnd;
		}
	}

	// 处理"没有命中结果"或"命中结果没返回 Actor"的情况
	UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	if (!HitActor)
	{
		HitActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->GetAvatarActor_Direct() : nullptr;
		if (HitActor)
		{
			ImpactLocation = HitActor->GetActorLocation();
		}
	}

	// 应用队伍伤害/自伤等规则
	float DamageInteractionAllowedMultiplier = 0.0f;
	if (HitActor)
	{
		UTenkaichiTeamSubsystem* TeamSubsystem = HitActor->GetWorld()->GetSubsystem<UTenkaichiTeamSubsystem>();
		if (ensure(TeamSubsystem))
		{
			DamageInteractionAllowedMultiplier = TeamSubsystem->CanCauseDamage(EffectCauser, HitActor) ? 1.0 : 0.0;
		}
	}

	// 计算距离
	double Distance = WORLD_MAX;

	if (TypedContext->HasOrigin())
	{
		Distance = FVector::Dist(TypedContext->GetOrigin(), ImpactLocation);
	}
	else if (EffectCauser)
	{
		Distance = FVector::Dist(EffectCauser->GetActorLocation(), ImpactLocation);
	}
	else
	{
		UE_LOG(LogTenkaichiAbilitySystem, Error, TEXT("Damage Calculation cannot deduce a source location for damage coming from %s; Falling back to WORLD_MAX dist!"), *GetPathNameSafe(Spec.Def));
	}

	// 应用技能来源修饰器（物理材质衰减 + 距离衰减）
	float PhysicalMaterialAttenuation = 1.0f;
	float DistanceAttenuation = 1.0f;
	if (const ITenkaichiAbilitySourceInterface* AbilitySource = TypedContext->GetAbilitySource())
	{
		if (const UPhysicalMaterial* PhysMat = TypedContext->GetPhysicalMaterial())
		{
			PhysicalMaterialAttenuation = AbilitySource->GetPhysicalMaterialAttenuation(PhysMat, SourceTags, TargetTags);
		}

		DistanceAttenuation = AbilitySource->GetDistanceAttenuation(Distance, SourceTags, TargetTags);
	}
	DistanceAttenuation = FMath::Max(DistanceAttenuation, 0.0f);

	// 钳制是在伤害被转成 -health 时做的
	const float DamageDone = FMath::Max(BaseDamage * DistanceAttenuation * PhysicalMaterialAttenuation * DamageInteractionAllowedMultiplier, 0.0f);

	if (DamageDone > 0.0f)
	{
		// 施加一个伤害修饰器，这会在目标上转成 -health
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UTenkaichiHealthSet::GetDamageAttribute(), EGameplayModOp::Additive, DamageDone));
	}
#endif // #if WITH_SERVER_CODE
#endif // ⚠️【注释挂起】结束
}