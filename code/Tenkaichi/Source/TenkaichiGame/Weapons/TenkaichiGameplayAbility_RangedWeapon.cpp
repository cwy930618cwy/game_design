// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiGameplayAbility_RangedWeapon.h"
#include "Weapons/TenkaichiRangedWeaponInstance.h"
#include "Physics/TenkaichiCollisionChannels.h"
#include "TenkaichiLogChannels.h"
#include "AIController.h"
#include "NativeGameplayTags.h"
#include "Weapons/TenkaichiWeaponStateComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/TenkaichiGameplayAbilityTargetData_SingleTargetHit.h"
#include "DrawDebugHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameplayAbility_RangedWeapon)

namespace TenkaichiConsoleVariables
{
	static float DrawBulletTracesDuration = 0.0f;
	static FAutoConsoleVariableRef CVarDrawBulletTraceDuraton(
		TEXT("tenkaichi.Weapon.DrawBulletTraceDuration"),
		DrawBulletTracesDuration,
		TEXT("是否绘制子弹轨迹的调试线（大于 0 时，值为绘制时长（秒））"),
		ECVF_Default);

	static float DrawBulletHitDuration = 0.0f;
	static FAutoConsoleVariableRef CVarDrawBulletHits(
		TEXT("tenkaichi.Weapon.DrawBulletHitDuration"),
		DrawBulletHitDuration,
		TEXT("是否绘制子弹落点的调试点（大于 0 时，值为绘制时长（秒））"),
		ECVF_Default);

	static float DrawBulletHitRadius = 3.0f;
	static FAutoConsoleVariableRef CVarDrawBulletHitRadius(
		TEXT("tenkaichi.Weapon.DrawBulletHitRadius"),
		DrawBulletHitRadius,
		TEXT("开启子弹落点调试绘制时，落点圆点半径多大（单位 uu）"),
		ECVF_Default);
}

// 玩家身上带有这个标签时，开火会被阻止/取消
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_WeaponFireBlocked, "Ability.Weapon.NoFiring");

//////////////////////////////////////////////////////////////////////

// 在锥体内返回一个随机方向。指数越大，子弹越往锥体中心聚拢
FVector VRandConeNormalDistribution(const FVector& Dir, const float ConeHalfAngleRad, const float Exponent)
{
	if (ConeHalfAngleRad > 0.f)
	{
		const float ConeHalfAngleDegrees = FMath::RadiansToDegrees(ConeHalfAngleRad);

		// 把锥体看成两个旋转的组合：一个"远离中心线"，一个"绕圆旋转"。
		// 把指数应用到"远离中心"的旋转上 —— 指数越大，点越往中心聚拢
		const float FromCenter = FMath::Pow(FMath::FRand(), Exponent);
		const float AngleFromCenter = FromCenter * ConeHalfAngleDegrees;
		const float AngleAround = FMath::FRand() * 360.0f;

		FRotator Rot = Dir.Rotation();
		FQuat DirQuat(Rot);
		FQuat FromCenterQuat(FRotator(0.0f, AngleFromCenter, 0.0f));
		FQuat AroundQuat(FRotator(0.0f, 0.0, AngleAround));
		FQuat FinalDirectionQuat = DirQuat * AroundQuat * FromCenterQuat;
		FinalDirectionQuat.Normalize();

		return FinalDirectionQuat.RotateVector(FVector::ForwardVector);
	}
	else
	{
		return Dir.GetSafeNormal();
	}
}


UTenkaichiGameplayAbility_RangedWeapon::UTenkaichiGameplayAbility_RangedWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 把"禁止开火"标签加进阻塞标签：角色带此标签时，这个技能无法激活
	SourceBlockedTags.AddTag(TAG_WeaponFireBlocked);
}

// 取关联的远程武器实例（装备实例强转成远程武器）
UTenkaichiRangedWeaponInstance* UTenkaichiGameplayAbility_RangedWeapon::GetWeaponInstance() const
{
	return Cast<UTenkaichiRangedWeaponInstance>(GetAssociatedEquipment());
}

