// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiCheatManager.h"
#include "GameFramework/Pawn.h"
#include "TenkaichiPlayerController.h"
#include "TenkaichiDebugCameraController.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Console.h"
#include "GameFramework/HUD.h"
#include "System/TenkaichiAssetManager.h"
#include "System/TenkaichiGameData.h"
#include "TenkaichiGameplayTags.h"
#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Character/TenkaichiHealthComponent.h"
#include "Character/TenkaichiPawnExtensionComponent.h"
#include "System/TenkaichiSystemStatics.h"
#include "Development/TenkaichiDeveloperSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiCheatManager)

#if 0 // ⚠️ 挂起：依赖 P12.6 DebugCameraController / P14 AssetManager+GameData / P18 DeveloperSettings，就位后解开

DEFINE_LOG_CATEGORY(LogTenkaichiCheat);

namespace TenkaichiCheat
{
	static const FName NAME_Fixed = FName(TEXT("Fixed"));

	static bool bEnableDebugCameraCycling = false;
	static FAutoConsoleVariableRef CVarEnableDebugCameraCycling(
		TEXT("TenkaichiCheat.EnableDebugCameraCycling"),
		bEnableDebugCameraCycling,
		TEXT("若为 true，则游戏运行时可以循环切换调试相机。"),
		ECVF_Cheat);

	static bool bStartInGodMode = false;
	static FAutoConsoleVariableRef CVarStartInGodMode(
		TEXT("TenkaichiCheat.StartInGodMode"),
		bStartInGodMode,
		TEXT("若为 true，则开始游玩时自动应用无敌作弊。"),
		ECVF_Cheat);
};


// 构造函数：把调试相机控制器指定为工程自己的 DebugCameraController。
UTenkaichiCheatManager::UTenkaichiCheatManager()
{
	DebugCameraControllerClass = ATenkaichiDebugCameraController::StaticClass();
}

// 作弊管理器初始化：编辑器下自动执行配置里"创建时"的作弊指令；若开启则进入无敌。
void UTenkaichiCheatManager::InitCheatManager()
{
	Super::InitCheatManager();

#if WITH_EDITOR
	if (GIsEditor)
	{
		APlayerController* PC = GetOuterAPlayerController();
		for (const FTenkaichiCheatToRun& CheatRow : GetDefault<UTenkaichiDeveloperSettings>()->CheatsToRun)
		{
			if (CheatRow.Phase == ECheatExecutionTime::OnCheatManagerCreated)
			{
				PC->ConsoleCommand(CheatRow.Cheat, /*bWriteToLog=*/ true);
			}
		}
	}
#endif

	if (TenkaichiCheat::bStartInGodMode)
	{
		God();
	}
}

// 把文本同时输出到控制台和日志。
void UTenkaichiCheatManager::CheatOutputText(const FString& TextToOutput)
{
#if USING_CHEAT_MANAGER
	// 输出到控制台。
	if (GEngine && GEngine->GameViewport && GEngine->GameViewport->ViewportConsole)
	{
		GEngine->GameViewport->ViewportConsole->OutputText(TextToOutput);
	}

	// 输出到日志。
	UE_LOG(LogTenkaichiCheat, Display, TEXT("%s"), *TextToOutput);
#endif // USING_CHEAT_MANAGER
}

// 为当前玩家在服务器上执行一条作弊指令（客户端输入，转发到服务器）。
void UTenkaichiCheatManager::Cheat(const FString& Msg)
{
	if (ATenkaichiPlayerController* TenkaichiPC = Cast<ATenkaichiPlayerController>(GetOuterAPlayerController()))
	{
		TenkaichiPC->ServerCheat(Msg.Left(128));
	}
}

// 为所有玩家在服务器上执行一条作弊指令。
void UTenkaichiCheatManager::CheatAll(const FString& Msg)
{
	if (ATenkaichiPlayerController* TenkaichiPC = Cast<ATenkaichiPlayerController>(GetOuterAPlayerController()))
	{
		TenkaichiPC->ServerCheatAll(Msg.Left(128));
	}
}

// 开始下一场比赛（直接调用系统工具，无需经过服务器）。
void UTenkaichiCheatManager::PlayNextGame()
{
	UTenkaichiSystemStatics::PlayNextGame(this);
}

// 启用调试相机（调用引擎父类逻辑）。
void UTenkaichiCheatManager::EnableDebugCamera()
{
	Super::EnableDebugCamera();
}

