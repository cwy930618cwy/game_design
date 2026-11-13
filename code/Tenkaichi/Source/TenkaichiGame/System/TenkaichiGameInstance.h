// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonGameInstance.h"

#include "TenkaichiGameInstance.generated.h"

#define UE_API TENKAICHIGAME_API

class ATenkaichiPlayerController;
class UObject;

// 游戏实例：整个游戏进程的生命周期管理者。
UCLASS(MinimalAPI, Config = Game)
class UTenkaichiGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

public:

	// 构造函数：空实现（父类处理初始化）。
	UE_API UTenkaichiGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 返回主玩家控制器（强类型）。
	UE_API ATenkaichiPlayerController* GetPrimaryPlayerController() const;

	// 能否加入请求的会话。
	UE_API virtual bool CanJoinRequestedSession() const override;
	// 用户初始化完成的回调。
	UE_API virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext) override;

	// 收到网络加密令牌。
	UE_API virtual void ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate) override;
	// 收到网络加密确认。
	UE_API virtual void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate) override;

protected:

	// 初始化（注册 InitState + 绑会话事件）。
	UE_API virtual void Init() override;
	// 关闭（解绑会话事件）。
	UE_API virtual void Shutdown() override;

	// 客户端传送到会话前（加加密令牌）。
	UE_API void OnPreClientTravelToSession(FString& URL);

	// 一个硬编码的加密密钥，用来试用加密代码。这是不安全的，生产环境别用这个技术！
	TArray<uint8> DebugTestEncryptionKey;
};

#undef UE_API