// 开火前校验：必须有远程武器实例才能开火
bool UTenkaichiGameplayAbility_RangedWeapon::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 先跑父类校验（装备技能基类：必须有装备实例等）
	bool bResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	if (bResult)
	{
		// 再校验：必须能拿到远程武器实例，否则报错
		if (GetWeaponInstance() == nullptr)
		{
			UE_LOG(LogTenkaichiAbilitySystem, Error, TEXT("Weapon ability %s cannot be activated because there is no associated ranged weapon (equipment instance=%s but needs to be derived from %s)"),
				*GetPathName(),
				*GetPathNameSafe(GetAssociatedEquipment()),
				*UTenkaichiRangedWeaponInstance::StaticClass()->GetName());
			bResult = false;
		}
	}

	return bResult;
}

// 在一堆命中结果里，找第一个"命中 Pawn（或其挂载物）"的索引；没有就返回 INDEX_NONE
int32 UTenkaichiGameplayAbility_RangedWeapon::FindFirstPawnHitResult(const TArray<FHitResult>& HitResults)
{
	for (int32 Idx = 0; Idx < HitResults.Num(); ++Idx)
	{
		const FHitResult& CurHitResult = HitResults[Idx];
		if (CurHitResult.HitObjectHandle.DoesRepresentClass(APawn::StaticClass()))
		{
			// 直接命中了一个 Pawn，返回它的索引
			return Idx;
		}
		else
		{
			AActor* HitActor = CurHitResult.HitObjectHandle.FetchActor();
			if ((HitActor != nullptr) && (HitActor->GetAttachParentActor() != nullptr) && (Cast<APawn>(HitActor->GetAttachParentActor()) != nullptr))
			{
				// 命中了一个挂在 Pawn 上的东西（如武器），也算命中
				return Idx;
			}
		}
	}

	return INDEX_NONE;
}

// 追加需要忽略的碰撞体：默认忽略挂在角色身上的所有附属物（防止误伤自己的配件）
void UTenkaichiGameplayAbility_RangedWeapon::AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const
{
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		// 取出挂在角色身上的所有子物体，加进忽略列表
		TArray<AActor*> AttachedActors;
		Avatar->GetAttachedActors(/*out*/ AttachedActors);
		TraceParams.AddIgnoredActors(AttachedActors);
	}
}

// 决定这次开火用哪个碰撞通道：默认用武器碰撞通道
ECollisionChannel UTenkaichiGameplayAbility_RangedWeapon::DetermineTraceChannel(FCollisionQueryParams& TraceParams, bool bIsSimulated) const
{
	return Tenkaichi_TraceChannel_Weapon;
}

// 真正调用物理引擎打射线/球体扫描，并去重命中
FHitResult UTenkaichiGameplayAbility_RangedWeapon::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHitResults) const
{
	TArray<FHitResult> HitResults;
	
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), /*bTraceComplex=*/ true, /*IgnoreActor=*/ GetAvatarActorFromActorInfo());
	TraceParams.bReturnPhysicalMaterial = true;
	AddAdditionalTraceIgnoreActors(TraceParams);
	//TraceParams.bDebugQuery = true;

	const ECollisionChannel TraceChannel = DetermineTraceChannel(TraceParams, bIsSimulated);

	if (SweepRadius > 0.0f)
	{
		// 球体扫描：从起点到终点扫一个球，适合命中细目标
		GetWorld()->SweepMultiByChannel(HitResults, StartTrace, EndTrace, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(SweepRadius), TraceParams);
	}
	else
	{
		// 纯射线
		GetWorld()->LineTraceMultiByChannel(HitResults, StartTrace, EndTrace, TraceChannel, TraceParams);
	}

	FHitResult Hit(ForceInit);
	if (HitResults.Num() > 0)
	{
		// 去重：防止同一 Actor 被扫到多次造成多次伤害
		for (FHitResult& CurHitResult : HitResults)
		{
			auto Pred = [&CurHitResult](const FHitResult& Other)
			{
				return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
			};

			if (!OutHitResults.ContainsByPredicate(Pred))
			{
				OutHitResults.Add(CurHitResult);
			}
		}

		Hit = OutHitResults.Last();
	}
	else
	{
		// 没命中：手动填起点/终点，方便后续调试/特效
		Hit.TraceStart = StartTrace;
		Hit.TraceEnd = EndTrace;
	}

	return Hit;
}

