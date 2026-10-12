// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiLogChannels.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogTenkaichi);
DEFINE_LOG_CATEGORY(LogTenkaichiExperience);
DEFINE_LOG_CATEGORY(LogTenkaichiAbilitySystem);
DEFINE_LOG_CATEGORY(LogTenkaichiTeams);

// 根据对象当前运行在网络中的角色（服务器/客户端），返回一个用于日志前缀的上下文字符串
FString GetClientServerContextString(UObject* ContextObject)
{
	ENetRole Role = ROLE_None;

	if (AActor* Actor = Cast<AActor>(ContextObject))
	{
		Role = Actor->GetLocalRole();
	}
	else if (UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None)
	{
		return (Role == ROLE_Authority) ? TEXT("Server") : TEXT("Client");
	}
	else
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			return GPlayInEditorContextString;
		}
#endif
	}

	return TEXT("[]");
}