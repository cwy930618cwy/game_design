// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiCharacter.h"

#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "Camera/TenkaichiCameraComponent.h"
#include "Character/TenkaichiHealthComponent.h"
#include "Character/TenkaichiPawnExtensionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TenkaichiCharacterMovementComponent.h"
#include "TenkaichiGameplayTags.h"
#include "TenkaichiLogChannels.h"
#include "Net/UnrealNetwork.h"
#include "Player/TenkaichiPlayerController.h"
#include "Player/TenkaichiPlayerState.h"
// #include "System/TenkaichiSignificanceManager.h"   // ⚠️ P14 未就位（挂起 ㉙，见 BeginPlay/EndPlay）
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiCharacter)

class AActor;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UInputComponent;

// 角色胶囊体的碰撞配置名（要到工程 ini 里定义这个 Profile）
static FName NAME_TenkaichiCharacterCollisionProfile_Capsule(TEXT("TenkaichiPawnCapsule"));
// 角色网格的碰撞配置名（同上）
static FName NAME_TenkaichiCharacterCollisionProfile_Mesh(TEXT("TenkaichiPawnMesh"));

// 角色构造函数：负责把"身体零件"（胶囊/网格/移动/组件）全装好
ATenkaichiCharacter::ATenkaichiCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UTenkaichiCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// 尽可能不 tick 角色（省性能）
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// 网络剔除距离（900,000,000 平方单位，约 3 万单位半径）
	SetNetCullDistanceSquared(900000000.0f);

	// ---- 胶囊体 ----
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);	// 半径 40，半高 90
	CapsuleComp->SetCollisionProfileName(NAME_TenkaichiCharacterCollisionProfile_Capsule);

	// ---- 骨骼网格 ----
	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));  // 网格默认 Y 向前，转成 X 向前
	MeshComp->SetCollisionProfileName(NAME_TenkaichiCharacterCollisionProfile_Mesh);

	// ---- 角色移动组件参数（用我们的移动组件） ----
	UTenkaichiCharacterMovementComponent* TenkaichiMoveComp = CastChecked<UTenkaichiCharacterMovementComponent>(GetCharacterMovement());
	TenkaichiMoveComp->GravityScale = 1.0f;		// 重力比例
	TenkaichiMoveComp->MaxAcceleration = 2400.0f;	// 最大加速度
	TenkaichiMoveComp->BrakingFrictionFactor = 1.0f;	// 制动摩擦系数
	TenkaichiMoveComp->BrakingFriction = 6.0f;	// 制动摩擦
	TenkaichiMoveComp->GroundFriction = 8.0f;	// 地面摩擦
	TenkaichiMoveComp->BrakingDecelerationWalking = 1400.0f;	// 走路制动减速度
	TenkaichiMoveComp->bUseControllerDesiredRotation = false;	// 不跟控制器期望旋转
	TenkaichiMoveComp->bOrientRotationToMovement = false;	// 不朝移动方向转向
	TenkaichiMoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);	// 每秒 720 度旋转速度
	TenkaichiMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;	// 动画根运动期间不允许物理旋转
	TenkaichiMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;	// 允许蹲伏
	TenkaichiMoveComp->bCanWalkOffLedgesWhenCrouching = true;	// 蹲伏时允许走下悬崖
	TenkaichiMoveComp->SetCrouchedHalfHeight(65.0f);	// 蹲伏半高 65

	// ---- Pawn 扩展组件（4.6 教过）----
	PawnExtComponent = CreateDefaultSubobject<UTenkaichiPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	// 注册 GAS 初始化的回调（ASC 就绪时触发 OnAbilitySystemInitialized）
	PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	// 注册 GAS 卸载的回调
	PawnExtComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

	// ---- 生命组件（GAS 断点，挂起）----
	HealthComponent = CreateDefaultSubobject<UTenkaichiHealthComponent>(TEXT("HealthComponent"));
	// 死亡开始/结束都绑到回调
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);

	// ---- 相机组件（GAS 断点，挂起）----
	CameraComponent = CreateDefaultSubobject<UTenkaichiCameraComponent>(TEXT("CameraComponent"));
	// 相机相对角色位置（身后 300，高 75）
	CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));

	// ---- 旋转控制 ----
	bUseControllerRotationPitch = false;	// 俯仰不跟控制器
	bUseControllerRotationYaw = true;	// 偏航跟控制器（角色朝向 = 鼠标方向）
	bUseControllerRotationRoll = false;	// 翻滚不跟控制器

	// ---- 眼高 ----
	BaseEyeHeight = 80.0f;		// 站着眼高
	CrouchedEyeHeight = 50.0f;	// 蹲伏眼高
}

