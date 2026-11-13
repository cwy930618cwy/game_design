// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiUserFacingExperienceDefinition.h"
#include "CommonSessionSubsystem.h"
#include "Containers/UnrealString.h"
#include "UObject/NameTypes.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
//#include "Replays/TenkaichiReplaySubsystem.h"   // ⚠️ 注释挂起：依赖 P17 ReplaySubsystem

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiUserFacingExperienceDefinition)

// 创建一个请求对象，用于按这些设置真正开启一场会话。
UCommonSession_HostSessionRequest* UTenkaichiUserFacingExperienceDefinition::CreateHostingRequest(const UObject* WorldContextObject) const
{
	const FString ExperienceName = ExperienceID.PrimaryAssetName.ToString();
	const FString UserFacingExperienceName = GetPrimaryAssetId().PrimaryAssetName.ToString();

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UCommonSession_HostSessionRequest* Result = nullptr;

	if (UCommonSessionSubsystem* Subsystem = GameInstance ? GameInstance->GetSubsystem<UCommonSessionSubsystem>() : nullptr)
	{
		Result = Subsystem->CreateOnlineHostSessionRequest();
	}

	if (!Result)
	{
		// 用不了子系统，就自己新建一个
		Result = NewObject<UCommonSession_HostSessionRequest>();
		Result->OnlineMode = ECommonSessionOnlineMode::Online;
		Result->bUseLobbies = true;
		Result->bUseLobbiesVoiceChat = false;
		// 我们总是为这个会话启用 presence，因为它是用于匹配的主会话。
		// 对于在意 presence 的在线系统，只有主会话才应该启用 presence。
		Result->bUsePresence = !IsRunningDedicatedServer();
	}
	Result->MapID = MapID;
	Result->ModeNameForAdvertisement = UserFacingExperienceName;
	Result->ExtraArgs = ExtraArgs;
	Result->ExtraArgs.Add(TEXT("Experience"), ExperienceName);
	Result->MaxPlayerCount = MaxPlayerCount;

#if 0
	// ⚠️ 注释挂起：依赖 P17 UTenkaichiReplaySubsystem（DoesPlatformSupportReplays）
	if (UTenkaichiReplaySubsystem::DoesPlatformSupportReplays())
	{
		if (bRecordReplay)
		{
			Result->ExtraArgs.Add(TEXT("DemoRec"), FString());
		}
	}
#endif

	return Result;
}