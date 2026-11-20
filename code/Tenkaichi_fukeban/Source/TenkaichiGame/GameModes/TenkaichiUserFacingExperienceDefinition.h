// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Engine/DataAsset.h"

#include "TenkaichiUserFacingExperienceDefinition.generated.h"

class FString;
class UCommonSession_HostSessionRequest;
class UObject;
class UTexture2D;
class UUserWidget;
struct FFrame;

// 用于在 UI 里展示经验、并开启一场新会话的设置描述。
UCLASS(BlueprintType)
class UTenkaichiUserFacingExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 要加载的具体地图。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience, meta=(AllowedTypes="Map"))
	FPrimaryAssetId MapID;

	// 要加载的玩法经验。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience, meta=(AllowedTypes="TenkaichiExperienceDefinition"))
	FPrimaryAssetId ExperienceID;

	// 作为 URL 选项传给游戏的额外参数。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	TMap<FString, FString> ExtraArgs;

	// UI 里的主标题。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	FText TileTitle;

	// 副标题。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	FText TileSubTitle;

	// 完整描述。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	FText TileDescription;

	// UI 里用的图标。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	TObjectPtr<UTexture2D> TileIcon;

	// 加载进入（或退出）某个经验时要显示的加载屏控件。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=LoadingScreen)
	TSoftClassPtr<UUserWidget> LoadingScreenWidget;

	// 如果为 true，这是一个默认经验，应该用于快速游玩并在 UI 里优先展示。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	bool bIsDefaultExperience = false;

	// 如果为 true，这会出现在前端的经验列表里。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	bool bShowInFrontEnd = true;

	// 如果为 true，会为这场游戏录制回放。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	bool bRecordReplay = false;

	// 这场会话的最大玩家数。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	int32 MaxPlayerCount = 16;

public:
	// 创建一个请求对象，用于按这些设置真正开启一场会话。
	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta = (WorldContext = "WorldContextObject"))
	UCommonSession_HostSessionRequest* CreateHostingRequest(const UObject* WorldContextObject) const;
};