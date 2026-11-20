// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiTeamInfoBase.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Teams/TenkaichiTeamSubsystem.h"   // ⚠️ 依赖 TeamSubsystem（4.5 才就位）

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiTeamInfoBase)

class FLifetimeProperty;

// 构造函数：队号初始化为"无队伍"，开启复制
ATenkaichiTeamInfoBase::ATenkaichiTeamInfoBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TeamId(INDEX_NONE)
{
	bReplicates = true;
	bAlwaysRelevant = true;
	NetPriority = 3.0f;
	SetReplicatingMovement(false);
}

// 声明哪些字段要复制（队号只在初次复制，标签栈持续复制）
void ATenkaichiTeamInfoBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TeamTags);
	DOREPLIFETIME_CONDITION(ThisClass, TeamId, COND_InitialOnly);
}

// 游戏开始时，尝试把自己的队伍信息注册到队伍子系统
void ATenkaichiTeamInfoBase::BeginPlay()
{
	Super::BeginPlay();

	TryRegisterWithTeamSubsystem();
}

// 结束时，若已注册则从队伍子系统注销
void ATenkaichiTeamInfoBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TeamId != INDEX_NONE)
	{
		UTenkaichiTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UTenkaichiTeamSubsystem>();
		if (TeamSubsystem)
		{
			// EndPlay 可能在子系统已被销毁的诡异时机触发，所以判空
			TeamSubsystem->UnregisterTeamInfo(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

// 把"这支队伍"注册到指定的队伍子系统（派生类可覆写）
void ATenkaichiTeamInfoBase::RegisterWithTeamSubsystem(UTenkaichiTeamSubsystem* Subsystem)
{
	Subsystem->RegisterTeamInfo(this);
}

// 若已有队号，则尝试注册到世界里的队伍子系统
void ATenkaichiTeamInfoBase::TryRegisterWithTeamSubsystem()
{
	if (TeamId != INDEX_NONE)
	{
		UTenkaichiTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UTenkaichiTeamSubsystem>();
		if (ensure(TeamSubsystem))
		{
			RegisterWithTeamSubsystem(TeamSubsystem);
		}
	}
}

// 服务器端设置队号：只能在"还没有队号"时设置一次
void ATenkaichiTeamInfoBase::SetTeamId(int32 NewTeamId)
{
	check(HasAuthority());
	check(TeamId == INDEX_NONE);
	check(NewTeamId != INDEX_NONE);

	TeamId = NewTeamId;

	TryRegisterWithTeamSubsystem();
}

// 客户端收到队号复制时触发，重新尝试注册
void ATenkaichiTeamInfoBase::OnRep_TeamId()
{
	TryRegisterWithTeamSubsystem();
}

