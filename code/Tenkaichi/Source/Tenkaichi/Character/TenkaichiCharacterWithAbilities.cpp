#include "Character/TenkaichiCharacterWithAbilities.h"

#include "AbilitySystem/Attributes/TenkaichiHealthSet.h"
#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"

#include "Input/TenkaichiInputComponent.h"
#include "Input/TenkaichiInputConfig.h"

#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiCharacterWithAbilities)

ATenkaichiCharacterWithAbilities::ATenkaichiCharacterWithAbilities(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UTenkaichiAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
	HealthSet = CreateDefaultSubobject<UTenkaichiHealthSet>(TEXT("HealthSet"));

	// AbilitySystemComponent needs to be updated at a high frequency.
	SetNetUpdateFrequency(100.0f);
}

void ATenkaichiCharacterWithAbilities::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent* ATenkaichiCharacterWithAbilities::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


void ATenkaichiCharacterWithAbilities::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 把引擎的输入组件转成我们自己那个（继承自 UEnhancedInputComponent）
	UTenkaichiInputComponent* TenkaichiIC = Cast<UTenkaichiInputComponent>(PlayerInputComponent);
	if (ensureMsgf(TenkaichiIC, TEXT("输入组件类型不对！应该用 UTenkaichiInputComponent 或其子类。")))
	{
		// ===== 注册映射上下文：让「按键 → InputAction」这张表生效 =====
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			// 从控制器拿本地玩家，再从本地玩家身上拿「输入子系统」
			if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
				{
					// 先清空旧映射，再注册我们自己的 IMC（优先级 0 即可，只有一个）
					Subsystem->ClearAllMappings();
					Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
				}
			}
		}

		// 用「字典」把 Tag 查到 InputAction，再绑定到回调（只认 Tag，不认按键）
		TenkaichiIC->BindNativeAction(InputConfig, FGameplayTag::RequestGameplayTag(TEXT("InputTag.Move")),       ETriggerEvent::Triggered, this, &ThisClass::Input_Move,     /*bLogIfNotFound=*/ false);
		TenkaichiIC->BindNativeAction(InputConfig, FGameplayTag::RequestGameplayTag(TEXT("InputTag.Look.Mouse")), ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
	}
}

void ATenkaichiCharacterWithAbilities::Input_Move(const FInputActionValue& InputActionValue)
{
	// 角色类自己就是 Pawn，直接用 this；控制器用 GetController()（AActor 的方法）
	AController* Controller = GetController();

	if (Controller)
	{
		// 取出二维输入值：X = 左右，Y = 前后
		const FVector2D Value = InputActionValue.Get<FVector2D>();

		// 用「视角朝向的偏航角」当基准，把「前后左右」换算成世界方向
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			// 右向量绕视角旋转 → 得到「右」在世界里的方向，按 X 缩放
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			// 前向量绕视角旋转 → 得到「前」在世界里的方向，按 Y 缩放
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void ATenkaichiCharacterWithAbilities::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	// 取出二维输入值：X = 鼠标左右，Y = 鼠标上下
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		// 左右晃鼠标 → 偏航（左右转头）
		AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		// 上下晃鼠标 → 俯仰（抬头低头）
		AddControllerPitchInput(Value.Y);
	}
}