// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Curves/CurveFloat.h"

#include "TenkaichiWeaponInstance.h"
#include "AbilitySystem/TenkaichiAbilitySourceInterface.h"

#include "TenkaichiRangedWeaponInstance.generated.h"

class UPhysicalMaterial;

/**
 * UTenkaichiRangedWeaponInstance
 *
 * 一件代表"远程武器"的装备，生成并应用到 Pawn 上
 */
UCLASS()
class UTenkaichiRangedWeaponInstance : public UTenkaichiWeaponInstance, public ITenkaichiAbilitySourceInterface
{
	GENERATED_BODY()

public:
	// 构造函数（带默认 ObjectInitializer，允许无参构造）
	UTenkaichiRangedWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 加载完成后的钩子：编辑器下刷新调试可视化
	virtual void PostLoad() override;

#if WITH_EDITOR
	// 编辑器下属性被修改时的回调：刷新调试可视化
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	// 更新调试可视化（把内部曲线/数值同步到只读的 Debug_* 属性上，方便在编辑器里观察）
	void UpdateDebugVisualization();
#endif

	// 返回"一枪打几发子弹"（散弹枪>1，步枪=1）
	int32 GetBulletsPerCartridge() const
	{
		return BulletsPerCartridge;
	}
	
	/** 返回当前的散布角（度，直径角） */
	float GetCalculatedSpreadAngle() const
	{
		return CurrentSpreadAngle;
	}

	// 返回当前综合散布倍率（第一发必准时返回 0 → 弹道完全居中）
	float GetCalculatedSpreadAngleMultiplier() const
	{
		return bHasFirstShotAccuracy ? 0.0f : CurrentSpreadAngleMultiplier;
	}

	// 是否处于"第一发必准"状态
	bool HasFirstShotAccuracy() const
	{
		return bHasFirstShotAccuracy;
	}

	// 返回散布指数（越大弹道越往中心聚拢）
	float GetSpreadExponent() const
	{
		return SpreadExponent;
	}

	// 返回最大有效射程（超出不造成伤害）
	float GetMaxDamageRange() const
	{
		return MaxDamageRange;
	}

	// 返回弹道扫描球半径（0 = 射线，>0 = 球体扫描）
	float GetBulletTraceSweepRadius() const
	{
		return BulletTraceSweepRadius;
	}

protected:
#if WITH_EDITORONLY_DATA
	// 以下 Debug_* 属性仅供编辑器可视化观察，运行时无作用

	// 编辑器可见：当前热量下限（由 HeatToSpreadCurve 等曲线推导）
	UPROPERTY(VisibleAnywhere, Category = "Spread|Fire Params")
	float Debug_MinHeat = 0.0f;

	// 编辑器可见：当前热量上限（由曲线推导）
	UPROPERTY(VisibleAnywhere, Category = "Spread|Fire Params")
	float Debug_MaxHeat = 0.0f;

	// 编辑器可见：当前最小散布角（度）
	UPROPERTY(VisibleAnywhere, Category="Spread|Fire Params", meta=(ForceUnits=deg))
	float Debug_MinSpreadAngle = 0.0f;

	// 编辑器可见：当前最大散布角（度）
	UPROPERTY(VisibleAnywhere, Category="Spread|Fire Params", meta=(ForceUnits=deg))
	float Debug_MaxSpreadAngle = 0.0f;

	// 编辑器可见：当前热量
	UPROPERTY(VisibleAnywhere, Category="Spread Debugging")
	float Debug_CurrentHeat = 0.0f;

	// 编辑器可见：当前散布角（度）
	UPROPERTY(VisibleAnywhere, Category="Spread Debugging", meta = (ForceUnits=deg))
	float Debug_CurrentSpreadAngle = 0.0f;

	// 编辑器可见：当前综合散布倍率
	UPROPERTY(VisibleAnywhere, Category = "Spread Debugging", meta=(ForceUnits=x))
	float Debug_CurrentSpreadAngleMultiplier = 1.0f;

#endif

	// 散布指数：决定弹道在散布圆内如何分布。越大越靠中心聚拢（默认 1.0 = 均匀散布）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.1), Category="Spread|Fire Params")
	float SpreadExponent = 1.0f;

	// 曲线：热量 → 散布角。X 范围决定热量上下限，Y 范围决定散布角上下限
	UPROPERTY(EditAnywhere, Category = "Spread|Fire Params")
	FRuntimeFloatCurve HeatToSpreadCurve;

	// 曲线：当前热量 → 单发子弹增加的热量。典型为平直曲线（单点），也可做"过热惩罚"（越热加得越多）
	UPROPERTY(EditAnywhere, Category="Spread|Fire Params")
	FRuntimeFloatCurve HeatToHeatPerShotCurve;
	
	// 曲线：当前热量 → 每秒冷却的热量。典型为平直曲线（单点），也可做"过热惩罚"（越热冷却越慢）
	UPROPERTY(EditAnywhere, Category="Spread|Fire Params")
	FRuntimeFloatCurve HeatToCoolDownPerSecondCurve;