// 取枪口位置：当前简单取 Pawn 的中心位置（后续可加偏移）
FVector UTenkaichiGameplayAbility_RangedWeapon::GetWeaponTargetingSourceLocation() const
{
	// 用 Pawn 的位置作为起点
	APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	check(AvatarPawn);

	const FVector SourceLoc = AvatarPawn->GetActorLocation();
	const FQuat SourceRot = AvatarPawn->GetActorQuat();

	FVector TargetingSourceLocation = SourceLoc;

	//@TODO: 未来可从武器实例加偏移，并根据蹲下/瞄准等状态调整

	return TargetingSourceLocation;
}

// 按瞄准模式算这次开火的起点 + 朝向（返回 Transform：位置=起点，X 轴=朝向）
FTransform UTenkaichiGameplayAbility_RangedWeapon::GetTargetingTransform(APawn* SourcePawn, ETenkaichiAbilityTargetingSource Source) const
{
	check(SourcePawn);
	AController* SourcePawnController = SourcePawn->GetController(); 
	UTenkaichiWeaponStateComponent* WeaponStateComponent = (SourcePawnController != nullptr) ? SourcePawnController->FindComponentByClass<UTenkaichiWeaponStateComponent>() : nullptr;

	// 如果是 Custom 模式，调用者必须自己算，不能走这里
	check(Source != ETenkaichiAbilityTargetingSource::Custom);

	const FVector ActorLoc = SourcePawn->GetActorLocation();
	FQuat AimQuat = SourcePawn->GetActorQuat();
	AController* Controller = SourcePawn->GetController();
	FVector SourceLoc;

	double FocalDistance = 1024.0f;
	FVector FocalLoc;

	FVector CamLoc;
	FRotator CamRot;
	bool bFoundFocus = false;


	if ((Controller != nullptr) && ((Source == ETenkaichiAbilityTargetingSource::CameraTowardsFocus) || (Source == ETenkaichiAbilityTargetingSource::PawnTowardsFocus) || (Source == ETenkaichiAbilityTargetingSource::WeaponTowardsFocus)))
	{
		// 需要镜头焦点的模式：先取镜头位置和朝向
		bFoundFocus = true;

		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC != nullptr)
		{
			PC->GetPlayerViewPoint(/*out*/ CamLoc, /*out*/ CamRot);
		}
		else
		{
			SourceLoc = GetWeaponTargetingSourceLocation();
			CamLoc = SourceLoc;
			CamRot = Controller->GetControlRotation();
		}

		// 算出初始焦点位置
		FVector AimDir = CamRot.Vector().GetSafeNormal();
		FocalLoc = CamLoc + (AimDir * FocalDistance);

		// 把起点和焦点移到角色前方
		if (PC)
		{
			const FVector WeaponLoc = GetWeaponTargetingSourceLocation();
			CamLoc = FocalLoc + (((WeaponLoc - FocalLoc) | AimDir) * AimDir);
			FocalLoc = CamLoc + (AimDir * FocalDistance);
		}
		// 如果是 AI：把起点挪到 AI 头部位置
		else if (AAIController* AIController = Cast<AAIController>(Controller))
		{
			CamLoc = SourcePawn->GetActorLocation() + FVector(0, 0, SourcePawn->BaseEyeHeight);
		}

		if (Source == ETenkaichiAbilityTargetingSource::CameraTowardsFocus)
		{
			// 镜头朝焦点模式：直接用镜头朝向和位置，完成
			return FTransform(CamRot, CamLoc);
		}
	}

	if ((Source == ETenkaichiAbilityTargetingSource::WeaponForward) || (Source == ETenkaichiAbilityTargetingSource::WeaponTowardsFocus))
	{
		SourceLoc = GetWeaponTargetingSourceLocation();
	}
	else
	{
		// 其余模式：用 Pawn 位置（或拿不到镜头时）
		SourceLoc = ActorLoc;
	}

	if (bFoundFocus && ((Source == ETenkaichiAbilityTargetingSource::PawnTowardsFocus) || (Source == ETenkaichiAbilityTargetingSource::WeaponTowardsFocus)))
	{
		// 朝焦点打：返回一个指向焦点的朝向 + 起点
		return FTransform((FocalLoc - SourceLoc).Rotation(), SourceLoc);
	}

	// 兜底：没有镜头或不想要镜头，就朝 Pawn 朝向打
	return FTransform(AimQuat, SourceLoc);
}