// 组件初始化前（当前为空，留扩展点）
void ATenkaichiCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

// 角色开始播放：非专用服务器时向 SignificanceManager 注册自己（当前注册逻辑是 TODO 占位）
void ATenkaichiCharacter::BeginPlay()
{
	Super::BeginPlay();

#if 0   // ⚠️ 挂起 ㉙：依赖 P14 UTenkaichiSignificanceManager
	UWorld* World = GetWorld();

	// 专用服务器不注册（不需要"重要度管理"）
	const bool bRegisterWithSignificanceManager = !IsNetMode(NM_DedicatedServer);
	if (bRegisterWithSignificanceManager)
	{
		if (UTenkaichiSignificanceManager* SignificanceManager = USignificanceManager::Get<UTenkaichiSignificanceManager>(World))
		{
			//@TODO: SignificanceManager->RegisterObject(this, (EFortSignificanceType)SignificanceType);
		}
	}
#endif
}

// 角色结束播放：从 SignificanceManager 注销
void ATenkaichiCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

#if 0   // ⚠️ 挂起 ㉙：依赖 P14 UTenkaichiSignificanceManager
	UWorld* World = GetWorld();

	// 专用服务器没注册过，也不用注销
	const bool bRegisterWithSignificanceManager = !IsNetMode(NM_DedicatedServer);
	if (bRegisterWithSignificanceManager)
	{
		if (UTenkaichiSignificanceManager* SignificanceManager = USignificanceManager::Get<UTenkaichiSignificanceManager>(World))
		{
			SignificanceManager->UnregisterObject(this);
		}
	}
#endif
}

// 重置角色：关移动碰撞 → 蓝图钩子 → 反初始化并销毁
void ATenkaichiCharacter::Reset()
{
	DisableMovementAndCollision();

	K2_OnReset();

	UninitAndDestroy();
}

// 注册复制属性：加速度（仅模拟端）+ 队伍 ID
void ATenkaichiCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedAcceleration, COND_SimulatedOnly);	// 加速度只复制给模拟端
	DOREPLIFETIME(ThisClass, MyTeamID)	// 队伍 ID 全端复制
}

// 复制前钩子：把当前加速度压缩进 3 字节（FastShared 的输入）
void ATenkaichiCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		// 压缩加速度：XY 压成"方向角 + 大小"，Z 压成直接值
		const double MaxAccel = MovementComponent->MaxAcceleration;
		const FVector CurrentAccel = MovementComponent->GetCurrentAcceleration();
		double AccelXYRadians, AccelXYMagnitude;
		FMath::CartesianToPolar(CurrentAccel.X, CurrentAccel.Y, AccelXYMagnitude, AccelXYRadians);	// 直角坐标→极坐标

		ReplicatedAcceleration.AccelXYRadians   = FMath::FloorToInt((AccelXYRadians / TWO_PI) * 255.0);     // [0, 2PI] -> [0, 255]
		ReplicatedAcceleration.AccelXYMagnitude = FMath::FloorToInt((AccelXYMagnitude / MaxAccel) * 255.0);	// [0, MaxAccel] -> [0, 255]
		ReplicatedAcceleration.AccelZ           = FMath::FloorToInt((CurrentAccel.Z / MaxAccel) * 127.0);   // [-MaxAccel, MaxAccel] -> [-127, 127]
	}
}