// 关闭调试相机：若原控制器正处于"固定"相机模式，则把相机位置同步回原控制器。
void UTenkaichiCheatManager::DisableDebugCamera()
{
	FVector DebugCameraLocation;
	FRotator DebugCameraRotation;

	ADebugCameraController* DebugCC = Cast<ADebugCameraController>(GetOuter());
	APlayerController* OriginalPC = nullptr;

	if (DebugCC)
	{
		OriginalPC = DebugCC->OriginalControllerRef;
		DebugCC->GetPlayerViewPoint(DebugCameraLocation, DebugCameraRotation);
	}

	Super::DisableDebugCamera();

	if (OriginalPC && OriginalPC->PlayerCameraManager && (OriginalPC->PlayerCameraManager->CameraStyle == TenkaichiCheat::NAME_Fixed))
	{
		OriginalPC->SetInitialLocationAndRotation(DebugCameraLocation, DebugCameraRotation);

		OriginalPC->PlayerCameraManager->ViewTarget.POV.Location = DebugCameraLocation;
		OriginalPC->PlayerCameraManager->ViewTarget.POV.Rotation = DebugCameraRotation;
		OriginalPC->PlayerCameraManager->PendingViewTarget.POV.Location = DebugCameraLocation;
		OriginalPC->PlayerCameraManager->PendingViewTarget.POV.Rotation = DebugCameraRotation;
	}
}

// 查询是否处于调试相机模式（当前控制器是否为 DebugCameraController）。
bool UTenkaichiCheatManager::InDebugCamera() const
{
	return (Cast<ADebugCameraController>(GetOuter()) ? true : false);
}

// 启用固定相机：把当前控制器的相机模式切换为 "Fixed"。
void UTenkaichiCheatManager::EnableFixedCamera()
{
	const ADebugCameraController* DebugCC = Cast<ADebugCameraController>(GetOuter());
	APlayerController* PC = (DebugCC ? ToRawPtr(DebugCC->OriginalControllerRef) : GetOuterAPlayerController());

	if (PC && PC->PlayerCameraManager)
	{
		PC->SetCameraMode(TenkaichiCheat::NAME_Fixed);
	}
}

// 关闭固定相机：把当前控制器的相机模式恢复为默认。
void UTenkaichiCheatManager::DisableFixedCamera()
{
	const ADebugCameraController* DebugCC = Cast<ADebugCameraController>(GetOuter());
	APlayerController* PC = (DebugCC ? ToRawPtr(DebugCC->OriginalControllerRef) : GetOuterAPlayerController());

	if (PC && PC->PlayerCameraManager)
	{
		PC->SetCameraMode(NAME_Default);
	}
}

// 查询是否处于固定相机模式。
bool UTenkaichiCheatManager::InFixedCamera() const
{
	const ADebugCameraController* DebugCC = Cast<ADebugCameraController>(GetOuter());
	const APlayerController* PC = (DebugCC ? ToRawPtr(DebugCC->OriginalControllerRef) : GetOuterAPlayerController());

	if (PC && PC->PlayerCameraManager)
	{
		return (PC->PlayerCameraManager->CameraStyle == TenkaichiCheat::NAME_Fixed);
	}

	return false;
}

// 切换固定相机：开 → 关，关 → 开。
void UTenkaichiCheatManager::ToggleFixedCamera()
{
	if (InFixedCamera())
	{
		DisableFixedCamera();
	}
	else
	{
		EnableFixedCamera();
	}
}

// 循环切换调试相机：依次在 调试相机 / 固定相机 / 默认相机 之间切换。
void UTenkaichiCheatManager::CycleDebugCameras()
{
	if (!TenkaichiCheat::bEnableDebugCameraCycling)
	{
		return;
	}

	if (InDebugCamera())
	{
		EnableFixedCamera();
		DisableDebugCamera();
	}
	else if (InFixedCamera())
	{
		DisableFixedCamera();
		DisableDebugCamera();
	}
	else
	{
		EnableDebugCamera();
		DisableFixedCamera();
	}
}

// 循环切换技能系统调试 HUD 的分类。
void UTenkaichiCheatManager::CycleAbilitySystemDebug()
{
	APlayerController* PC = Cast<APlayerController>(GetOuterAPlayerController());

	if (PC && PC->MyHUD)
	{
		if (!PC->MyHUD->bShowDebugInfo || !PC->MyHUD->DebugDisplay.Contains(TEXT("AbilitySystem")))
		{
			PC->MyHUD->ShowDebug(TEXT("AbilitySystem"));
		}

		PC->ConsoleCommand(TEXT("AbilitySystem.Debug.NextCategory"));
	}
}

