// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiTeamStatics.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TenkaichiLogChannels.h"
#include "Teams/TenkaichiTeamDisplayAsset.h"
#include "Teams/TenkaichiTeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiTeamStatics)

class UTexture;

//////////////////////////////////////////////////////////////////////

// 从任意对象查队伍信息（队号 + 是否在队 + 外观资产）
void UTenkaichiTeamStatics::FindTeamFromObject(const UObject* Agent, bool& bIsPartOfTeam, int32& TeamId, UTenkaichiTeamDisplayAsset*& DisplayAsset, bool bLogIfNotSet)
{
	bIsPartOfTeam = false;
	TeamId = INDEX_NONE;
	DisplayAsset = nullptr;

	if (UWorld* World = GEngine->GetWorldFromContextObject(Agent, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UTenkaichiTeamSubsystem* TeamSubsystem = World->GetSubsystem<UTenkaichiTeamSubsystem>())
		{
			TeamId = TeamSubsystem->FindTeamFromObject(Agent);
			if (TeamId != INDEX_NONE)
			{
				bIsPartOfTeam = true;

				DisplayAsset = TeamSubsystem->GetTeamDisplayAsset(TeamId, INDEX_NONE);

				if ((DisplayAsset == nullptr) && bLogIfNotSet)
				{
					UE_LOG(LogTenkaichiTeams, Log, TEXT("FindTeamFromObject(%s) called too early (found team %d but no display asset set yet"), *GetPathNameSafe(Agent), TeamId);
				}
			}
		}
		else
		{
			UE_LOG(LogTenkaichiTeams, Error, TEXT("FindTeamFromObject(%s) failed: Team subsystem does not exist yet"), *GetPathNameSafe(Agent));
		}
	}
}

// 按队号查外观资产
UTenkaichiTeamDisplayAsset* UTenkaichiTeamStatics::GetTeamDisplayAsset(const UObject* WorldContextObject, int32 TeamId)
{
	UTenkaichiTeamDisplayAsset* Result = nullptr;
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UTenkaichiTeamSubsystem* TeamSubsystem = World->GetSubsystem<UTenkaichiTeamSubsystem>())
		{
			return TeamSubsystem->GetTeamDisplayAsset(TeamId, INDEX_NONE);
		}
	}
	return Result;
}

// 取浮点参数，取不到返回默认值
float UTenkaichiTeamStatics::GetTeamScalarWithFallback(UTenkaichiTeamDisplayAsset* DisplayAsset, FName ParameterName, float DefaultValue)
{
	if (DisplayAsset)
	{
		if (float* pValue = DisplayAsset->ScalarParameters.Find(ParameterName))
		{
			return *pValue;
		}
	}
	return DefaultValue;
}

// 取颜色参数，取不到返回默认值
FLinearColor UTenkaichiTeamStatics::GetTeamColorWithFallback(UTenkaichiTeamDisplayAsset* DisplayAsset, FName ParameterName, FLinearColor DefaultValue)
{
	if (DisplayAsset)
	{
		if (FLinearColor* pColor = DisplayAsset->ColorParameters.Find(ParameterName))
		{
			return *pColor;
		}
	}
	return DefaultValue;
}

// 取贴图参数，取不到返回默认值
UTexture* UTenkaichiTeamStatics::GetTeamTextureWithFallback(UTenkaichiTeamDisplayAsset* DisplayAsset, FName ParameterName, UTexture* DefaultValue)
{
	if (DisplayAsset)
	{
		if (TObjectPtr<UTexture>* pTexture = DisplayAsset->TextureParameters.Find(ParameterName))
		{
			return *pTexture;
		}
	}
	return DefaultValue;
}

