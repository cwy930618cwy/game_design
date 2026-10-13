// Copyright Epic Games, Inc. All Rights Reserved.

#if 0	// ⚠️ 挂起：硬依赖 GAS/Input/Camera/Player/Character 全线，等 L5 GAS 核心 + GameFeatures 就位后放开

#include "TenkaichiHeroComponent.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Logging/MessageLog.h"
#include "TenkaichiLogChannels.h"
#include "EnhancedInputSubsystems.h"
#include "Player/TenkaichiPlayerController.h"
#include "Player/TenkaichiPlayerState.h"
#include "Player/TenkaichiLocalPlayer.h"
#include "Character/TenkaichiPawnExtensionComponent.h"
#include "Character/TenkaichiPawnData.h"
#include "Character/TenkaichiCharacter.h"
#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "Input/TenkaichiInputConfig.h"
#include "Input/TenkaichiInputComponent.h"
#include "Camera/TenkaichiCameraComponent.h"
#include "TenkaichiGameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"
#include "PlayerMappableInputConfig.h"
#include "Camera/TenkaichiCameraMode.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "InputMappingContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiHeroComponent)

#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif	// WITH_EDITOR

namespace TenkaichiHero
{
	static const float LookYawRate = 300.0f;
	static const float LookPitchRate = 165.0f;
};

// 特性名字 + "可绑输入"事件名
const FName UTenkaichiHeroComponent::NAME_BindInputsNow("BindInputsNow");
const FName UTenkaichiHeroComponent::NAME_ActorFeatureName("Hero");

// 构造函数：初始化相机覆盖为空、输入未绑定
UTenkaichiHeroComponent::UTenkaichiHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilityCameraMode = nullptr;
	bReadyToBindInputs = false;
}

// 注册时：校验必须挂在 Pawn 上（否则蓝图报错），是 Pawn 才注册到初始化状态机
void UTenkaichiHeroComponent::OnRegister()
{
	Super::OnRegister();

	if (!GetPawn<APawn>())
	{
		UE_LOG(LogTenkaichi, Error, TEXT("[UTenkaichiHeroComponent::OnRegister] This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint."));

#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("TenkaichiHeroComponent", "NotOnPawnError", "has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint. This will cause a crash if you PIE!");
			static const FName HeroMessageLogName = TEXT("TenkaichiHeroComponent");
			
			FMessageLog(HeroMessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));
				
			FMessageLog(HeroMessageLogName).Open();
		}
#endif
	}
	else
	{
		// 尽早注册到初始化状态系统（仅游戏世界才有效）
		RegisterInitStateFeature();
	}
}

// 状态机核心：判断能否从当前状态进入目标状态
bool UTenkaichiHeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() && DesiredState == TenkaichiGameplayTags::InitState_Spawned)
	{
		// 只要在合法 Pawn 上，就算作已生成
		if (Pawn)
		{
			return true;
		}
	}
	else if (CurrentState == TenkaichiGameplayTags::InitState_Spawned && DesiredState == TenkaichiGameplayTags::InitState_DataAvailable)
	{
		// 玩家状态是必需的。
		if (!GetPlayerState<ATenkaichiPlayerState>())
		{
			return false;
		}

		// 如果我们是权威端或自主端，需要等一个已配对 PlayerState 所有权的控制器。
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = GetController<AController>();

			const bool bHasControllerPairedWithPS = (Controller != nullptr) && \
				(Controller->PlayerState != nullptr) && \
				(Controller->PlayerState->GetOwner() == Controller);

			if (!bHasControllerPairedWithPS)
			{
				return false;
			}
		}

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();

		if (bIsLocallyControlled && !bIsBot)
		{
			ATenkaichiPlayerController* TenkaichiPC = GetController<ATenkaichiPlayerController>();

			// 本地控制时需要输入组件和本地玩家。
			if (!Pawn->InputComponent || !TenkaichiPC || !TenkaichiPC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == TenkaichiGameplayTags::InitState_DataAvailable && DesiredState == TenkaichiGameplayTags::InitState_DataInitialized)
	{
		// 等玩家状态和扩展组件
		ATenkaichiPlayerState* TenkaichiPS = GetPlayerState<ATenkaichiPlayerState>();

		return TenkaichiPS && Manager->HasFeatureReachedInitState(Pawn, UTenkaichiPawnExtensionComponent::NAME_ActorFeatureName, TenkaichiGameplayTags::InitState_DataInitialized);
	}
	else if (CurrentState == TenkaichiGameplayTags::InitState_DataInitialized && DesiredState == TenkaichiGameplayTags::InitState_GameplayReady)
	{
		// TODO 添加能力初始化检查？
		return true;
	}

	return false;
}