// 强制取消所有由输入激活的技能（用于排查技能打断问题）。
void UTenkaichiCheatManager::CancelActivatedAbilities()
{
	if (UTenkaichiAbilitySystemComponent* TenkaichiASC = GetPlayerAbilitySystemComponent())
	{
		const bool bReplicateCancelAbility = true;
		TenkaichiASC->CancelInputActivatedAbilities(bReplicateCancelAbility);
	}
}

// 给当前玩家加上一个动态标签。
void UTenkaichiCheatManager::AddTagToSelf(FString TagName)
{
	FGameplayTag Tag = TenkaichiGameplayTags::FindTagByString(TagName, true);
	if (Tag.IsValid())
	{
		if (UTenkaichiAbilitySystemComponent* TenkaichiASC = GetPlayerAbilitySystemComponent())
		{
			TenkaichiASC->AddDynamicTagGameplayEffect(Tag);
		}
	}
	else
	{
		UE_LOG(LogTenkaichiCheat, Display, TEXT("AddTagToSelf: Could not find any tag matching [%s]."), *TagName);
	}
}

// 移除当前玩家身上的一个动态标签。
void UTenkaichiCheatManager::RemoveTagFromSelf(FString TagName)
{
	FGameplayTag Tag = TenkaichiGameplayTags::FindTagByString(TagName, true);
	if (Tag.IsValid())
	{
		if (UTenkaichiAbilitySystemComponent* TenkaichiASC = GetPlayerAbilitySystemComponent())
		{
			TenkaichiASC->RemoveDynamicTagGameplayEffect(Tag);
		}
	}
	else
	{
		UE_LOG(LogTenkaichiCheat, Display, TEXT("RemoveTagFromSelf: Could not find any tag matching [%s]."), *TagName);
	}
}

// 对当前玩家造成指定伤害（内部走 SetByCaller 伤害效果）。
void UTenkaichiCheatManager::DamageSelf(float DamageAmount)
{
	if (UTenkaichiAbilitySystemComponent* TenkaichiASC = GetPlayerAbilitySystemComponent())
	{
		ApplySetByCallerDamage(TenkaichiASC, DamageAmount);
	}
}

// 对玩家当前瞄准的目标造成伤害（客户端自动转发到服务器）。
void UTenkaichiCheatManager::DamageTarget(float DamageAmount)
{
	if (ATenkaichiPlayerController* TenkaichiPC = Cast<ATenkaichiPlayerController>(GetOuterAPlayerController()))
	{
		if (TenkaichiPC->GetNetMode() == NM_Client)
		{
			// 为方便起见，自动把作弊指令发给服务器。
			TenkaichiPC->ServerCheat(FString::Printf(TEXT("DamageTarget %.2f"), DamageAmount));
			return;
		}

		FHitResult TargetHitResult;
		AActor* TargetActor = GetTarget(TenkaichiPC, TargetHitResult);

		if (UTenkaichiAbilitySystemComponent* TenkaichiTargetASC = Cast<UTenkaichiAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor)))
		{
			ApplySetByCallerDamage(TenkaichiTargetASC, DamageAmount);
		}
	}
}

// 用 SetByCaller 方式对指定 ASC 造成伤害。
void UTenkaichiCheatManager::ApplySetByCallerDamage(UTenkaichiAbilitySystemComponent* TenkaichiASC, float DamageAmount)
{
	check(TenkaichiASC);

	TSubclassOf<UGameplayEffect> DamageGE = UTenkaichiAssetManager::GetSubclass(UTenkaichiGameData::Get().DamageGameplayEffect_SetByCaller);
	FGameplayEffectSpecHandle SpecHandle = TenkaichiASC->MakeOutgoingSpec(DamageGE, 1.0f, TenkaichiASC->MakeEffectContext());

	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(TenkaichiGameplayTags::SetByCaller_Damage, DamageAmount);
		TenkaichiASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

// 对当前玩家进行一定治疗。
void UTenkaichiCheatManager::HealSelf(float HealAmount)
{
	if (UTenkaichiAbilitySystemComponent* TenkaichiASC = GetPlayerAbilitySystemComponent())
	{
		ApplySetByCallerHeal(TenkaichiASC, HealAmount);
	}
}

// 对玩家当前瞄准的目标进行一定治疗。
void UTenkaichiCheatManager::HealTarget(float HealAmount)
{
	if (ATenkaichiPlayerController* TenkaichiPC = Cast<ATenkaichiPlayerController>(GetOuterAPlayerController()))
	{
		FHitResult TargetHitResult;
		AActor* TargetActor = GetTarget(TenkaichiPC, TargetHitResult);

		if (UTenkaichiAbilitySystemComponent* TenkaichiTargetASC = Cast<UTenkaichiAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor)))
		{
			ApplySetByCallerHeal(TenkaichiTargetASC, HealAmount);
		}
	}
}

