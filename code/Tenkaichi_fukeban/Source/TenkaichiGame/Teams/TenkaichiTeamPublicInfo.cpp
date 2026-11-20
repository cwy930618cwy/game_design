// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiTeamPublicInfo.h"

#include "Net/UnrealNetwork.h"
#include "Teams/TenkaichiTeamInfoBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiTeamPublicInfo)

class FLifetimeProperty;

// 构造函数：只调基类构造函数
ATenkaichiTeamPublicInfo::ATenkaichiTeamPublicInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 声明哪些字段要复制（外观数据只在初次复制）
void ATenkaichiTeamPublicInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, TeamDisplayAsset, COND_InitialOnly);
}

// 服务器端设置公开外观：只能在"还没有外观"时设置一次
void ATenkaichiTeamPublicInfo::SetTeamDisplayAsset(TObjectPtr<UTenkaichiTeamDisplayAsset> NewDisplayAsset)
{
	check(HasAuthority());
	check(TeamDisplayAsset == nullptr);

	TeamDisplayAsset = NewDisplayAsset;

	TryRegisterWithTeamSubsystem();
}

// 客户端收到外观数据复制时触发，重新尝试注册
void ATenkaichiTeamPublicInfo::OnRep_TeamDisplayAsset()
{
	TryRegisterWithTeamSubsystem();
}

