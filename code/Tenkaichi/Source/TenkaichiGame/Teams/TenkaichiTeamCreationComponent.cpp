// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiTeamCreationComponent.h"

#if 0 // ⚠️ 挂起①：依赖 P13 GameModes，P13 就位后解开（登记注释备忘录㉛）
#include "GameModes/TenkaichiExperienceManagerComponent.h"
#endif

#include "TenkaichiTeamPublicInfo.h"
#include "TenkaichiTeamPrivateInfo.h"
#include "Player/TenkaichiPlayerState.h"
#include "Engine/World.h"

#if 0 // ⚠️ 挂起②：依赖 P13 GameModes，P13 就位后解开（登记注释备忘录㉛）
#include "GameModes/TenkaichiGameMode.h"
#endif

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiTeamCreationComponent)

// 构造函数：给两个队伍信息类填上默认类型
// 构造函数：给两个队伍信息类填上默认类型
UTenkaichiTeamCreationComponent::UTenkaichiTeamCreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PublicTeamInfoClass = ATenkaichiTeamPublicInfo::StaticClass();
	PrivateTeamInfoClass = ATenkaichiTeamPrivateInfo::StaticClass();
}

#if WITH_EDITOR
EDataValidationResult UTenkaichiTeamCreationComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	//@TODO: TEAMS: 校验所有外观资产都设置了相同的属性！

	return Result;
}
#endif

void UTenkaichiTeamCreationComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听体验加载完成
#if 0 // ⚠️ 挂起③：依赖 P13 TenkaichiExperienceManagerComponent，P13 就位后解开（㉛）
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	UTenkaichiExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UTenkaichiExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_HighPriority(FOnTenkaichiExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
#endif
}

#if 0 // ⚠️ 挂起④：依赖 P13 TenkaichiExperienceDefinition，P13 就位后解开（㉛）
// 体验加载完成的回调：在服务器端创建所有队伍并给已有玩家分配队伍
void UTenkaichiTeamCreationComponent::OnExperienceLoaded(const UTenkaichiExperienceDefinition* Experience)
{
#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		ServerCreateTeams();
		ServerAssignPlayersToTeams();
	}
#endif
}
#endif

#if WITH_SERVER_CODE

// 服务器端：按配置表逐个创建队伍
void UTenkaichiTeamCreationComponent::ServerCreateTeams()
{
	for (const auto& KVP : TeamsToCreate)
	{
		const int32 TeamId = KVP.Key;
		ServerCreateTeam(TeamId, KVP.Value);
	}
}

// 服务器端：给当前已存在的玩家分配队伍，并监听之后新登录的玩家
void UTenkaichiTeamCreationComponent::ServerAssignPlayersToTeams()
{
	// 给已经存在的玩家分配队伍
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (ATenkaichiPlayerState* TenkaichiPS = Cast<ATenkaichiPlayerState>(PS))
		{
			ServerChooseTeamForPlayer(TenkaichiPS);
		}
	}

	// 监听新玩家登录
#if 0 // ⚠️ 挂起⑤：依赖 P13 TenkaichiGameMode，P13 就位后解开（㉛）
	ATenkaichiGameMode* GameMode = Cast<ATenkaichiGameMode>(GameState->AuthorityGameMode);
	check(GameMode);

	GameMode->OnGameModePlayerInitialized.AddUObject(this, &ThisClass::OnPlayerInitialized);
#endif
}

// 服务器端：为单个玩家选定队伍（观战玩家不分配队伍）
void UTenkaichiTeamCreationComponent::ServerChooseTeamForPlayer(ATenkaichiPlayerState* PS)
{
	if (PS->IsOnlyASpectator())
	{
		PS->SetGenericTeamId(FGenericTeamId::NoTeam);
	}
	else
	{
		const FGenericTeamId TeamID = IntegerToGenericTeamId(GetLeastPopulatedTeamID());
		PS->SetGenericTeamId(TeamID);
	}
}