// 单颗子弹：先尝试纯射线，没命中就回退到球体扫描（SweepRadius>0 时）
FHitResult UTenkaichiGameplayAbility_RangedWeapon::DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHits) const
{
#if ENABLE_DRAW_DEBUG
	if (TenkaichiConsoleVariables::DrawBulletTracesDuration > 0.0f)
	{
		static float DebugThickness = 1.0f;
		DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false, TenkaichiConsoleVariables::DrawBulletTracesDuration, 0, DebugThickness);
	}
#endif // ENABLE_DRAW_DEBUG

	FHitResult Impact;

	// 先打一条纯射线（SweepRadius=0）
	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		Impact = WeaponTrace(StartTrace, EndTrace, /*SweepRadius=*/ 0.0f, bIsSimulated, /*out*/ OutHits);
	}

	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		// 纯射线没命中人，且支持球体扫描 → 回退到球体扫描（更容易命中细目标）
		if (SweepRadius > 0.0f)
		{
			TArray<FHitResult> SweepHits;
			Impact = WeaponTrace(StartTrace, EndTrace, SweepRadius, bIsSimulated, /*out*/ SweepHits);

			// 球体扫描命中了 Pawn 的话，检查是否应该用它
			const int32 FirstPawnIdx = FindFirstPawnHitResult(SweepHits);
			if (SweepHits.IsValidIndex(FirstPawnIdx))
			{
				// 如果纯射线在球体命中 Pawn 之前已经撞到了墙，就保持纯射线的结果
				bool bUseSweepHits = true;
				for (int32 Idx = 0; Idx < FirstPawnIdx; ++Idx)
				{
					const FHitResult& CurHitResult = SweepHits[Idx];

					auto Pred = [&CurHitResult](const FHitResult& Other)
					{
						return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
					};
					if (CurHitResult.bBlockingHit && OutHits.ContainsByPredicate(Pred))
					{
						bUseSweepHits = false;
						break;
					}
				}

				if (bUseSweepHits)
				{
					OutHits = SweepHits;
				}
			}
		}
	}

	return Impact;
}

// 本地射线追踪：算起点/方向/终点，打出一串命中结果（只在本地控制的角色上跑）
void UTenkaichiGameplayAbility_RangedWeapon::PerformLocalTargeting(OUT TArray<FHitResult>& OutHits)
{
	APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());

	UTenkaichiRangedWeaponInstance* WeaponData = GetWeaponInstance();
	if (AvatarPawn && AvatarPawn->IsLocallyControlled() && WeaponData)
	{
		FRangedWeaponFiringInput InputData;
		InputData.WeaponData = WeaponData;
		InputData.bCanPlayBulletFX = (AvatarPawn->GetNetMode() != NM_DedicatedServer);

		//@TODO: 玩家贴墙时应该有更复杂的逻辑

		// 默认用"镜头朝焦点"模式（FPS 手感）
		const FTransform TargetTransform = GetTargetingTransform(AvatarPawn, ETenkaichiAbilityTargetingSource::CameraTowardsFocus);
		InputData.AimDir = TargetTransform.GetUnitAxis(EAxis::X);
		InputData.StartTrace = TargetTransform.GetTranslation();

		InputData.EndAim = InputData.StartTrace + InputData.AimDir * WeaponData->GetMaxDamageRange();

#if ENABLE_DRAW_DEBUG
		if (TenkaichiConsoleVariables::DrawBulletTracesDuration > 0.0f)
		{
			static float DebugThickness = 2.0f;
			DrawDebugLine(GetWorld(), InputData.StartTrace, InputData.StartTrace + (InputData.AimDir * 100.0f), FColor::Yellow, false, TenkaichiConsoleVariables::DrawBulletTracesDuration, 0, DebugThickness);
		}
#endif

		TraceBulletsInCartridge(InputData, /*out*/ OutHits);
	}
}

