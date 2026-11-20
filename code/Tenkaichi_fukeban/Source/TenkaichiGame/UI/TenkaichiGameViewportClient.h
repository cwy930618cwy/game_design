// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonGameViewportClient.h"

#include "TenkaichiGameViewportClient.generated.h"

class UGameInstance;
class UObject;

// 游戏视口客户端：按平台决定用软件光标还是硬件光标。
UCLASS(BlueprintType)
class UTenkaichiGameViewportClient : public UCommonGameViewportClient
{
	GENERATED_BODY()

public:
	// 构造函数。
	UTenkaichiGameViewportClient();

	// 初始化视口客户端（按平台设置光标模式）。
	virtual void Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice = true) override;
};