// 状态改变时的处理：DataAvailable→DataInitialized 时初始化能力系统/输入/相机
void UTenkaichiHeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == TenkaichiGameplayTags::InitState_DataAvailable && DesiredState == TenkaichiGameplayTags::InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		ATenkaichiPlayerState* TenkaichiPS = GetPlayerState<ATenkaichiPlayerState>();
		if (!ensure(Pawn && TenkaichiPS))
		{
			return;
		}

		const UTenkaichiPawnData* PawnData = nullptr;

		if (UTenkaichiPawnExtensionComponent* PawnExtComp = UTenkaichiPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnData = PawnExtComp->GetPawnData<UTenkaichiPawnData>();

			// PlayerState 持有玩家的持久数据（跨死亡、跨多个 Pawn 存活的状态）。
			// 能力系统组件和属性集挂在 PlayerState 上。
			PawnExtComp->InitializeAbilitySystem(TenkaichiPS->GetTenkaichiAbilitySystemComponent(), TenkaichiPS);
		}

		if (ATenkaichiPlayerController* TenkaichiPC = GetController<ATenkaichiPlayerController>())
		{
			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}

		// 为所有 Pawn 挂上相机委托（之后可能观战）
		if (PawnData)
		{
			if (UTenkaichiCameraComponent* CameraComponent = UTenkaichiCameraComponent::FindCameraComponent(Pawn))
			{
				CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
			}
		}
	}
}

// 别的特性状态改变时：若 PawnExtension 到了 DataInitialized，就推进自己
void UTenkaichiHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UTenkaichiPawnExtensionComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == TenkaichiGameplayTags::InitState_DataInitialized)
		{
			// 如果扩展组件说所有其他组件都初始化好了，就尝试推进到下一状态
			CheckDefaultInitialization();
		}
	}
}

// 推进默认初始化：沿状态链从 spawned 推到 gameplay ready
void UTenkaichiHeroComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = { TenkaichiGameplayTags::InitState_Spawned, TenkaichiGameplayTags::InitState_DataAvailable, TenkaichiGameplayTags::InitState_DataInitialized, TenkaichiGameplayTags::InitState_GameplayReady };

	// 这会从 spawned（在 BeginPlay 里设置）开始推进，经过各数据初始化阶段直到 gameplay ready
	ContinueInitStateChain(StateChain);
}