// 控制器变化：从控制器同步队伍 ID（仅权威端）
void ATenkaichiCharacter::NotifyControllerChanged()
{
	const FGenericTeamId OldTeamId = GetGenericTeamId();

	Super::NotifyControllerChanged();

	// 权威端 + 有控制器时，从控制器抄队伍 ID
	if (HasAuthority() && (GetController() != nullptr))
	{
		if (ITenkaichiTeamAgentInterface* ControllerWithTeam = Cast<ITenkaichiTeamAgentInterface>(GetController()))
		{
			MyTeamID = ControllerWithTeam->GetGenericTeamId();
			ConditionalBroadcastTeamChanged(this, OldTeamId, MyTeamID);
		}
	}
}

// 接管（服务器）：抄控制器队伍 + 监听控制器换队
void ATenkaichiCharacter::PossessedBy(AController* NewController)
{
	const FGenericTeamId OldTeamID = MyTeamID;

	Super::PossessedBy(NewController);

	// 通知 Pawn 扩展组件控制器变了
	PawnExtComponent->HandleControllerChanged();

	// 从新控制器拿队伍 ID + 监听它以后的换队
	if (ITenkaichiTeamAgentInterface* ControllerAsTeamProvider = Cast<ITenkaichiTeamAgentInterface>(NewController))
	{
		MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	}
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

// 释放（服务器）：解绑控制器换队监听 + 按策略决定新队伍
void ATenkaichiCharacter::UnPossessed()
{
	AController* const OldController = GetController();

	// 先记录旧队伍，并停止监听旧控制器的换队
	const FGenericTeamId OldTeamID = MyTeamID;
	if (ITenkaichiTeamAgentInterface* ControllerAsTeamProvider = Cast<ITenkaichiTeamAgentInterface>(OldController))
	{
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	Super::UnPossessed();

	// 通知 Pawn 扩展组件控制器变了
	PawnExtComponent->HandleControllerChanged();

	// 释放后队伍去哪？交给策略钩子决定（默认 NoTeam）
	MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

// 控制器复制到客户端：通知 Pawn 扩展组件
void ATenkaichiCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtComponent->HandleControllerChanged();
}

// PlayerState 复制到客户端：通知 Pawn 扩展组件
void ATenkaichiCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	PawnExtComponent->HandlePlayerStateReplicated();
}

// 拿本项目类型 PlayerController（允许为空）
ATenkaichiPlayerController* ATenkaichiCharacter::GetTenkaichiPlayerController() const
{
	return CastChecked<ATenkaichiPlayerController>(GetController(), ECastCheckedType::NullAllowed);
}

// 拿本项目类型 PlayerState（允许为空）
ATenkaichiPlayerState* ATenkaichiCharacter::GetTenkaichiPlayerState() const
{
	return CastChecked<ATenkaichiPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

// 拿本项目类型 ASC（返回 GAS 通用 ASC 强转）
UTenkaichiAbilitySystemComponent* ATenkaichiCharacter::GetTenkaichiAbilitySystemComponent() const
{
	return Cast<UTenkaichiAbilitySystemComponent>(GetAbilitySystemComponent());
}

// 覆盖 IAbilitySystemInterface：从 Pawn 扩展组件拿 ASC（返回通用接口类型）
UAbilitySystemComponent* ATenkaichiCharacter::GetAbilitySystemComponent() const
{
	if (PawnExtComponent == nullptr)
	{
		return nullptr;
	}

	return PawnExtComponent->GetTenkaichiAbilitySystemComponent();
}

// GAS 初始化完成：把 ASC 交给生命组件，并初始化移动标签
void ATenkaichiCharacter::OnAbilitySystemInitialized()
{
	UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent();
	check(TenkaichiASC);

	// 把 ASC 交给生命组件（生命值、死亡都由 GAS 驱动）
	HealthComponent->InitializeWithAbilitySystem(TenkaichiASC);

	// 根据当前移动模式打上对应 GameplayTag
	InitializeGameplayTags();
}

// GAS 卸载：生命组件与 ASC 解绑
void ATenkaichiCharacter::OnAbilitySystemUninitialized()
{
	HealthComponent->UninitializeFromAbilitySystem();
}

// 绑定玩家输入：转交给 Pawn 扩展组件处理
void ATenkaichiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PawnExtComponent->SetupPlayerInputComponent();
}

// 初始化移动标签：先清掉残留标签，再按当前移动模式打标签
void ATenkaichiCharacter::InitializeGameplayTags()
{
	// 清掉上一个 Pawn 可能在 ASC 上残留的标签
	if (UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent())
	{
		for (const TPair<uint8, FGameplayTag>& TagMapping : TenkaichiGameplayTags::MovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				TenkaichiASC->SetLooseGameplayTagCount(TagMapping.Value, 0);	// 计数清零
			}
		}

		for (const TPair<uint8, FGameplayTag>& TagMapping : TenkaichiGameplayTags::CustomMovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				TenkaichiASC->SetLooseGameplayTagCount(TagMapping.Value, 0);	// 计数清零
			}
		}

		// 按当前移动模式打上对应标签
		UTenkaichiCharacterMovementComponent* TenkaichiMoveComp = CastChecked<UTenkaichiCharacterMovementComponent>(GetCharacterMovement());
		SetMovementModeTag(TenkaichiMoveComp->MovementMode, TenkaichiMoveComp->CustomMovementMode, true);
	}
}

