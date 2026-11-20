// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiPlayerSpawningManagerComponent.h"
#include "GameFramework/PlayerState.h"
#include "EngineUtils.h"
#include "Engine/PlayerStartPIE.h"
#include "TenkaichiPlayerStart.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiPlayerSpawningManagerComponent)

DEFINE_LOG_CATEGORY_STATIC(LogPlayerSpawning, Log, All);

UTenkaichiPlayerSpawningManagerComponent::UTenkaichiPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	// 默认不复制，自动注册/激活，想要初始化。
	SetIsReplicatedByDefault(false);
	bAutoRegister = true;
	bAutoActivate = true;
	bWantsInitializeComponent = true;
	// 组件心跳：PrePhysics 阶段、可以 tick、专用服务器也 tick、默认不开（需要时才开）。
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UTenkaichiPlayerSpawningManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// 监听"新关卡加入世界"，把新关卡里的出生点收进缓存。
	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);

	// 监听"场景里新出生 Actor"，若是出生点就收进缓存。
	UWorld* World = GetWorld();
	World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::HandleOnActorSpawned));

	// 扫描场景里所有已存在的出生点，先全部收进缓存。
	for (TActorIterator<ATenkaichiPlayerStart> It(World); It; ++It)
	{
		if (ATenkaichiPlayerStart* PlayerStart = *It)
		{
			CachedPlayerStarts.Add(PlayerStart);
		}
	}
}

void UTenkaichiPlayerSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
	// 只处理本世界的新关卡。
	if (InWorld == GetWorld())
	{
		for (AActor* Actor : InLevel->Actors)
		{
			if (ATenkaichiPlayerStart* PlayerStart = Cast<ATenkaichiPlayerStart>(Actor))
			{
				ensure(!CachedPlayerStarts.Contains(PlayerStart));
				CachedPlayerStarts.Add(PlayerStart);
			}
		}
	}
}

void UTenkaichiPlayerSpawningManagerComponent::HandleOnActorSpawned(AActor* SpawnedActor)
{
	// 新出生的是出生点，就收进缓存。
	if (ATenkaichiPlayerStart* PlayerStart = Cast<ATenkaichiPlayerStart>(SpawnedActor))
	{
		CachedPlayerStarts.Add(PlayerStart);
	}
}

// ATenkaichiGameMode 代理调用 —— 需要处理有人用引擎常规方式重生玩家的情况。
//======================================================================AActor* UTenkaichiPlayerSpawningManagerComponent::ChoosePlayerStart(AController* Player)
{
	if (Player)
	{
#if WITH_EDITOR
		// 编辑器里优先用"从这开始玩"的出生点。
		if (APlayerStart* PlayerStart = FindPlayFromHereStart(Player))
		{
			return PlayerStart;
		}
#endif

		// 把缓存里仍然有效的出生点收集进临时列表（顺手清掉失效的弱引用）。
		TArray<ATenkaichiPlayerStart*> StarterPoints;
		for (auto StartIt = CachedPlayerStarts.CreateIterator(); StartIt; ++StartIt)
		{
			if (ATenkaichiPlayerStart* Start = (*StartIt).Get())
			{
				StarterPoints.Add(Start);
			}
			else
			{
				StartIt.RemoveCurrent();
			}
		}

		// 纯观战者：随便挑一个出生点位置，但不认领它。
		if (APlayerState* PlayerState = Player->GetPlayerState<APlayerState>())
		{
			if (PlayerState->IsOnlyASpectator())
			{
				if (!StarterPoints.IsEmpty())
				{
					return StarterPoints[FMath::RandRange(0, StarterPoints.Num() - 1)];
				}

				return nullptr;
			}
		}

		// 先让子类（Experience）自己选；没选到就走"随机空位"。
		AActor* PlayerStart = OnChoosePlayerStart(Player, StarterPoints);

		if (!PlayerStart)
		{
			PlayerStart = GetFirstRandomUnoccupiedPlayerStart(Player, StarterPoints);
		}

		// 选中的出生点被这个玩家认领，避免别人再用。
		if (ATenkaichiPlayerStart* LyraStart = Cast<ATenkaichiPlayerStart>(PlayerStart))
		{
			LyraStart->TryClaim(Player);
		}

		return PlayerStart;
	}

	return nullptr;
}

#if WITH_EDITOR
APlayerStart* UTenkaichiPlayerSpawningManagerComponent::FindPlayFromHereStart(AController* Player)
{
	// 只有真正的玩家控制器才用"从这开始玩"，机器人等一律走普通出生点。
	if (Player->IsA<APlayerController>())
	{
		if (UWorld* World = GetWorld())
		{
			for (TActorIterator<APlayerStart> It(World); It; ++It)
			{
				if (APlayerStart* PlayerStart = *It)
				{
					if (PlayerStart->IsA<APlayerStartPIE>())
					{
						// 在 PIE 模式下找到"从这开始玩"的出生点，永远优先用第一个。
						return PlayerStart;
					}
				}
			}
		}
	}

	return nullptr;
}
#endif

bool UTenkaichiPlayerSpawningManagerComponent::ControllerCanRestart(AController* Player)
{
	bool bCanRestart = true;

	// TODO 能重生吗？

	return bCanRestart;
}

void UTenkaichiPlayerSpawningManagerComponent::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	// 重生完成后：先调 C++ 虚函数，再调蓝图事件。
	OnFinishRestartPlayer(NewPlayer, StartRotation);
	K2_OnFinishRestartPlayer(NewPlayer, StartRotation);
}

//================================================================

void UTenkaichiPlayerSpawningManagerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

APlayerStart* UTenkaichiPlayerSpawningManagerComponent::GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<ATenkaichiPlayerStart*>& StartPoints) const
{
	if (Controller)
	{
		// 分成"空位"和"半空"两组。
		TArray<ATenkaichiPlayerStart*> UnOccupiedStartPoints;
		TArray<ATenkaichiPlayerStart*> OccupiedStartPoints;

		for (ATenkaichiPlayerStart* StartPoint : StartPoints)
		{
			ETenkaichiPlayerStartLocationOccupancy State = StartPoint->GetLocationOccupancy(Controller);

			switch (State)
			{
				case ETenkaichiPlayerStartLocationOccupancy::Empty:
					UnOccupiedStartPoints.Add(StartPoint);
					break;
				case ETenkaichiPlayerStartLocationOccupancy::Partial:
					OccupiedStartPoints.Add(StartPoint);
					break;

			}
		}

		// 有"空位"优先挑空位；没有就挑"半空"的。
		if (UnOccupiedStartPoints.Num() > 0)
		{
			return UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
		}
		else if (OccupiedStartPoints.Num() > 0)
		{
			return OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
		}
	}

	return nullptr;
}