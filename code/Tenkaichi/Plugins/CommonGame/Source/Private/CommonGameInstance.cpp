// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonGameInstance.h"

#include "CommonLocalPlayer.h"
#include "CommonSessionSubsystem.h"
#include "CommonUISettings.h"
#include "CommonUserSubsystem.h"
#include "GameUIManagerSubsystem.h"
#include "ICommonUIModule.h"
#include "LogCommonGame.h"
#include "Messaging/CommonGameDialog.h"
#include "Messaging/CommonMessagingSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonGameInstance)

UCommonGameInstance::UCommonGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UCommonGameInstance::HandleSystemMessage(FGameplayTag MessageType, FText Title, FText Message)
{
	ULocalPlayer* FirstPlayer = GetFirstGamePlayer();
	// 把严重的消息转发给第一个玩家的错误对话框
	if (FirstPlayer && MessageType.MatchesTag(FCommonUserTags::SystemMessage_Error))
	{
		if (UCommonMessagingSubsystem* Messaging = FirstPlayer->GetSubsystem<UCommonMessagingSubsystem>())
		{
			Messaging->ShowError(UCommonGameDialogDescriptor::CreateConfirmationOk(Title, Message));
		}
	}
}

void UCommonGameInstance::HandlePrivilegeChanged(const UCommonUserInfo* UserInfo, ECommonUserPrivilege Privilege, ECommonUserAvailability OldAvailability, ECommonUserAvailability NewAvailability)
{
	// 默认情况下，如果第一个玩家丢失了游玩权限，就显示错误并断开连接
	if (Privilege == ECommonUserPrivilege::CanPlay && OldAvailability == ECommonUserAvailability::NowAvailable && NewAvailability != ECommonUserAvailability::NowAvailable)
	{
		UE_LOG(LogCommonGame, Error, TEXT("HandlePrivilegeChanged: Player %d no longer has permission to play the game!"), UserInfo->LocalPlayerIndex);
		// TODO: 游戏可以在子类里做特定处理
		// ReturnToMainMenu();
	}
}

void UCommonGameInstance::HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
	// 子类可以重写这个
}

int32 UCommonGameInstance::AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId)
{
	int32 ReturnVal = Super::AddLocalPlayer(NewPlayer, UserId);
	if (ReturnVal != INDEX_NONE)
	{
		if (!PrimaryPlayer.IsValid())
		{
			UE_LOG(LogCommonGame, Log, TEXT("AddLocalPlayer: Set %s to Primary Player"), *NewPlayer->GetName());
			PrimaryPlayer = NewPlayer;
		}
		
		GetSubsystem<UGameUIManagerSubsystem>()->NotifyPlayerAdded(Cast<UCommonLocalPlayer>(NewPlayer));
	}
	
	return ReturnVal;
}

bool UCommonGameInstance::RemoveLocalPlayer(ULocalPlayer* ExistingPlayer)
{
	if (PrimaryPlayer == ExistingPlayer)
	{
		//TODO: 我们是否要回退到另一个玩家？
		PrimaryPlayer.Reset();
		UE_LOG(LogCommonGame, Log, TEXT("RemoveLocalPlayer: Unsetting Primary Player from %s"), *ExistingPlayer->GetName());
	}
	GetSubsystem<UGameUIManagerSubsystem>()->NotifyPlayerDestroyed(Cast<UCommonLocalPlayer>(ExistingPlayer));

	return Super::RemoveLocalPlayer(ExistingPlayer);
}

void UCommonGameInstance::Init()
{
	Super::Init();

	// 在子系统初始化之后，把它们串起来
	FGameplayTagContainer PlatformTraits = ICommonUIModule::GetSettings().GetPlatformTraits();

	UCommonUserSubsystem* UserSubsystem = GetSubsystem<UCommonUserSubsystem>();
	if (ensure(UserSubsystem))
	{
		UserSubsystem->SetTraitTags(PlatformTraits);
		UserSubsystem->OnHandleSystemMessage.AddDynamic(this, &UCommonGameInstance::HandleSystemMessage);
		UserSubsystem->OnUserPrivilegeChanged.AddDynamic(this, &UCommonGameInstance::HandlePrivilegeChanged);
		UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &UCommonGameInstance::HandlerUserInitialized);
	}

	UCommonSessionSubsystem* SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>();
	if (ensure(SessionSubsystem))
	{
		SessionSubsystem->OnUserRequestedSessionEvent.AddUObject(this, &UCommonGameInstance::OnUserRequestedSession);
		SessionSubsystem->OnDestroySessionRequestedEvent.AddUObject(this, &UCommonGameInstance::OnDestroySessionRequested);
	}
}

