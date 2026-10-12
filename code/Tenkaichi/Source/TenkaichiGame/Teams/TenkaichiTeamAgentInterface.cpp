// Copyright Epic Games, Inc. All Rights Reserved.

#include "Teams/TenkaichiTeamAgentInterface.h"

#include "TenkaichiLogChannels.h"
#include "UObject/ScriptInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiTeamAgentInterface)

// 构造函数：交给父类（UGenericTeamAgentInterface）做初始化
UTenkaichiTeamAgentInterface::UTenkaichiTeamAgentInterface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 静态工具：判断队伍真的变了，才广播"队伍变更"委托给所有监听者
void ITenkaichiTeamAgentInterface::ConditionalBroadcastTeamChanged(TScriptInterface<ITenkaichiTeamAgentInterface> This, FGenericTeamId OldTeamID, FGenericTeamId NewTeamID)
{
	if (OldTeamID != NewTeamID)
	{
		// 把引擎队伍ID转成普通整数（没队伍就是 INDEX_NONE）
		const int32 OldTeamIndex = GenericTeamIdToInteger(OldTeamID); 
		const int32 NewTeamIndex = GenericTeamIdToInteger(NewTeamID);

		// 拿到实际的对象（比如某个角色），打日志用
		UObject* ThisObj = This.GetObject();
		UE_LOG(LogTenkaichiTeams, Verbose, TEXT("[%s] %s assigned team %d"), *GetClientServerContextString(ThisObj), *GetPathNameSafe(ThisObj), NewTeamIndex);

		// 拿到"队伍变更委托"并广播：告诉监听者"谁、从旧队、到新队"
		This.GetInterface()->GetTeamChangedDelegateChecked().Broadcast(ThisObj, OldTeamIndex, NewTeamIndex);
	}
}