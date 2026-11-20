// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Equipment/TenkaichiGameplayAbility_FromEquipment.h"

#include "TenkaichiGameplayAbility_RangedWeapon.generated.h"

enum ECollisionChannel : int;

class APawn;
class UTenkaichiRangedWeaponInstance;
class UObject;
struct FCollisionQueryParams;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;
struct FGameplayTag;
struct FGameplayTagContainer;

/** 定义一次开火从哪里开始射线追踪、以及朝向哪里 */
UENUM(BlueprintType)
enum class ETenkaichiAbilityTargetingSource : uint8
{
	// 从玩家的镜头朝"镜头焦点"打（默认，FPS 手感）
	CameraTowardsFocus,
	// 从 Pawn 中心，沿 Pawn 的朝向打
	PawnForward,
	// 从 Pawn 中心，朝镜头焦点打
	PawnTowardsFocus,
	// 从武器枪口位置，沿 Pawn 的朝向打
	WeaponForward,
	// 从武器枪口位置，朝镜头焦点打
	WeaponTowardsFocus,
	// 自定义起点（需要蓝图自己设置起始位置和朝向）
	Custom
};



/**
 * UTenkaichiGameplayAbility_RangedWeapon
 *
 * 由远程武器实例授予、并与该武器关联的"开枪"技能
 */
UCLASS()
class UTenkaichiGameplayAbility_RangedWeapon : public UTenkaichiGameplayAbility_FromEquipment
{
	GENERATED_BODY()

public:

	// 构造函数
	UTenkaichiGameplayAbility_RangedWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 取关联的远程武器实例（装备实例强转）
	UFUNCTION(BlueprintCallable, Category="Tenkaichi|Ability")
	UTenkaichiRangedWeaponInstance* GetWeaponInstance() const;

	//~UGameplayAbility interface
	// 开火前校验：必须有远程武器实例才能开火
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	// 激活技能：绑定目标数据回调 + 更新武器开火时间
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	// 技能结束：解绑回调 + 清目标数据
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface

protected:
	// 一次开火的输入参数：射线起点/终点/方向 + 武器数据 + 能否播放弹道特效
	struct FRangedWeaponFiringInput
	{
		// 射线追踪的起点
		FVector StartTrace;

		// 若瞄准完美时的射线终点
		FVector EndAim;

		// 若瞄准完美时的射线方向
		FVector AimDir;

		// 武器实例（这次开火的数据来源）
		UTenkaichiRangedWeaponInstance* WeaponData = nullptr;

		// 本次开火命中时，能否播放弹道特效（客户端才播放）
		bool bCanPlayBulletFX = false;

		// 构造函数：把 3 个向量清零
		FRangedWeaponFiringInput()
			: StartTrace(ForceInitToZero)
			, EndAim(ForceInitToZero)
			, AimDir(ForceInitToZero)
		{
		}
	};

protected:
	// 在一堆命中结果里，找第一个"命中 Pawn（或其挂载物）"的索引
	static int32 FindFirstPawnHitResult(const TArray<FHitResult>& HitResults);

	// 真正调用物理引擎打射线/球体扫描（SweepRadius>0 时用球体扫描）
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHitResults) const;

	// 单颗子弹：先尝试纯射线，没打中就回退到球体扫描（SweepRadius>0 时）
	FHitResult DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHits) const;

	// 按"每弹夹子弹数"循环：每颗子弹按散布角散开，单独打一条射线
	void TraceBulletsInCartridge(const FRangedWeaponFiringInput& InputData, OUT TArray<FHitResult>& OutHits);

	// 追加需要忽略的碰撞体（默认忽略挂在角色身上的附属物）
	virtual void AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const;

	// 决定这次开火用哪个碰撞通道（默认用武器碰撞通道）
	virtual ECollisionChannel DetermineTraceChannel(FCollisionQueryParams& TraceParams, bool bIsSimulated) const;

	// 本地射线追踪：算起点/方向/终点，打出一串命中结果
	void PerformLocalTargeting(OUT TArray<FHitResult>& OutHits);

	// 取枪口位置（当前简单取 Pawn 位置）
	FVector GetWeaponTargetingSourceLocation() const;

	// 按瞄准模式算这次开火的起点+朝向（返回一个 Transform：位置=起点，X 轴=朝向）
	FTransform GetTargetingTransform(APawn* SourcePawn, ETenkaichiAbilityTargetingSource Source) const;

	// 目标数据就绪回调：上报服务器 + 提交技能 + 触发蓝图接口
	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

	// 触发一次"瞄准→射线→回调"完整流程（蓝图可调用）
	UFUNCTION(BlueprintCallable)
	void StartRangedWeaponTargeting();

	// 命中结果就绪事件：蓝图在这里实现"伤害/特效/声音"
	UFUNCTION(BlueprintImplementableEvent)
	void OnRangedWeaponTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);

private:
	// 目标数据回调的委托句柄（用于技能结束时取消注册）
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;
};