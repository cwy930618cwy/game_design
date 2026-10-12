// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GenericTeamAgentInterface.h"
#include "UObject/Object.h"

#include "UObject/WeakObjectPtr.h"
#include "TenkaichiTeamAgentInterface.generated.h"

#define UE_API TENKAICHIGAME_API

template <typename InterfaceType> class TScriptInterface;

// 队伍变更通知委托：当某个对象换了队伍时，通过它广播（旧队伍→新队伍）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTenkaichiTeamIndexChangedDelegate, UObject*, ObjectChangingTeam, int32, OldTeamID, int32, NewTeamID);

// 把引擎的队伍ID（FGenericTeamId）转成普通整数；没队伍返回 INDEX_NONE（-1）
inline int32 GenericTeamIdToInteger(FGenericTeamId ID)
{
	return (ID == FGenericTeamId::NoTeam) ? INDEX_NONE : (int32)ID;
}

// 把普通整数转回引擎的队伍ID；INDEX_NONE 表示没有队伍
inline FGenericTeamId IntegerToGenericTeamId(int32 ID)
{
	return (ID == INDEX_NONE) ? FGenericTeamId::NoTeam : FGenericTeamId((uint8)ID);
}

/** 供能被划入某个队伍的对象实现的接口 */
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UTenkaichiTeamAgentInterface : public UGenericTeamAgentInterface
{
	GENERATED_UINTERFACE_BODY()
};

class ITenkaichiTeamAgentInterface : public IGenericTeamAgentInterface
{
	GENERATED_IINTERFACE_BODY()

	// 返回"队伍变更委托"的指针（实现类覆写；默认无，返回空）
	virtual FOnTenkaichiTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() { return nullptr; }

	// 静态工具：判断队伍确实变了，才广播"队伍变更"给监听者
	static UE_API void ConditionalBroadcastTeamChanged(TScriptInterface<ITenkaichiTeamAgentInterface> This, FGenericTeamId OldTeamID, FGenericTeamId NewTeamID);
	
	// 获取"队伍变更委托"并断言其非空（实现类必须已覆写，否则崩溃）
	FOnTenkaichiTeamIndexChangedDelegate& GetTeamChangedDelegateChecked()
	{
		FOnTenkaichiTeamIndexChangedDelegate* Result = GetOnTeamIndexChangedDelegate();
		check(Result);
		return *Result;
	}
};

#undef UE_API