// 把角色身上所有标签塞进 TagContainer
void ATenkaichiCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent())
	{
		TenkaichiASC->GetOwnedGameplayTags(TagContainer);
	}
}

// 是否拥有某个标签
bool ATenkaichiCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (const UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent())
	{
		return TenkaichiASC->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

// 是否拥有容器里全部标签
bool ATenkaichiCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent())
	{
		return TenkaichiASC->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

// 是否拥有容器里任意一个标签
bool ATenkaichiCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent())
	{
		return TenkaichiASC->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}

// 掉出世界：直接自杀（交给生命组件的自毁逻辑）
void ATenkaichiCharacter::FellOutOfWorld(const class UDamageType& dmgType)
{
	HealthComponent->DamageSelfDestruct(/*bFellOutOfWorld=*/ true);
}

// 死亡开始：关闭移动和碰撞（尸体躺下，不再移动）
void ATenkaichiCharacter::OnDeathStarted(AActor*)
{
	DisableMovementAndCollision();
}

// 死亡结束：下一帧销毁角色
void ATenkaichiCharacter::OnDeathFinished(AActor*)
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::DestroyDueToDeath);
}

// 工具①：关掉输入 + 关碰撞 + 停移动
void ATenkaichiCharacter::DisableMovementAndCollision()
{
	if (GetController())
	{
		GetController()->SetIgnoreMoveInput(true);	// 忽略移动输入
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 关胶囊碰撞
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);	// 对所有通道都忽略

	UTenkaichiCharacterMovementComponent* TenkaichiMoveComp = CastChecked<UTenkaichiCharacterMovementComponent>(GetCharacterMovement());
	TenkaichiMoveComp->StopMovementImmediately();	// 立刻停住
	TenkaichiMoveComp->DisableMovement();	// 禁用移动
}

// 工具②：触发蓝图死亡钩子 + 反初始化销毁
void ATenkaichiCharacter::DestroyDueToDeath()
{
	K2_OnDeathFinished();	// 蓝图可以在这做"死亡表现"

	UninitAndDestroy();
}

