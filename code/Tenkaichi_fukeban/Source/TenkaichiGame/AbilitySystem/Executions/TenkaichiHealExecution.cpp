// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiHealExecution.h"
#include "AbilitySystem/Attributes/TenkaichiHealthSet.h"
#include "AbilitySystem/Attributes/TenkaichiCombatSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiHealExecution)

// 治疗计算的静态数据：把"要抓取的属性"提前算好存起来
struct FHealStatics
{
	// 基础治疗属性的捕获定义（从源对象 CombatSet 抓取）
	FGameplayEffectAttributeCaptureDefinition BaseHealDef;

	// 构造函数：用 CombatSet 的 BaseHeal 属性 + 源来源 + 可选 构建捕获定义
	FHealStatics()
	{
		BaseHealDef = FGameplayEffectAttributeCaptureDefinition(UTenkaichiCombatSet::GetBaseHealAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

// 返回治疗计算的静态单例（第一次调用时构造，之后复用）
static FHealStatics& HealStatics()
{
	static FHealStatics Statics;
	return Statics;
}


// 构造函数：往捕获列表注册基础治疗属性，GAS 执行时就会帮我们抓取它
UTenkaichiHealExecution::UTenkaichiHealExecution()
{
	RelevantAttributesToCapture.Add(HealStatics().BaseHealDef);
}

// 执行计算的核心虚函数：GE 施加时被调用，算出这次治疗多少
void UTenkaichiHealExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// 评估参数：带上源/目标标签，让属性聚合能正确计算
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	// 从捕获里取出基础治疗值
	float BaseHeal = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HealStatics().BaseHealDef, EvaluateParameters, BaseHeal);

	// 治疗量钳到 ≥0（治疗不可能是负数）
	const float HealingDone = FMath::Max(0.0f, BaseHeal);

	if (HealingDone > 0.0f)
	{
		// 施加一个治疗修饰器，这会在目标上转成 +health
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UTenkaichiHealthSet::GetHealingAttribute(), EGameplayModOp::Additive, HealingDone));
	}
#endif // #if WITH_SERVER_CODE
}