// 新玩家初始化完成的回调：为其分配队伍
void UTenkaichiTeamCreationComponent::OnPlayerInitialized(AGameModeBase* GameMode, AController* NewPlayer)
{
	check(NewPlayer);
	check(NewPlayer->PlayerState);
	if (ATenkaichiPlayerState* TenkaichiPS = Cast<ATenkaichiPlayerState>(NewPlayer->PlayerState))
	{
		ServerChooseTeamForPlayer(TenkaichiPS);
	}
}

// 服务器端：为指定队号生成一对队伍信息 Actor（公开 + 私有）并设置外观
void UTenkaichiTeamCreationComponent::ServerCreateTeam(int32 TeamId, UTenkaichiTeamDisplayAsset* DisplayAsset)
{
	check(HasAuthority());

	//@TODO: 确保这个队伍还不存在

	UWorld* World = GetWorld();
	check(World);

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATenkaichiTeamPublicInfo* NewTeamPublicInfo = World->SpawnActor<ATenkaichiTeamPublicInfo>(PublicTeamInfoClass, SpawnInfo);
	checkf(NewTeamPublicInfo != nullptr, TEXT("Failed to create public team actor from class %s"), *GetPathNameSafe(*PublicTeamInfoClass));
	NewTeamPublicInfo->SetTeamId(TeamId);
	NewTeamPublicInfo->SetTeamDisplayAsset(DisplayAsset);

	ATenkaichiTeamPrivateInfo* NewTeamPrivateInfo = World->SpawnActor<ATenkaichiTeamPrivateInfo>(PrivateTeamInfoClass, SpawnInfo);
	checkf(NewTeamPrivateInfo != nullptr, TEXT("Failed to create private team actor from class %s"), *GetPathNameSafe(*PrivateTeamInfoClass));
	NewTeamPrivateInfo->SetTeamId(TeamId);
}

// 返回当前人数最少的队伍 ID，若没有任何有效队伍则返回 INDEX_NONE
int32 UTenkaichiTeamCreationComponent::GetLeastPopulatedTeamID() const
{
	const int32 NumTeams = TeamsToCreate.Num();
	if (NumTeams > 0)
	{
		TMap<int32, uint32> TeamMemberCounts;
		TeamMemberCounts.Reserve(NumTeams);

		for (const auto& KVP : TeamsToCreate)
		{
			const int32 TeamId = KVP.Key;
			TeamMemberCounts.Add(TeamId, 0);
		}

		AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
		for (APlayerState* PS : GameState->PlayerArray)
		{
			if (ATenkaichiPlayerState* TenkaichiPS = Cast<ATenkaichiPlayerState>(PS))
			{
				const int32 PlayerTeamID = TenkaichiPS->GetTeamId();

				if ((PlayerTeamID != INDEX_NONE) && !TenkaichiPS->IsInactive())	// 不统计未分配队伍或已断线的玩家
				{
					check(TeamMemberCounts.Contains(PlayerTeamID))
					TeamMemberCounts[PlayerTeamID] += 1;
				}
			}
		}

		// 先按队伍人数从少到多排序，人数相同再按队伍 ID 排序
		int32 BestTeamId = INDEX_NONE;
		uint32 BestPlayerCount = TNumericLimits<uint32>::Max();
		for (const auto& KVP : TeamMemberCounts)
		{
			const int32 TestTeamId = KVP.Key;
			const uint32 TestTeamPlayerCount = KVP.Value;

			if (TestTeamPlayerCount < BestPlayerCount)
			{
				BestTeamId = TestTeamId;
				BestPlayerCount = TestTeamPlayerCount;
			}
			else if (TestTeamPlayerCount == BestPlayerCount)
			{
				if ((TestTeamId < BestTeamId) || (BestTeamId == INDEX_NONE))
				{
					BestTeamId = TestTeamId;
					BestPlayerCount = TestTeamPlayerCount;
				}
			}
		}

		return BestTeamId;
	}

	return INDEX_NONE;
}
#endif	// WITH_SERVER_CODE