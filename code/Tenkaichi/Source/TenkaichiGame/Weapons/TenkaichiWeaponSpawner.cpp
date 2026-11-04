// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.


#include "TenkaichiWeaponSpawner.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Equipment/TenkaichiPickupDefinition.h"
#include "GameFramework/Pawn.h"
#include "Inventory/InventoryFragment_SetStats.h"
#include "Kismet/GameplayStatics.h"
#include "TenkaichiLogChannels.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiWeaponSpawner)

class FLifetimeProperty;
class UNiagaraSystem;
class USoundBase;
struct FHitResult;

// 构造函数：搭建这个 Actor 的组件并设默认值
ATenkaichiWeaponSpawner::ATenkaichiWeaponSpawner()
{
	// 让这个 Actor 每帧调用 Tick()（不需要可关掉提升性能）
	PrimaryActorTick.bCanEverTick = true;

	// 碰撞体直接作为根节点（Actor 的坐标原点）
	RootComponent = CollisionVolume = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionVolume"));
	CollisionVolume->InitCapsuleSize(80.f, 80.f);
	// 绑定"有东西走进碰撞体"的事件回调
	CollisionVolume->OnComponentBeginOverlap.AddDynamic(this, &ATenkaichiWeaponSpawner::OnOverlapBegin);

	// 底座：静态网格，贴在地面上的圆盘
	PadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PadMesh"));
	PadMesh->SetupAttachment(RootComponent);

	// 枪模：悬浮旋转的武器模型，挂在根节点下
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	// 默认参数
	WeaponMeshRotationSpeed = 40.0f;		// 枪每秒转 40 度
	CoolDownTime = 30.0f;					// 默认 30 秒后重生
	CheckExistingOverlapDelay = 0.25f;		// 重生后延迟 0.25 秒再检测脚下
	bIsWeaponAvailable = true;				// 初始状态：枪可以被捡
	bReplicates = true;						// 开启网络复制（联网同步必需）
}

// 游戏开始或游戏生成时调用：校验武器配置
void ATenkaichiWeaponSpawner::BeginPlay()
{
	Super::BeginPlay();

	// 有配置 → 用配置里的"重生冷却时间"覆盖默认值
	if (WeaponDefinition && WeaponDefinition->InventoryItemDefinition)
	{
		CoolDownTime = WeaponDefinition->SpawnCoolDownSeconds;
	}
	// 没有配置 → 报错提醒（回放时不算）
	else if (const UWorld* World = GetWorld())
	{
		if (!World->IsPlayingReplay())
		{
			UE_LOG(LogTenkaichi, Error, TEXT("'%s' does not have a valid weapon definition! Make sure to set this data on the instance!"), *GetNameSafe(this));
		}
	}
}

// 游戏结束或销毁时调用：清掉所有计时器，防止悬空引用
void ATenkaichiWeaponSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CoolDownTimerHandle);
		World->GetTimerManager().ClearTimer(CheckOverlapsDelayTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

// 每一帧调用：更新重生倒计时进度条 + 让枪模持续旋转
void ATenkaichiWeaponSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 更新 CoolDownPercentage，用来驱动"武器重生倒计时"显示条
	UWorld* World = GetWorld();
	if (World->GetTimerManager().IsTimerActive(CoolDownTimerHandle))
	{
		// 剩余时间 / 总时间 = 剩余比例；1 减它 = 已完成比例（0~1）
		CoolDownPercentage = 1.0f - World->GetTimerManager().GetTimerRemaining(CoolDownTimerHandle) / CoolDownTime;
	}

	// 枪围绕 Y 轴（竖轴）持续旋转，制造"悬浮转动"效果
	WeaponMesh->AddRelativeRotation(FRotator(0.0f, World->GetDeltaSeconds() * WeaponMeshRotationSpeed, 0.0f));
}

// 编辑器里摆放/改参数时实时预览武器外观（设计师所见即所得）
void ATenkaichiWeaponSpawner::OnConstruction(const FTransform& Transform)
{
	if (WeaponDefinition != nullptr && WeaponDefinition->DisplayMesh != nullptr)
	{
		// 把配置里的枪模显示出来，并套上偏移和缩放
		WeaponMesh->SetStaticMesh(WeaponDefinition->DisplayMesh);
		WeaponMesh->SetRelativeLocation(WeaponDefinition->WeaponMeshOffset);
		WeaponMesh->SetRelativeScale3D(WeaponDefinition->WeaponMeshScale);
	}
}

// 碰撞回调：有东西走进碰撞体时触发（只服务器真正处理）
void ATenkaichiWeaponSpawner::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	APawn* OverlappingPawn = Cast<APawn>(OtherActor);
	if (GetLocalRole() == ROLE_Authority && bIsWeaponAvailable && OverlappingPawn != nullptr)
	{
		AttemptPickUpWeapon(OverlappingPawn);
	}
}

// 重生后检查：枪重新出现时，是否有人正站在刷新点上
void ATenkaichiWeaponSpawner::CheckForExistingOverlaps()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, APawn::StaticClass());

	for (AActor* OverlappingActor : OverlappingActors)
	{
		AttemptPickUpWeapon(Cast<APawn>(OverlappingActor));
	}
}