// 工具③：断开控制器 + 反初始化 ASC + 隐藏
void ATenkaichiCharacter::UninitAndDestroy()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();	// 断开控制器
		SetLifeSpan(0.1f);	// 0.1 秒后销毁
	}

	// 如果自己还是 ASC 的 avatar 角色，就反初始化（否则别的 Pawn 接管时已处理）
	if (UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent())
	{
		if (TenkaichiASC->GetAvatarActor() == this)
		{
			PawnExtComponent->UninitializeAbilitySystem();
		}
	}

	SetActorHiddenInGame(true);	// 隐藏角色
}

// 移动模式变化：旧模式摘标签，新模式挂标签
void ATenkaichiCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	UTenkaichiCharacterMovementComponent* TenkaichiMoveComp = CastChecked<UTenkaichiCharacterMovementComponent>(GetCharacterMovement());

	SetMovementModeTag(PrevMovementMode, PreviousCustomMode, false);	// 旧模式摘标签
	SetMovementModeTag(TenkaichiMoveComp->MovementMode, TenkaichiMoveComp->CustomMovementMode, true);	// 新模式挂标签
}

// 给指定移动模式挂/摘标签（按查表找标签，改松散标签计数）
void ATenkaichiCharacter::SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled)
{
	if (UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent())
	{
		const FGameplayTag* MovementModeTag = nullptr;
		if (MovementMode == MOVE_Custom)
		{
			// 自定义模式走自定义查表
			MovementModeTag = TenkaichiGameplayTags::CustomMovementModeTagMap.Find(CustomMovementMode);
		}
		else
		{
			// 普通模式走普通查表
			MovementModeTag = TenkaichiGameplayTags::MovementModeTagMap.Find(MovementMode);
		}

		if (MovementModeTag && MovementModeTag->IsValid())
		{
			TenkaichiASC->SetLooseGameplayTagCount(*MovementModeTag, (bTagEnabled ? 1 : 0));	// 开=1，关=0
		}
	}
}

// 切换蹲伏/站起
void ATenkaichiCharacter::ToggleCrouch()
{
	const UTenkaichiCharacterMovementComponent* TenkaichiMoveComp = CastChecked<UTenkaichiCharacterMovementComponent>(GetCharacterMovement());

	if (IsCrouched() || TenkaichiMoveComp->bWantsToCrouch)
	{
		UnCrouch();	// 已蹲或想蹲 → 站起
	}
	else if (TenkaichiMoveComp->IsMovingOnGround())
	{
		Crouch();	// 在地面 → 蹲下
	}
}

// 开始蹲伏：挂上"蹲伏中"标签
void ATenkaichiCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent())
	{
		TenkaichiASC->SetLooseGameplayTagCount(TenkaichiGameplayTags::Status_Crouching, 1);
	}

	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

// 结束蹲伏：摘掉"蹲伏中"标签
void ATenkaichiCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent())
	{
		TenkaichiASC->SetLooseGameplayTagCount(TenkaichiGameplayTags::Status_Crouching, 0);
	}

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

// 能否跳跃：和 ACharacter 一样，但去掉蹲伏检查（蹲着也能跳）
bool ATenkaichiCharacter::CanJumpInternal_Implementation() const
{
	// 与 ACharacter 的实现相同，但不做蹲伏检查
	return JumpIsAllowedInternal();
}