// 用 SetByCaller 方式对指定 ASC 进行治疗。
void UTenkaichiCheatManager::ApplySetByCallerHeal(UTenkaichiAbilitySystemComponent* TenkaichiASC, float HealAmount)
{
	check(TenkaichiASC);

	TSubclassOf<UGameplayEffect> HealGE = UTenkaichiAssetManager::GetSubclass(UTenkaichiGameData::Get().HealGameplayEffect_SetByCaller);
	FGameplayEffectSpecHandle SpecHandle = TenkaichiASC->MakeOutgoingSpec(HealGE, 1.0f, TenkaichiASC->MakeEffectContext());

	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(TenkaichiGameplayTags::SetByCaller_Heal, HealAmount);
		TenkaichiASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

// 获取当前玩家的技能系统组件（ASC）。
UTenkaichiAbilitySystemComponent* UTenkaichiCheatManager::GetPlayerAbilitySystemComponent() const
{
	if (ATenkaichiPlayerController* TenkaichiPC = Cast<ATenkaichiPlayerController>(GetOuterAPlayerController()))
	{
		return TenkaichiPC->GetTenkaichiAbilitySystemComponent();
	}
	return nullptr;
}

// 对当前玩家造成足以致死的伤害（需先进入 GameplayReady 状态）。
void UTenkaichiCheatManager::DamageSelfDestruct()
{
	if (ATenkaichiPlayerController* TenkaichiPC = Cast<ATenkaichiPlayerController>(GetOuterAPlayerController()))
	{
		if (const UTenkaichiPawnExtensionComponent* PawnExtComp = UTenkaichiPawnExtensionComponent::FindPawnExtensionComponent(TenkaichiPC->GetPawn()))
		{
			if (PawnExtComp->HasReachedInitState(TenkaichiGameplayTags::InitState_GameplayReady))
			{
				if (UTenkaichiHealthComponent* HealthComponent = UTenkaichiHealthComponent::FindHealthComponent(TenkaichiPC->GetPawn()))
				{
					HealthComponent->DamageSelfDestruct();
				}
			}
		}
	}
}

// 无敌模式：切换"无敌"动态标签（客户端自动转发到服务器）。
void UTenkaichiCheatManager::God()
{
	if (ATenkaichiPlayerController* TenkaichiPC = Cast<ATenkaichiPlayerController>(GetOuterAPlayerController()))
	{
		if (TenkaichiPC->GetNetMode() == NM_Client)
		{
			// 为了方便起见，自动把作弊指令发给服务器。
			TenkaichiPC->ServerCheat(FString::Printf(TEXT("God")));
			return;
		}

		if (UTenkaichiAbilitySystemComponent* TenkaichiASC = TenkaichiPC->GetTenkaichiAbilitySystemComponent())
		{
			const FGameplayTag Tag = TenkaichiGameplayTags::Cheat_GodMode;
			const bool bHasTag = TenkaichiASC->HasMatchingGameplayTag(Tag);

			if (bHasTag)
			{
				TenkaichiASC->RemoveDynamicTagGameplayEffect(Tag);
			}
			else
			{
				TenkaichiASC->AddDynamicTagGameplayEffect(Tag);
			}
		}
	}
}

// 无限生命：切换"无限生命"动态标签（Enabled=-1 表示切换）。
void UTenkaichiCheatManager::UnlimitedHealth(int32 Enabled)
{
	if (UTenkaichiAbilitySystemComponent* TenkaichiASC = GetPlayerAbilitySystemComponent())
	{
		const FGameplayTag Tag = TenkaichiGameplayTags::Cheat_UnlimitedHealth;
		const bool bHasTag = TenkaichiASC->HasMatchingGameplayTag(Tag);

		if ((Enabled == -1) || ((Enabled > 0) && !bHasTag) || ((Enabled == 0) && bHasTag))
		{
			if (bHasTag)
			{
				TenkaichiASC->RemoveDynamicTagGameplayEffect(Tag);
			}
			else
			{
				TenkaichiASC->AddDynamicTagGameplayEffect(Tag);
			}
		}
	}
}
#endif // ⚠️ 挂起结束