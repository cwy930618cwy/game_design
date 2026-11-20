// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiBotCreationComponent.h"
#include "TenkaichiGameMode.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/TenkaichiExperienceManagerComponent.h"
//#include "Development/TenkaichiDeveloperSettings.h"   // ⚠️ 注释挂起：依赖 P18 DeveloperSettings
#include "Character/TenkaichiPawnExtensionComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Character/TenkaichiHealthComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiBotCreationComponent)

// 构造函数：空实现（父类处理初始化）。
UTenkaichiBotCreationComponent::UTenkaichiBotCreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 组件开始游戏：监听"经验加载完成"。
void UTenkaichiBotCreationComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听经验加载完成
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	UTenkaichiExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UTenkaichiExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_LowPriority(FOnTenkaichiExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

// 经验加载完成的回调（最低优先级）：权威端生成 Bot。
void UTenkaichiBotCreationComponent::OnExperienceLoaded(const UTenkaichiExperienceDefinition* Experience)
{
#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		ServerCreateBots();
	}
#endif
}

#if WITH_SERVER_CODE

// 生成最多 NumBotsToCreate 个 Bot。
void UTenkaichiBotCreationComponent::ServerCreateBots_Implementation()
{
	if (BotControllerClass == nullptr)
	{
		return;
	}

	RemainingBotNames = RandomBotNames;

	// 确定要生成多少个 Bot
	int32 EffectiveBotCount = NumBotsToCreate;

	// 给开发者设置一个覆盖它的机会
	if (GIsEditor)
	{
		const UTenkaichiDeveloperSettings* DeveloperSettings = GetDefault<UTenkaichiDeveloperSettings>();

		if (DeveloperSettings->bOverrideBotCount)
		{
			EffectiveBotCount = DeveloperSettings->OverrideNumPlayerBotsToSpawn;
		}
	}

	// 给 URL 一个覆盖它的机会
	if (AGameModeBase* GameModeBase = GetGameMode<AGameModeBase>())
	{
		EffectiveBotCount = UGameplayStatics::GetIntOption(GameModeBase->OptionsString, TEXT("NumBots"), EffectiveBotCount);
	}

	// 创建它们
	for (int32 Count = 0; Count < EffectiveBotCount; ++Count)
	{
		SpawnOneBot();
	}
}

// 按玩家索引生成一个 Bot 名字。
FString UTenkaichiBotCreationComponent::CreateBotName(int32 PlayerIndex)
{
	FString Result;
	if (RemainingBotNames.Num() > 0)
	{
		const int32 NameIndex = FMath::RandRange(0, RemainingBotNames.Num() - 1);
		Result = RemainingBotNames[NameIndex];
		RemainingBotNames.RemoveAtSwap(NameIndex);
	}
	else
	{
		//@TODO: PlayerId 目前只对玩家初始化了
		PlayerIndex = FMath::RandRange(260, 260+100);
		Result = FString::Printf(TEXT("Tinplate %d"), PlayerIndex);
	}
	return Result;
}

// 生成单个 Bot。
void UTenkaichiBotCreationComponent::SpawnOneBot()
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = GetComponentLevel();
	SpawnInfo.ObjectFlags |= RF_Transient;
	AAIController* NewController = GetWorld()->SpawnActor<AAIController>(BotControllerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);

	if (NewController != nullptr)
	{
		ATenkaichiGameMode* GameMode = GetGameMode<ATenkaichiGameMode>();
		check(GameMode);

		if (NewController->PlayerState != nullptr)
		{
			NewController->PlayerState->SetPlayerName(CreateBotName(NewController->PlayerState->GetPlayerId()));
		}

		GameMode->GenericPlayerInitialization(NewController);
		GameMode->RestartPlayer(NewController);

		if (NewController->GetPawn() != nullptr)
		{
			if (UTenkaichiPawnExtensionComponent* PawnExtComponent = NewController->GetPawn()->FindComponentByClass<UTenkaichiPawnExtensionComponent>())
			{
				PawnExtComponent->CheckDefaultInitialization();
			}
		}

		SpawnedBotList.Add(NewController);
	}
}

// 如果可能，删除最后生成的一个 Bot。
void UTenkaichiBotCreationComponent::RemoveOneBot()
{
	if (SpawnedBotList.Num() > 0)
	{
		// 现在只是随机删一个 Bot（因为它们都一样）；以后可以按技能高低等来选择删哪个
		const int32 BotToRemoveIndex = FMath::RandRange(0, SpawnedBotList.Num() - 1);

		AAIController* BotToRemove = SpawnedBotList[BotToRemoveIndex];
		SpawnedBotList.RemoveAtSwap(BotToRemoveIndex);

		if (BotToRemove)
		{
			// 如果能找到生命组件，就自毁它，否则直接销毁 Actor
			if (APawn* ControlledPawn = BotToRemove->GetPawn())
			{
				if (UTenkaichiHealthComponent* HealthComponent = UTenkaichiHealthComponent::FindHealthComponent(ControlledPawn))
				{
					// 注意，现在这个效果还不太理想：控制器被销毁时玩家状态一消失，
					// 死亡动画这类能力会立刻被打断
					HealthComponent->DamageSelfDestruct();
				}
				else
				{
					ControlledPawn->Destroy();
				}
			}

			// 销毁控制器（会导致它 Logout 等）
			BotToRemove->Destroy();
		}
	}
}

#else // !WITH_SERVER_CODE

// 客户端版：这些函数在客户端不存在。
void UTenkaichiBotCreationComponent::ServerCreateBots_Implementation()
{
	ensureMsgf(0, TEXT("Bot functions do not exist in TenkaichiClient!"));
}

// 客户端版：这些函数在客户端不存在。
void UTenkaichiBotCreationComponent::SpawnOneBot()
{
	ensureMsgf(0, TEXT("Bot functions do not exist in TenkaichiClient!"));
}

// 客户端版：这些函数在客户端不存在。
void UTenkaichiBotCreationComponent::RemoveOneBot()
{
	ensureMsgf(0, TEXT("Bot functions do not exist in TenkaichiClient!"));
}

#endif