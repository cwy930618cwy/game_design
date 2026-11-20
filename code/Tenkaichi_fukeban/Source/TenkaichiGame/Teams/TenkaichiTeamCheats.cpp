// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiTeamCheats.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Teams/TenkaichiTeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiTeamCheats)

//////////////////////////////////////////////////////////////////////
// UTenkaichiTeamCheats

// 把当前玩家轮换到下一个队伍（到末尾则绕回第一个队伍）
void UTenkaichiTeamCheats::CycleTeam()
{
	if (UTenkaichiTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<UTenkaichiTeamSubsystem>(GetWorld()))
	{
		APlayerController* PC = GetPlayerController();

		const int32 OldTeamId = TeamSubsystem->FindTeamFromObject(PC);
		const TArray<int32> TeamIds = TeamSubsystem->GetTeamIDs();
		
		if (TeamIds.Num())
		{
			const int32 IndexOfOldTeam = TeamIds.Find(OldTeamId);
			const int32 IndexToUse = (IndexOfOldTeam + 1) % TeamIds.Num();

			const int32 NewTeamId = TeamIds[IndexToUse];

			TeamSubsystem->ChangeTeamForActor(PC, NewTeamId);
		}

		const int32 ActualNewTeamId = TeamSubsystem->FindTeamFromObject(PC);

		UE_LOG(LogConsoleResponse, Log, TEXT("Changed to team %d (from team %d)"), ActualNewTeamId, OldTeamId);
	}
}

// 把当前玩家移动到指定的队伍
void UTenkaichiTeamCheats::SetTeam(int32 TeamID)
{
	if (UTenkaichiTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<UTenkaichiTeamSubsystem>(GetWorld()))
	{
		if (TeamSubsystem->DoesTeamExist(TeamID))
		{
			APlayerController* PC = GetPlayerController();

			TeamSubsystem->ChangeTeamForActor(PC, TeamID);
		}
	}
}

// 打印所有队伍的列表
void UTenkaichiTeamCheats::ListTeams()
{
	if (UTenkaichiTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<UTenkaichiTeamSubsystem>(GetWorld()))
	{
		const TArray<int32> TeamIDs = TeamSubsystem->GetTeamIDs();

		for (const int32 TeamID : TeamIDs)
		{
			UE_LOG(LogConsoleResponse, Log, TEXT("Team ID %d"), TeamID);
		}
	}
}