// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ControllerComponent.h"
#include "GameplayTagContainer.h"

#include "TenkaichiWeaponStateComponent.generated.h"

class UObject;
struct FFrame;
struct FGameplayAbilityTargetDataHandle;
struct FGameplayEffectContextHandle;
struct FHitResult;

// 命中标记会显示在准星上，用来提示远程武器打中了哪里
// '成功的'命中标记 = 打中了敌人并造成伤害
struct FTenkaichiScreenSpaceHitLocation
{
	/** 命中点在视口上的屏幕坐标 */
	FVector2D Location;
	FGameplayTag HitZone;
	bool bShowAsSuccess = false;
};

struct FTenkaichiServerSideHitMarkerBatch
{
	FTenkaichiServerSideHitMarkerBatch() { }

	FTenkaichiServerSideHitMarkerBatch(uint16 InUniqueId) :
		UniqueId(InUniqueId)
	{ }

	TArray<FTenkaichiScreenSpaceHitLocation> Markers;

	uint16 UniqueId = 0;
};

// 追踪武器状态和最近确认的命中标记，用来在屏幕上显示
UCLASS()
class UTenkaichiWeaponStateComponent : public UControllerComponent
{
	GENERATED_BODY()

public:

	UTenkaichiWeaponStateComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Client, Reliable)
	void ClientConfirmTargetData(uint16 UniqueId, bool bSuccess, const TArray<uint8>& HitReplaces);

	void AddUnconfirmedServerSideHitMarkers(const FGameplayAbilityTargetDataHandle& InTargetData, const TArray<FHitResult>& FoundHits);

	/** 更新这个玩家最近一次打出伤害的时间 */
	void UpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext);

	/** 获取这个玩家最近打出的伤害所在的屏幕坐标（用于准星命中提示） */
	void GetLastWeaponDamageScreenLocations(TArray<FTenkaichiScreenSpaceHitLocation>& WeaponDamageScreenLocations)
	{
		WeaponDamageScreenLocations = LastWeaponDamageScreenLocations;
	}

	/** 返回距最近一次"命中伤害"通知过了多久 */
	double GetTimeSinceLastHitNotification() const;

	/** 返回还有多少个"服务器还没确认"的命中标记 */
	int32 GetUnconfirmedServerSideHitMarkerCount() const
	{
		return UnconfirmedServerSideHitMarkers.Num();
	}

protected:
	// 过滤命中结果，决定这次命中是否算"成功命中"
	// 默认行为：打中了与拥有者控制器 Pawn 不同队伍的角色就算成功
	virtual bool ShouldShowHitAsSuccess(const FHitResult& Hit) const;

	virtual bool ShouldUpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext) const;

	void ActuallyUpdateDamageInstigatedTime();

private:
	/** 这个控制器最近一次造成武器伤害的时刻 */
	double LastWeaponDamageInstigatedTime = 0.0;

	/** 我们最近造成的武器伤害所在的屏幕坐标（已确认的命中） */
	TArray<FTenkaichiScreenSpaceHitLocation> LastWeaponDamageScreenLocations;

	/** 还没被服务器确认的命中标记 */
	TArray<FTenkaichiServerSideHitMarkerBatch> UnconfirmedServerSideHitMarkers;
};