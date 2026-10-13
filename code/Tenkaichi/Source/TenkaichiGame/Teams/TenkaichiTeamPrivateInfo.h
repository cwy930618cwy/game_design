// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "TenkaichiTeamInfoBase.h"

#include "TenkaichiTeamPrivateInfo.generated.h"

class UObject;

// 队伍的"私有"信息：为将来"只有本队能看"的私有数据预留的空类
UCLASS()
class ATenkaichiTeamPrivateInfo : public ATenkaichiTeamInfoBase
{
	GENERATED_BODY()

public:
	ATenkaichiTeamPrivateInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};