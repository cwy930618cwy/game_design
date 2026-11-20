// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiWeaponStateComponent.h"

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Equipment/TenkaichiEquipmentManagerComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffectTypes.h"
#include "Kismet/GameplayStatics.h"
#include "NativeGameplayTags.h"
#include "Physics/PhysicalMaterialWithTags.h"
#include "Teams/TenkaichiTeamSubsystem.h"
#include "Weapons/TenkaichiRangedWeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiWeaponStateComponent)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Tenkaichi_Gameplay_Zone, "Gameplay.Zone");

// 构造函数：开启复制 + 开启 Tick
UTenkaichiWeaponStateComponent::UTenkaichiWeaponStateComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);

	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bCanEverTick = true;
}

// 每帧更新：找到玩家手上那件远程武器，调用它的 Tick（让散布/热量持续演化）
void UTenkaichiWeaponStateComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (APawn* Pawn = GetPawn<APawn>())
	{
		if (UTenkaichiEquipmentManagerComponent* EquipmentManager = Pawn->FindComponentByClass<UTenkaichiEquipmentManagerComponent>())
		{
			if (UTenkaichiRangedWeaponInstance* CurrentWeapon = Cast<UTenkaichiRangedWeaponInstance>(EquipmentManager->GetFirstInstanceOfType(UTenkaichiRangedWeaponInstance::StaticClass())))
			{
				CurrentWeapon->Tick(DeltaTime);
			}
		}
	}
}

// 判断一次命中是否算"成功命中"：打中不同队伍的角色就算成功
bool UTenkaichiWeaponStateComponent::ShouldShowHitAsSuccess(const FHitResult& Hit) const
{
	AActor* HitActor = Hit.GetActor();

	//@TODO: 别把"没造成伤害（无敌等）"的命中当成功
	UWorld* World = GetWorld();
	if (UTenkaichiTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<UTenkaichiTeamSubsystem>(GetWorld()))
	{
		return TeamSubsystem->CanCauseDamage(GetController<APlayerController>(), Hit.GetActor());
	}

	return false;
}

// 判断这次伤害要不要更新"最近伤害时间"（默认：有施害者就算）
bool UTenkaichiWeaponStateComponent::ShouldUpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext) const
{
	//@TODO: 实现我，对本组件来说其实只关心"由武器或武器射出的投射物造成的伤害"，应过滤到那部分
	//（或者看看施害者是否也是我们当前准星配置的来源）
	return EffectContext.GetEffectCauser() != nullptr;
}

// Client RPC 实现：服务器确认一批命中后，把"成功命中"的屏幕坐标记录到最近伤害列表
void UTenkaichiWeaponStateComponent::ClientConfirmTargetData_Implementation(uint16 UniqueId, bool bSuccess, const TArray<uint8>& HitReplaces)
{
	for (int i = 0; i < UnconfirmedServerSideHitMarkers.Num(); i++)
	{
		FTenkaichiServerSideHitMarkerBatch& Batch = UnconfirmedServerSideHitMarkers[i];
		if (Batch.UniqueId == UniqueId)
		{
			if (bSuccess && (HitReplaces.Num() != Batch.Markers.Num()))
			{
				UWorld* World = GetWorld();
				bool bFoundShowAsSuccessHit = false;

				int32 HitLocationIndex = 0;
				for (const FTenkaichiScreenSpaceHitLocation& Entry : Batch.Markers)
				{
					if (!HitReplaces.Contains(HitLocationIndex) && Entry.bShowAsSuccess)
					{
						// 只需要记录一次伤害时间
						if (!bFoundShowAsSuccessHit)
						{
							ActuallyUpdateDamageInstigatedTime();
						}

						bFoundShowAsSuccessHit = true;

						LastWeaponDamageScreenLocations.Add(Entry);
					}
					++HitLocationIndex;
				}
			}

			UnconfirmedServerSideHitMarkers.RemoveAt(i);
			break;
		}
	}
}

// 记录一批"待确认"命中标记（含屏幕坐标 + 部位标签 + 是否成功）
void UTenkaichiWeaponStateComponent::AddUnconfirmedServerSideHitMarkers(const FGameplayAbilityTargetDataHandle& InTargetData, const TArray<FHitResult>& FoundHits)
{
	FTenkaichiServerSideHitMarkerBatch& NewUnconfirmedHitMarker = UnconfirmedServerSideHitMarkers.Emplace_GetRef(InTargetData.UniqueId);

	if (APlayerController* OwnerPC = GetController<APlayerController>())
	{
		for (const FHitResult& Hit : FoundHits)
		{
			FVector2D HitScreenLocation;
			if (UGameplayStatics::ProjectWorldToScreen(OwnerPC, Hit.Location, /*out*/ HitScreenLocation, /*bPlayerViewportRelative=*/ false))
			{
				FTenkaichiScreenSpaceHitLocation& Entry = NewUnconfirmedHitMarker.Markers.AddDefaulted_GetRef();
				Entry.Location = HitScreenLocation;
				Entry.bShowAsSuccess = ShouldShowHitAsSuccess(Hit);

				// 确定命中部位
				FGameplayTag HitZone;
				if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(Hit.PhysMaterial.Get()))
				{
					for (const FGameplayTag MaterialTag : PhysMatWithTags->Tags)
					{
						if (MaterialTag.MatchesTag(TAG_Tenkaichi_Gameplay_Zone))
						{
							Entry.HitZone = MaterialTag;
							break;
						}
					}
				}
			}
		}
	}
}

// 对外入口：更新"最近一次打出伤害"的时间
void UTenkaichiWeaponStateComponent::UpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext)
{
	if (ShouldUpdateDamageInstigatedTime(EffectContext))
	{
		ActuallyUpdateDamageInstigatedTime();
	}
}

// 真正更新时间戳：若距上次时间超过 0.1 秒，先清空旧命中坐标列表
void UTenkaichiWeaponStateComponent::ActuallyUpdateDamageInstigatedTime()
{
	// 如果距上次伤害时间不短（>0.1 秒），清空旧命中坐标
	UWorld* World = GetWorld();
	if (World->GetTimeSeconds() - LastWeaponDamageInstigatedTime > 0.1)
	{
		LastWeaponDamageScreenLocations.Reset();
	}
	LastWeaponDamageInstigatedTime = World->GetTimeSeconds();
}

// 返回距最近一次命中通知过了多久
double UTenkaichiWeaponStateComponent::GetTimeSinceLastHitNotification() const
{
	UWorld* World = GetWorld();
	return World->TimeSince(LastWeaponDamageInstigatedTime);
}