// 尝试把武器发给目标玩家（C++ 默认实现，蓝图可覆盖）
void ATenkaichiWeaponSpawner::AttemptPickUpWeapon_Implementation(APawn* Pawn)
{
	// 只服务器 + 枪还"可捡" + 该 Pawn 有 GAS 组件才继续
	if (GetLocalRole() == ROLE_Authority && bIsWeaponAvailable && UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn))
	{
		TSubclassOf<UTenkaichiInventoryItemDefinition> WeaponItemDefinition = WeaponDefinition ? WeaponDefinition->InventoryItemDefinition : nullptr;
		if (WeaponItemDefinition != nullptr)
		{
			// 调用蓝图实现的 GiveWeapon，真正把武器"塞给"玩家
			if (GiveWeapon(WeaponItemDefinition, Pawn))
			{
				// 成功捡走 → 标记不可捡 + 隐藏枪 + 播拾取特效 + 开始重生倒计时
				bIsWeaponAvailable = false;
				SetWeaponPickupVisibility(false);
				PlayPickupEffects();
				StartCoolDown();
			}
		}
	}
}

// 开启"武器重生倒计时"
void ATenkaichiWeaponSpawner::StartCoolDown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CoolDownTimerHandle, this, &ATenkaichiWeaponSpawner::OnCoolDownTimerComplete, CoolDownTime);
	}
}

// 冷却结束：重置为"可捡"，重新显示枪 + 播特效 + 延迟检测脚下（蓝图可调用）
void ATenkaichiWeaponSpawner::ResetCoolDown()
{
	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(CoolDownTimerHandle);
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		bIsWeaponAvailable = true;
		PlayRespawnEffects();
		SetWeaponPickupVisibility(true);

		if (World)
		{
			World->GetTimerManager().SetTimer(CheckOverlapsDelayTimerHandle, this, &ATenkaichiWeaponSpawner::CheckForExistingOverlaps, CheckExistingOverlapDelay);
		}
	}

	// 进度条归零
	CoolDownPercentage = 0.0f;
}

// 计时器到点回调：进入重生流程
void ATenkaichiWeaponSpawner::OnCoolDownTimerComplete()
{
	ResetCoolDown();
}

// 显示/隐藏枪模型（捡走时隐藏，重生时显示）
void ATenkaichiWeaponSpawner::SetWeaponPickupVisibility(bool bShouldBeVisible)
{
	WeaponMesh->SetVisibility(bShouldBeVisible, true);
}

// 播放"枪被捡走"的音效 + 特效（C++ 默认实现，蓝图可覆盖）
void ATenkaichiWeaponSpawner::PlayPickupEffects_Implementation()
{
	if (WeaponDefinition != nullptr)
	{
		USoundBase* PickupSound = WeaponDefinition->PickedUpSound;
		if (PickupSound != nullptr)
		{
			// 在 Actor 所在位置播放拾取音效
			UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
		}

		UNiagaraSystem* PickupEffect = WeaponDefinition->PickedUpEffect;
		if (PickupEffect != nullptr)
		{
			// 在枪模位置生成拾取粒子特效
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffect, WeaponMesh->GetComponentLocation());
		}
	}
}

// 播放"枪重新出现"的音效 + 特效（C++ 默认实现，蓝图可覆盖）
void ATenkaichiWeaponSpawner::PlayRespawnEffects_Implementation()
{
	if (WeaponDefinition != nullptr)
	{
		USoundBase* RespawnSound = WeaponDefinition->RespawnedSound;
		if (RespawnSound != nullptr)
		{
			// 在 Actor 位置播放重生音效
			UGameplayStatics::PlaySoundAtLocation(this, RespawnSound, GetActorLocation());
		}

		UNiagaraSystem* RespawnEffect = WeaponDefinition->RespawnedEffect;
		if (RespawnEffect != nullptr)
		{
			// 在枪模位置生成重生粒子特效
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, RespawnEffect, WeaponMesh->GetComponentLocation());
		}
	}
}

// 服务器复制"可捡状态"到客户端时，客户端调用此回调
void ATenkaichiWeaponSpawner::OnRep_WeaponAvailability()
{
	if (bIsWeaponAvailable)
	{
		// 变成"可捡"：播重生特效 + 重新显示枪
		PlayRespawnEffects();
		SetWeaponPickupVisibility(true);
	}
	else
	{
		// 变成"不可捡"：隐藏枪 + 开始重生倒计时 + 播拾取特效
		SetWeaponPickupVisibility(false);
		StartCoolDown();
		PlayPickupEffects();
	}
}

// 网络复制：声明哪些属性需要复制到客户端
void ATenkaichiWeaponSpawner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// "武器是否可捡"状态复制到所有客户端
	DOREPLIFETIME(ATenkaichiWeaponSpawner, bIsWeaponAvailable);
}

// 静态工具：在某个物品定义里查找匹配的属性并返回它的值，找不到就返回 0
int32 ATenkaichiWeaponSpawner::GetDefaultStatFromItemDef(const TSubclassOf<UTenkaichiInventoryItemDefinition> WeaponItemClass, FGameplayTag StatTag)
{
	if (WeaponItemClass != nullptr)
	{
		if (UTenkaichiInventoryItemDefinition* WeaponItemCDO = WeaponItemClass->GetDefaultObject<UTenkaichiInventoryItemDefinition>())
		{
			if (const UInventoryFragment_SetStats* ItemStatsFragment = Cast<UInventoryFragment_SetStats>(WeaponItemCDO->FindFragmentByClass(UInventoryFragment_SetStats::StaticClass())))
			{
				return ItemStatsFragment->GetItemStatByTag(StatTag);
			}
		}
	}

	return 0;
}