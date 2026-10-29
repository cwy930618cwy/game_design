// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiTeamPrivateInfo.h"
#include "Teams/TenkaichiTeamInfoBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiTeamPrivateInfo)

// 构造函数：调用父类构造，私有队伍信息暂与公开信息同样复制（待改用复制图表真正私有化）
ATenkaichiTeamPrivateInfo::ATenkaichiTeamPrivateInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//@TODO: 真正做成私有的（使用复制图表 Replication Graph）
}