	// 停止射击后，过多久才开始冷却回准（秒）
	UPROPERTY(EditAnywhere, Category="Spread|Fire Params", meta=(ForceUnits=s))
	float SpreadRecoveryCooldownDelay = 0.0f;

	// 是否允许"热量最低 + 各倍率归位"时进入第一发必准状态
	UPROPERTY(EditAnywhere, Category="Spread|Fire Params")
	bool bAllowFirstShotAccuracy = false;

	// 瞄准（开镜）时的散布倍率（一般 <1 更准）
	UPROPERTY(EditAnywhere, Category="Spread|Player Params", meta=(ForceUnits=x))
	float SpreadAngleMultiplier_Aiming = 1.0f;

	// 站定/极慢移动时的散布倍率（越接近静止越准）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits=x))
	float SpreadAngleMultiplier_StandingStill = 1.0f;

	// 站定准度的过渡速度（越大切换越快，0 为瞬切）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params")
	float TransitionRate_StandingStill = 5.0f;

	// 速度 ≤ 该值时算"静止"（cm/s）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits="cm/s"))
	float StandingStillSpeedThreshold = 80.0f;

	// 从"静止"到"移动"的过渡带宽度（cm/s），超过后站定加成完全消失
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits="cm/s"))
	float StandingStillToMovingSpeedRange = 20.0f;

	// 蹲下时的散布倍率（平滑过渡）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits=x))
	float SpreadAngleMultiplier_Crouching = 1.0f;

	// 蹲下准度的过渡速率
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params")
	float TransitionRate_Crouching = 5.0f;

	// 跳跃/下落时的散布倍率（通常 >1 更飘）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits=x))
	float SpreadAngleMultiplier_JumpingOrFalling = 1.0f;

	// 空中准度的过渡速率
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params")
	float TransitionRate_JumpingOrFalling = 5.0f;

	// 一枪发射的子弹数（散弹枪>1，步枪=1）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config")
	int32 BulletsPerCartridge = 1;

	// 最大有效射程（cm），超出不造成伤害
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config", meta=(ForceUnits=cm))
	float MaxDamageRange = 25000.0f;

	// 弹道扫描球半径（0 = 射线，>0 = 球体扫描，便于命中细目标）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config", meta=(ForceUnits=cm))
	float BulletTraceSweepRadius = 0.0f;

	// 曲线：距离（cm）→ 伤害倍率。无数据 = 不衰减
	UPROPERTY(EditAnywhere, Category = "Weapon Config")
	FRuntimeFloatCurve DistanceDamageFalloff;

	// 物理材质标签 → 伤害倍率。多个标签命中时相乘
	UPROPERTY(EditAnywhere, Category = "Weapon Config")
	TMap<FGameplayTag, float> MaterialDamageMultiplier;

private:
	// 上次开火时间（相对世界时间）
	double LastFireTime = 0.0;

	// 当前热量
	float CurrentHeat = 0.0f;

	// 当前散布角（度，直径角）
	float CurrentSpreadAngle = 0.0f;

	// 是否处于"第一发必准"状态
	bool bHasFirstShotAccuracy = false;

	// 当前综合散布倍率（瞄准×站定×蹲下×跳跃 相乘）
	float CurrentSpreadMultiplier = 1.0f;

	// 站定倍率（当前平滑过渡后的值）
	float StandingStillMultiplier = 1.0f;

	// 空中倍率（当前平滑过渡后的值）
	float JumpFallMultiplier = 1.0f;

	// 蹲下倍率（当前平滑过渡后的值）
	float CrouchingMultiplier = 1.0f;

public:
	// 每帧更新热量/散布/倍率，算出是否"第一发必准"
	void Tick(float DeltaSeconds);

	//~UTenkaichiWeaponInstance 接口开始
	// 装备时：初始化热量到中间值 + 重算散布角
	virtual void OnEquipped() override;
	// 卸下时
	virtual void OnUnequipped() override;
	//~UTenkaichiWeaponInstance 接口结束

	// 开枪时调用：加热量 → 重算散布角
	void AddSpread();

	//~ITenkaichiAbilitySourceInterface 接口开始
	// 距离衰减：根据命中距离返回伤害倍率（实现接口）
	virtual float GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const override;
	// 物理材质衰减：按命中物物理材质标签返回伤害倍率（实现接口）
	virtual float GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const override;
	//~ITenkaichiAbilitySourceInterface 接口结束

private:
	// 由三张热量曲线推算热量下限与上限
	void ComputeHeatRange(float& MinHeat, float& MaxHeat);
	// 由散布曲线推算散布角下限与上限
	void ComputeSpreadRange(float& MinSpread, float& MaxSpread);

	// 把热量钳制在 [下限, 上限] 内
	inline float ClampHeat(float NewHeat)
	{
		float MinHeat;
		float MaxHeat;
		ComputeHeatRange(/*out*/ MinHeat, /*out*/ MaxHeat);

		return FMath::Clamp(NewHeat, MinHeat, MaxHeat);
	}

	// 更新热量与散布角，返回散布是否已回到最小值
	bool UpdateSpread(float DeltaSeconds);

	// 更新各倍率（瞄准/站定/蹲下/跳跃），返回是否全部归位
	bool UpdateMultipliers(float DeltaSeconds);
};