void UCommonGameInstance::ResetUserAndSessionState()
{
	UCommonUserSubsystem* UserSubsystem = GetSubsystem<UCommonUserSubsystem>();
	if (ensure(UserSubsystem))
	{
		UserSubsystem->ResetUserState();
	}

	UCommonSessionSubsystem* SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>();
	if (ensure(SessionSubsystem))
	{
		SessionSubsystem->CleanUpSessions();
	}
}

void UCommonGameInstance::ReturnToMainMenu()
{
	// 默认情况下，返回主菜单时我们应该重置一切
	ResetUserAndSessionState();

	Super::ReturnToMainMenu();
}

void UCommonGameInstance::OnUserRequestedSession(const FPlatformUserId& PlatformUserId, UCommonSession_SearchResult* InRequestedSession, const FOnlineResultInformation& RequestedSessionResult)
{
	if (InRequestedSession)
	{
		SetRequestedSession(InRequestedSession);
	}
	else
	{
		HandleSystemMessage(FCommonUserTags::SystemMessage_Error, NSLOCTEXT("CommonGame", "Warning_RequestedSessionFailed", "Requested Session Failed"), RequestedSessionResult.ErrorText);
	}
}

void UCommonGameInstance::OnDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName)
{
	// 当请求销毁会话时，请确保你的项目处于能够销毁会话并从中过渡出去的正确状态

	UE_LOG(LogCommonGame, Verbose, TEXT("[%hs] PlatformUserId:%d, SessionName: %s)"), __FUNCTION__, PlatformUserId.GetInternalId(), *SessionName.ToString());

	ReturnToMainMenu();
}

void UCommonGameInstance::SetRequestedSession(UCommonSession_SearchResult* InRequestedSession)
{
	RequestedSession = InRequestedSession;
	if (RequestedSession)
	{
		if (CanJoinRequestedSession())
		{
			JoinRequestedSession();
		}
		else
		{
			ResetGameAndJoinRequestedSession();
		}
	}
}

bool UCommonGameInstance::CanJoinRequestedSession() const
{
	// 默认行为是始终允许加入所请求的会话
	return true;
}

void UCommonGameInstance::JoinRequestedSession()
{
	if (RequestedSession)
	{
		if (ULocalPlayer* const FirstPlayer = GetFirstGamePlayer())
		{
			UCommonSessionSubsystem* SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>();
			if (ensure(SessionSubsystem))
			{
				// 清除我们当前所请求的会话，因为我们现在正在对它进行操作。
				UCommonSession_SearchResult* LocalRequestedSession = RequestedSession;
				RequestedSession = nullptr;
				SessionSubsystem->JoinSession(FirstPlayer->PlayerController, LocalRequestedSession);
			}
		}
	}
}

void UCommonGameInstance::ResetGameAndJoinRequestedSession()
{
	// 默认行为是返回主菜单。游戏必须在处于就绪状态时调用 JoinRequestedSession。
	ReturnToMainMenu();
}


//void UCommonGameInstance::OnPreLoadMap(const FString& MapName)
//{
//	if (!IsDedicatedServerInstance())
//	{
//		if (!bWasInLoadMap)
//		{
//			UGameUIManagerSubsystem* UIManager = GetSubsystem<UGameUIManagerSubsystem>();
//			for (ULocalPlayer* LocalPlayer : LocalPlayers)
//			{
//				UIManager->NotifyPlayerAdded(Cast<UCommonLocalPlayer>(LocalPlayer));
//			}
//		}
//	}
//}
er));
//			}
//		}
//	}
//}