// 按"每弹夹子弹数"循环：每颗子弹按散布角散开，单独打一条射线
void UTenkaichiGameplayAbility_RangedWeapon::TraceBulletsInCartridge(const FRangedWeaponFiringInput& InputData, OUT TArray<FHitResult>& OutHits)
{
	UTenkaichiRangedWeaponInstance* WeaponData = InputData.WeaponData;
	check(WeaponData);

	// 这一枪打几发（散弹枪>1，步枪=1）
	const int32 BulletsPerCartridge = WeaponData->GetBulletsPerCartridge();

	for (int32 BulletIndex = 0; BulletIndex < BulletsPerCartridge; ++BulletIndex)
	{
		// 综合散布角 = 基础散布角 × 倍率（瞄准/站定/蹲下/跳跃 等）
		const float BaseSpreadAngle = WeaponData->GetCalculatedSpreadAngle();
		const float SpreadAngleMultiplier = WeaponData->GetCalculatedSpreadAngleMultiplier();
		const float ActualSpreadAngle = BaseSpreadAngle * SpreadAngleMultiplier;

		// 角度转弧度，取一半（散布是直径角，要除以 2）
		const float HalfSpreadAngleInRadians = FMath::DegreesToRadians(ActualSpreadAngle * 0.5f);

		// 在散布锥体内随机挑一个方向（指数控制聚拢程度）
		const FVector BulletDir = VRandConeNormalDistribution(InputData.AimDir, HalfSpreadAngleInRadians, WeaponData->GetSpreadExponent());

		// 这颗子弹的射线终点
		const FVector EndTrace = InputData.StartTrace + (BulletDir * WeaponData->GetMaxDamageRange());
		FVector HitLocation = EndTrace;

		TArray<FHitResult> AllImpacts;

		// 打这颗子弹的射线
		FHitResult Impact = DoSingleBulletTrace(InputData.StartTrace, EndTrace, WeaponData->GetBulletTraceSweepRadius(), /*bIsSimulated=*/ false, /*out*/ AllImpacts);

		const AActor* HitActor = Impact.GetActor();

		if (HitActor)
		{
#if ENABLE_DRAW_DEBUG
			if (TenkaichiConsoleVariables::DrawBulletHitDuration > 0.0f)
			{
				DrawDebugPoint(GetWorld(), Impact.ImpactPoint, TenkaichiConsoleVariables::DrawBulletHitRadius, FColor::Red, false, TenkaichiConsoleVariables::DrawBulletHitRadius);
			}
#endif

			// 把这次命中的所有结果并进总结果
			if (AllImpacts.Num() > 0)
			{
				OutHits.Append(AllImpacts);
			}

			HitLocation = Impact.ImpactPoint;
		}

		// 保证 OutHits 里始终有这一颗子弹的记录（用于弹道特效等）
		if (OutHits.Num() == 0)
		{
			if (!Impact.bBlockingHit)
			{
				// 没命中实体的子弹：把"落点"放在射线终点
				Impact.Location = EndTrace;
				Impact.ImpactPoint = EndTrace;
			}

			OutHits.Add(Impact);
		}
	}
}

// 激活技能：绑定目标数据回调 + 更新武器开火时间 + 交给父类
void UTenkaichiGameplayAbility_RangedWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 绑定"目标数据就绪"回调
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	OnTargetDataReadyCallbackDelegateHandle = MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReadyCallback);

	// 更新这把武器的"上次开火时间"
	UTenkaichiRangedWeaponInstance* WeaponData = GetWeaponInstance();
	check(WeaponData);
	WeaponData->UpdateFiringTime();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

// 技能结束：解绑回调 + 消费已复制的目标数据
void UTenkaichiGameplayAbility_RangedWeapon::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
			return;
		}

		UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
		check(MyAbilityComponent);

		// 技能结束时：移除回调 + 消费掉已复制的目标数据（防止泄漏）
		MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
		MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