// BeginPlay：监听 PawnExtension 的状态变化，然后推进到 Spawned 状态
void UTenkaichiHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听 pawn 扩展组件的状态变化
	BindOnActorInitStateChanged(UTenkaichiPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// 通知状态管理器"已生成"，然后继续默认初始化
	ensure(TryToChangeInitState(TenkaichiGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

// EndPlay：注销初始化状态，再调父类
void UTenkaichiHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

// 初始化玩家输入：清空旧映射，绑定 PawnData 里的输入配置
void UTenkaichiHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const UTenkaichiLocalPlayer* LP = Cast<UTenkaichiLocalPlayer>(PC->GetLocalPlayer());
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();

	if (const UTenkaichiPawnExtensionComponent* PawnExtComp = UTenkaichiPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UTenkaichiPawnData* PawnData = PawnExtComp->GetPawnData<UTenkaichiPawnData>())
		{
			if (const UTenkaichiInputConfig* InputConfig = PawnData->InputConfig)
			{
				for (const FInputMappingContextAndPriority& Mapping : DefaultInputMappings)
				{
					if (UInputMappingContext* IMC = Mapping.InputMapping.LoadSynchronous())
					{
						if (Mapping.bRegisterWithSettings)
						{
							if (UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings())
							{
								Settings->RegisterInputMappingContext(IMC);
							}
							
							FModifyContextOptions Options = {};
							Options.bIgnoreAllPressedKeysUntilRelease = false;
							// 实际上把配置加到本地玩家上
							Subsystem->AddMappingContext(IMC, Mapping.Priority, Options);
						}
					}
				}

				// Tenkaichi 输入组件有一些额外函数，把 GameplayTag 映射到输入动作。
				// 如果你想要这个功能但想换输入组件类，把它改成 UTenkaichiInputComponent 的子类
				// 或者相应修改本组件。
				UTenkaichiInputComponent* TenkaichiIC = Cast<UTenkaichiInputComponent>(PlayerInputComponent);
				if (ensureMsgf(TenkaichiIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UTenkaichiInputComponent or a subclass of it.")))
				{
					// 添加可能由玩家设置的按键映射
					TenkaichiIC->AddInputMappings(InputConfig, Subsystem);

					// 这就是我们把输入动作绑定到 gameplay tag 的地方，意味着 Gameplay Ability 蓝图会
					// 直接被这些输入动作的 Triggered 事件触发。
					TArray<uint32> BindHandles;
					TenkaichiIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

					TenkaichiIC->BindNativeAction(InputConfig, TenkaichiGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
					TenkaichiIC->BindNativeAction(InputConfig, TenkaichiGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
					TenkaichiIC->BindNativeAction(InputConfig, TenkaichiGameplayTags::InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
					TenkaichiIC->BindNativeAction(InputConfig, TenkaichiGameplayTags::InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);
					TenkaichiIC->BindNativeAction(InputConfig, TenkaichiGameplayTags::InputTag_AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);
				}
			}
		}
	}

	if (ensure(!bReadyToBindInputs))
	{
		bReadyToBindInputs = true;
	}
 
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}

// 添加模式专属的输入配置：把能力动作绑定到输入组件
void UTenkaichiHeroComponent::AddAdditionalInputConfig(const UTenkaichiInputConfig* InputConfig)
{
	TArray<uint32> BindHandles;

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}
	
	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	if (const UTenkaichiPawnExtensionComponent* PawnExtComp = UTenkaichiPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		UTenkaichiInputComponent* TenkaichiIC = Pawn->FindComponentByClass<UTenkaichiInputComponent>();
		if (ensureMsgf(TenkaichiIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UTenkaichiInputComponent or a subclass of it.")))
		{
			TenkaichiIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
		}
	}
}

// 移除模式专属的输入配置：TODO 未实现
void UTenkaichiHeroComponent::RemoveAdditionalInputConfig(const UTenkaichiInputConfig* InputConfig)
{
	//@TODO: Implement me!
}

// 输入是否已绑定好（非玩家永远 false）
bool UTenkaichiHeroComponent::IsReadyToBindInputs() const
{
	return bReadyToBindInputs;
}

// 能力输入按下：转发给能力系统组件（走 GAS 输入标签）
void UTenkaichiHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const UTenkaichiPawnExtensionComponent* PawnExtComp = UTenkaichiPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			if (UTenkaichiAbilitySystemComponent* TenkaichiASC = PawnExtComp->GetTenkaichiAbilitySystemComponent())
			{
				TenkaichiASC->AbilityInputTagPressed(InputTag);
			}
		}	
	}
}

// 能力输入松开：转发给能力系统组件
void UTenkaichiHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const UTenkaichiPawnExtensionComponent* PawnExtComp = UTenkaichiPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (UTenkaichiAbilitySystemComponent* TenkaichiASC = PawnExtComp->GetTenkaichiAbilitySystemComponent())
		{
			TenkaichiASC->AbilityInputTagReleased(InputTag);
		}
	}
}

// 移动：把输入向量转成世界方向，加移动输入
void UTenkaichiHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;

	// 如果玩家又尝试移动，就取消自动奔跑
	if (ATenkaichiPlayerController* TenkaichiController = Cast<ATenkaichiPlayerController>(Controller))
	{
		TenkaichiController->SetIsAutoRunning(false);
	}
	
	if (Controller)
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			Pawn->AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			Pawn->AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

// 鼠标视角：直接把输入增量加到控制器的 yaw/pitch
void UTenkaichiHeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

// 摇杆视角：输入乘速率再乘 DeltaTime，模拟平滑转视角
void UTenkaichiHeroComponent::Input_LookStick(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	const UWorld* World = GetWorld();
	check(World);

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X * TenkaichiHero::LookYawRate * World->GetDeltaSeconds());
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y * TenkaichiHero::LookPitchRate * World->GetDeltaSeconds());
	}
}

// 蹲伏：切换角色的蹲伏状态
void UTenkaichiHeroComponent::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (ATenkaichiCharacter* Character = GetPawn<ATenkaichiCharacter>())
	{
		Character->ToggleCrouch();
	}
}

// 自动奔跑：切换自动奔跑状态
void UTenkaichiHeroComponent::Input_AutoRun(const FInputActionValue& InputActionValue)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		if (ATenkaichiPlayerController* Controller = Cast<ATenkaichiPlayerController>(Pawn->GetController()))
		{
			// 切换自动奔跑
			Controller->SetIsAutoRunning(!Controller->GetIsAutoRunning());
		}	
	}
}

// 决定当前相机模式：优先用能力覆盖的，否则用 PawnData 里的默认相机
TSubclassOf<UTenkaichiCameraMode> UTenkaichiHeroComponent::DetermineCameraMode() const
{
	if (AbilityCameraMode)
	{
		return AbilityCameraMode;
	}

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return nullptr;
	}

	if (UTenkaichiPawnExtensionComponent* PawnExtComp = UTenkaichiPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UTenkaichiPawnData* PawnData = PawnExtComp->GetPawnData<UTenkaichiPawnData>())
		{
			return PawnData->DefaultCameraMode;
		}
	}

	return nullptr;
}

// 设置能力相机覆盖：技能调用，记录相机模式 + 技能的 Spec 句柄
void UTenkaichiHeroComponent::SetAbilityCameraMode(TSubclassOf<UTenkaichiCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (CameraMode)
	{
		AbilityCameraMode = CameraMode;
		AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
	}
}

// 清除能力相机覆盖：只有"设覆盖的那个技能"才能清除自己的覆盖
void UTenkaichiHeroComponent::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
	{
		AbilityCameraMode = nullptr;
		AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
	}
}

#endif	// ⚠️ L5 GAS 核心 + GameFeatures 就位后放开