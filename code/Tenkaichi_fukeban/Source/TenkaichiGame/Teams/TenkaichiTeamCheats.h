// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/CheatManager.h"

#include "TenkaichiTeamCheats.generated.h"

class UObject;
struct FFrame;

/** 与队伍相关的作弊指令 */
UCLASS()
class UTenkaichiTeamCheats : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	// 把这个玩家移动到下一个可用的队伍，如果已经在队伍列表末尾则绕回第一个队伍
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void CycleTeam();

	// 把这个玩家移动到指定的队伍
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void SetTeam(int32 TeamID);

	// 打印所有队伍的列表
	UFUNCTION(Exec)
	virtual void ListTeams();
};