// 目标数据就绪回调：上报服务器 + 提交技能 + 触发蓝图接口
void UTenkaichiGameplayAbility_RangedWeapon::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	if (const FGameplayAbilitySpec* AbilitySpec = MyAbilityComponent->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FScopedPredictionWindow ScopedPrediction(MyAbilityComponent);

		// 转移目标数据所有权，防止回调中别处代码改动它
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

		// 客户端需要把瞄准结果上报服务器
		const bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();
		if (bShouldNotifyServer)
		{
			MyAbilityComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), LocalTargetDataHandle, ApplicationTag, MyAbilityComponent->ScopedPredictionKey);
		}

		const bool bIsTargetDataValid = true;

		bool bProjectileWeapon = false;

#if WITH_SERVER_CODE
		if (!bProjectileWeapon)
		{
			if (AController* Controller = GetControllerFromActorInfo())
			{
				if (Controller->GetLocalRole() == ROLE_Authority)
				{
					// 服务器确认命中标记（走 P11.3）
					if (UTenkaichiWeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<UTenkaichiWeaponStateComponent>())
					{
						TArray<uint8> HitReplaces;
						for (uint8 i = 0; (i < LocalTargetDataHandle.Num()) && (i < 255); ++i)
						{
							if (FGameplayAbilityTargetData_SingleTargetHit* SingleTargetHit = static_cast<FGameplayAbilityTargetData_SingleTargetHit*>(LocalTargetDataHandle.Get(i)))
							{
								if (SingleTargetHit->bHitReplaced)
								{
									HitReplaces.Add(i);
								}
							}
						}

						WeaponStateComponent->ClientConfirmTargetData(LocalTargetDataHandle.UniqueId, bIsTargetDataValid, HitReplaces);
					}
				}
			}
		}
#endif //WITH_SERVER_CODE


		// 还有弹药且技能提交成功
		if (bIsTargetDataValid && CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			// 我们开了一枪，加散布（这把枪变"热"了）
			UTenkaichiRangedWeaponInstance* WeaponData = GetWeaponInstance();
			check(WeaponData);
			WeaponData->AddSpread();

			// 把命中的目标数据交给蓝图，让蓝图为所欲为（伤害/特效/声音）
			OnRangedWeaponTargetDataReady(LocalTargetDataHandle);
		}
		else
		{
			UE_LOG(LogTenkaichiAbilitySystem, Warning, TEXT("Weapon ability %s failed to commit (bIsTargetDataValid=%d)"), *GetPathName(), bIsTargetDataValid ? 1 : 0);
			K2_EndAbility();
		}
	}

	// 数据已处理完，消费掉
	MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

// 触发一次"瞄准→射线→回调"完整流程（蓝图可调用）
void UTenkaichiGameplayAbility_RangedWeapon::StartRangedWeaponTargeting()
{
	check(CurrentActorInfo);

	AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);

	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	AController* Controller = GetControllerFromActorInfo();
	check(Controller);
	UTenkaichiWeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<UTenkaichiWeaponStateComponent>();

	FScopedPredictionWindow ScopedPrediction(MyAbilityComponent, CurrentActivationInfo.GetActivationPredictionKey());

	// 本地打射线
	TArray<FHitResult> FoundHits;
	PerformLocalTargeting(/*out*/ FoundHits);

	// 把命中结果填进目标数据
	FGameplayAbilityTargetDataHandle TargetData;
	TargetData.UniqueId = WeaponStateComponent ? WeaponStateComponent->GetUnconfirmedServerSideHitMarkerCount() : 0;

	if (FoundHits.Num() > 0)
	{
		const int32 CartridgeID = FMath::Rand();

		for (const FHitResult& FoundHit : FoundHits)
		{
			FTenkaichiGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FTenkaichiGameplayAbilityTargetData_SingleTargetHit();
			NewTargetData->HitResult = FoundHit;
			NewTargetData->CartridgeID = CartridgeID;

			TargetData.Add(NewTargetData);
		}
	}

	// 发送命中标记信息（供准星 UI 用）
	if (WeaponStateComponent != nullptr)
	{
		WeaponStateComponent->AddUnconfirmedServerSideHitMarkers(TargetData, FoundHits);
	}

	// 立刻处理这批目标数据
	OnTargetDataReadyCallback(TargetData, FGameplayTag());
}