// 客户端收到压缩加速度：解压回浮点加速度，喂给移动组件
void ATenkaichiCharacter::OnRep_ReplicatedAcceleration()
{
	if (UTenkaichiCharacterMovementComponent* TenkaichiMovementComponent = Cast<UTenkaichiCharacterMovementComponent>(GetCharacterMovement()))
	{
		// 解压加速度：把 3 字节量化值还原成浮点
		const double MaxAccel         = TenkaichiMovementComponent->MaxAcceleration;
		const double AccelXYMagnitude = double(ReplicatedAcceleration.AccelXYMagnitude) * MaxAccel / 255.0; // [0, 255] -> [0, MaxAccel]
		const double AccelXYRadians   = double(ReplicatedAcceleration.AccelXYRadians) * TWO_PI / 255.0;     // [0, 255] -> [0, 2PI]

		FVector UnpackedAcceleration(FVector::ZeroVector);
		FMath::PolarToCartesian(AccelXYMagnitude, AccelXYRadians, UnpackedAcceleration.X, UnpackedAcceleration.Y);	// 极坐标→直角坐标
		UnpackedAcceleration.Z = double(ReplicatedAcceleration.AccelZ) * MaxAccel / 127.0; // [-127, 127] -> [-MaxAccel, MaxAccel]

		TenkaichiMovementComponent->SetReplicatedAcceleration(UnpackedAcceleration);
	}
}

// 外部设队：仅当没有控制器 + 权威端时允许
void ATenkaichiCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (GetController() == nullptr)
	{
		if (HasAuthority())
		{
			const FGenericTeamId OldTeamID = MyTeamID;
			MyTeamID = NewTeamID;
			ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
		}
		else
		{
			UE_LOG(LogTenkaichiTeams, Error, TEXT("You can't set the team ID on a character (%s) except on the authority"), *GetPathNameSafe(this));
		}
	}
	else
	{
		UE_LOG(LogTenkaichiTeams, Error, TEXT("You can't set the team ID on a possessed character (%s); it's driven by the associated controller"), *GetPathNameSafe(this));
	}
}

// 返回当前队伍 ID
FGenericTeamId ATenkaichiCharacter::GetGenericTeamId() const
{
	return MyTeamID;
}

// 返回队伍变更广播委托的指针
FOnTenkaichiTeamIndexChangedDelegate* ATenkaichiCharacter::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

// 控制器换队回调：同步自己的队伍 + 广播
void ATenkaichiCharacter::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	const FGenericTeamId MyOldTeamID = MyTeamID;
	MyTeamID = IntegerToGenericTeamId(NewTeam);
	ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
}

// 客户端收到队伍 ID 复制：本地广播
void ATenkaichiCharacter::OnRep_MyTeamID(FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

// 判断这次要不要发共享移动更新（内容变了才发）
bool ATenkaichiCharacter::UpdateSharedReplication()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		FTenkaichiSharedRepMovement SharedMovement;
		if (SharedMovement.FillForCharacter(this))
		{
			// 只有数据跟上一次不同才广播；相同就复用上一次的 bunch（给新加入的客户端补发）
			if (!SharedMovement.Equals(LastSharedReplication, this))
			{
				LastSharedReplication = SharedMovement;
				SetReplicatedMovementMode(SharedMovement.RepMovementMode);

				FastSharedReplication(SharedMovement);	// NetMulticast 广播给所有人
			}
			return true;
		}
	}

	// 现在没法快速复制，不发
	return false;
}

// NetMulticast 广播的接收端：模拟端用它应用移动状态
void ATenkaichiCharacter::FastSharedReplication_Implementation(const FTenkaichiSharedRepMovement& SharedRepMovement)
{
	if (GetWorld()->IsPlayingReplay())	// 回放时不处理
	{
		return;
	}

	// 时间戳用于拒绝过期的移动
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		// 时间戳
		SetReplicatedServerLastTransformUpdateTimeStamp(SharedRepMovement.RepTimeStamp);

		// 移动模式
		if (GetReplicatedMovementMode() != SharedRepMovement.RepMovementMode)
		{
			SetReplicatedMovementMode(SharedRepMovement.RepMovementMode);
			GetCharacterMovement()->bNetworkMovementModeChanged = true;
			GetCharacterMovement()->bNetworkUpdateReceived = true;
		}

		// 位置 / 旋转 / 速度 等
		FRepMovement& MutableRepMovement = GetReplicatedMovement_Mutable();
		MutableRepMovement = SharedRepMovement.RepMovement;

		// 这也更新了 LastRepMovement
		OnRep_ReplicatedMovement();

		// 跳跃力
		SetProxyIsJumpForceApplied(SharedRepMovement.bProxyIsJumpForceApplied);

		// 蹲伏
		if (IsCrouched() != SharedRepMovement.bIsCrouched)
		{
			SetIsCrouched(SharedRepMovement.bIsCrouched);
			OnRep_IsCrouched();
		}
	}
}

// 共享移动包构造函数：默认位置量化到两位小数（省带宽）
FTenkaichiSharedRepMovement::FTenkaichiSharedRepMovement()
{
	RepMovement.LocationQuantizationLevel = EVectorQuantization::RoundTwoDecimals;
}

// 从一个 ACharacter 填满这份共享移动包（取它的移动状态）
bool FTenkaichiSharedRepMovement::FillForCharacter(ACharacter* Character)
{
	if (USceneComponent* PawnRootComponent = Character->GetRootComponent())
	{
		UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();

		RepMovement.Location = FRepMovement::RebaseOntoZeroOrigin(PawnRootComponent->GetComponentLocation(), Character);	// 位置重定基到零点
		RepMovement.Rotation = PawnRootComponent->GetComponentRotation();	// 旋转
		RepMovement.LinearVelocity = CharacterMovement->Velocity;	// 线速度
		RepMovementMode = CharacterMovement->PackNetworkMovementMode();	// 打包移动模式
		bProxyIsJumpForceApplied = Character->GetProxyIsJumpForceApplied() || (Character->JumpForceTimeRemaining > 0.0f);	// 是否跳跃力生效
		bIsCrouched = Character->IsCrouched();	// 是否蹲伏

		// 时间戳：线性平滑或强制传时间戳时才带，否则 0
		if ((CharacterMovement->NetworkSmoothingMode == ENetworkSmoothingMode::Linear) || CharacterMovement->bNetworkAlwaysReplicateTransformUpdateTimestamp)
		{
			RepTimeStamp = CharacterMovement->GetServerLastTransformUpdateTimeStamp();
		}
		else
		{
			RepTimeStamp = 0.f;
		}

		return true;
	}
	return false;
}

// 比较两次共享移动包：任何一项不同就返回 false（要重新广播）
bool FTenkaichiSharedRepMovement::Equals(const FTenkaichiSharedRepMovement& Other, ACharacter* Character) const
{
	if (RepMovement.Location != Other.RepMovement.Location)
	{
		return false;
	}

	if (RepMovement.Rotation != Other.RepMovement.Rotation)
	{
		return false;
	}

	if (RepMovement.LinearVelocity != Other.RepMovement.LinearVelocity)
	{
		return false;
	}

	if (RepMovementMode != Other.RepMovementMode)
	{
		return false;
	}

	if (bProxyIsJumpForceApplied != Other.bProxyIsJumpForceApplied)
	{
		return false;
	}

	if (bIsCrouched != Other.bIsCrouched)
	{
		return false;
	}

	return true;	// 全相同 → 不用重新广播
}

// 自定义网络序列化：把共享移动包压进网络流
bool FTenkaichiSharedRepMovement::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	RepMovement.NetSerialize(Ar, Map, bOutSuccess);	// 先序列化 RepMovement
	Ar << RepMovementMode;	// 移动模式
	Ar << bProxyIsJumpForceApplied;	// 跳跃力
	Ar << bIsCrouched;	// 蹲伏

	// 时间戳：非零才传 1 位标志 + 值，为零只传标志
	uint8 bHasTimeStamp = (RepTimeStamp != 0.f);
	Ar.SerializeBits(&bHasTimeStamp, 1);
	if (bHasTimeStamp)
	{
		Ar << RepTimeStamp;
	}
	else
	{
		RepTimeStamp = 0.f;
	}

	return true;
}

