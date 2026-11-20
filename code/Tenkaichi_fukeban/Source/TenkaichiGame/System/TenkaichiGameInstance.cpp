// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiGameInstance.h"

#include "CommonSessionSubsystem.h"
#include "CommonUserSubsystem.h"
#include "Components/GameFrameworkComponentManager.h"
#include "HAL/IConsoleManager.h"
#include "TenkaichiGameplayTags.h"
#include "Misc/Paths.h"
#include "Player/TenkaichiPlayerController.h"
#include "Player/TenkaichiLocalPlayer.h"
#include "GameFramework/PlayerState.h"

#if UE_WITH_DTLS
#include "DTLSCertStore.h"
#include "DTLSHandlerComponent.h"
#include "Misc/FileHelper.h"
#endif // UE_WITH_DTLS

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameInstance)

namespace Tenkaichi
{
	// 是否测试加密（控制台变量）。
	static bool bTestEncryption = false;
	static FAutoConsoleVariableRef CVarTenkaichiTestEncryption(
		TEXT("Tenkaichi.TestEncryption"),
		bTestEncryption,
		TEXT("If true, clients will send an encryption token with their request to join the server and attempt to encrypt the connection using a debug key. This is NOT SECURE and for demonstration purposes only."),
		ECVF_Default);

#if UE_WITH_DTLS
	// 是否使用 DTLS 加密（控制台变量）。
	static bool bUseDTLSEncryption = false;
	static FAutoConsoleVariableRef CVarTenkaichiUseDTLSEncryption(
		TEXT("Tenkaichi.UseDTLSEncryption"),
		bUseDTLSEncryption,
		TEXT("Set to true if using Tenkaichi.TestEncryption and the DTLS packet handler."),
		ECVF_Default);

	// 是否测试 DTLS 指纹（控制台变量）。
	static bool bTestDTLSFingerprint = false;
	static FAutoConsoleVariableRef CVarTenkaichiTestDTLSFingerprint(
		TEXT("Tenkaichi.TestDTLSFingerprint"),
		bTestDTLSFingerprint,
		TEXT("If true and using DTLS encryption, generate unique cert per connection and fingerprint will be written to file to simulate passing through an online service."),
		ECVF_Default);

#if !UE_BUILD_SHIPPING
	// 控制台命令：生成 DTLS 自签名证书用于测试。
	static FAutoConsoleCommandWithWorldAndArgs CmdGenerateDTLSCertificate(
		TEXT("GenerateDTLSCertificate"),
		TEXT("Generate a DTLS self-signed certificate for testing and export to PEM."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& InArgs, UWorld* InWorld)
			{
				if (InArgs.Num() == 1)
				{
					const FString& CertName = InArgs[0];

					FTimespan CertExpire = FTimespan::FromDays(365);
					TSharedPtr<FDTLSCertificate> Cert = FDTLSCertStore::Get().CreateCert(CertExpire, CertName);
					if (Cert.IsValid())
					{
						const FString CertPath = FPaths::ProjectContentDir() / TEXT("DTLS") / FPaths::MakeValidFileName(FString::Printf(TEXT("%s.pem"), *CertName));

						if (!Cert->ExportCertificate(CertPath))
						{
							UE_LOG(LogTemp, Error, TEXT("GenerateDTLSCertificate: Failed to export certificate."));
						}
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("GenerateDTLSCertificate: Failed to generate certificate."));
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("GenerateDTLSCertificate: Invalid argument(s)."));
				}
			}));
#endif // UE_BUILD_SHIPPING
#endif // UE_WITH_DTLS
};

// 构造函数：空实现（父类处理初始化）。
UTenkaichiGameInstance::UTenkaichiGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 初始化（注册 InitState + 绑会话事件）。
void UTenkaichiGameInstance::Init()
{
	Super::Init();

	// 注册我们自定义的初始化状态
	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(TenkaichiGameplayTags::InitState_Spawned, false, FGameplayTag());
		ComponentManager->RegisterInitState(TenkaichiGameplayTags::InitState_DataAvailable, false, TenkaichiGameplayTags::InitState_Spawned);
		ComponentManager->RegisterInitState(TenkaichiGameplayTags::InitState_DataInitialized, false, TenkaichiGameplayTags::InitState_DataAvailable);
		ComponentManager->RegisterInitState(TenkaichiGameplayTags::InitState_GameplayReady, false, TenkaichiGameplayTags::InitState_DataInitialized);
	}

	// 用固定值初始化 AES256 的调试密钥。这不安全，仅作示例。
	DebugTestEncryptionKey.SetNum(32);

	for (int32 i = 0; i < DebugTestEncryptionKey.Num(); ++i)
	{
		DebugTestEncryptionKey[i] = uint8(i);
	}

	if (UCommonSessionSubsystem* SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>())
	{
		SessionSubsystem->OnPreClientTravelEvent.AddUObject(this, &UTenkaichiGameInstance::OnPreClientTravelToSession);
	}
}

// 关闭（解绑会话事件）。
void UTenkaichiGameInstance::Shutdown()
{
	if (UCommonSessionSubsystem* SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>())
	{
		SessionSubsystem->OnPreClientTravelEvent.RemoveAll(this);
	}

	Super::Shutdown();
}

// 返回主玩家控制器（强类型）。
ATenkaichiPlayerController* UTenkaichiGameInstance::GetPrimaryPlayerController() const
{
	return Cast<ATenkaichiPlayerController>(Super::GetPrimaryPlayerController(false));
}

// 能否加入请求的会话。
bool UTenkaichiGameInstance::CanJoinRequestedSession() const
{
	// 临时第一版：总是返回 true
	// 之后会充实为检查玩家状态
	if (!Super::CanJoinRequestedSession())
	{
		return false;
	}
	return true;
}

// 用户初始化完成的回调（登录成功后加载设置）。
void UTenkaichiGameInstance::HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
	Super::HandlerUserInitialized(UserInfo, bSuccess, Error, RequestedPrivilege, OnlineContext);

	// 如果登录成功，让本地玩家加载它们的设置
	if (bSuccess && ensure(UserInfo))
	{
		UTenkaichiLocalPlayer* LocalPlayer = Cast<UTenkaichiLocalPlayer>(GetLocalPlayerByIndex(UserInfo->LocalPlayerIndex));

		// 专用服务器用户不会有本地玩家附着
		if (LocalPlayer)
		{
			LocalPlayer->LoadSharedSettingsFromDisk();
		}
	}
}

// 收到网络加密令牌（演示代码）。
void UTenkaichiGameInstance::ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate)
{
	// 这是一个简单实现，演示用硬编码密钥对游戏流量加密。
	// 完整实现应该从一个安全来源获取加密密钥（比如 HTTPS 的 web 服务）。
	// 这可以在这个函数里做，甚至异步——只要密钥知道了就调用传入的响应委托。
	// EncryptionToken 的内容由用户决定，但通常会包含用于生成唯一加密密钥的信息（比如用户/会话 ID）。

	FEncryptionKeyResponse Response(EEncryptionResponse::Failure, TEXT("Unknown encryption failure"));

	if (EncryptionToken.IsEmpty())
	{
		Response.Response = EEncryptionResponse::InvalidToken;
		Response.ErrorMsg = TEXT("Encryption token is empty.");
	}
	else
	{
#if UE_WITH_DTLS
		if (Tenkaichi::bUseDTLSEncryption)
		{
			TSharedPtr<FDTLSCertificate> Cert;

			if (Tenkaichi::bTestDTLSFingerprint)
			{
				// 为这个标识符生成服务器证书，发布指纹
				FTimespan CertExpire = FTimespan::FromHours(4);
				Cert = FDTLSCertStore::Get().CreateCert(CertExpire, EncryptionToken);
			}
			else
			{
				// 从磁盘加载证书用于测试（生产环境绝不这么干）
				const FString CertPath = FPaths::ProjectContentDir() / TEXT("DTLS") / TEXT("TenkaichiTest.pem");

				Cert = FDTLSCertStore::Get().GetCert(EncryptionToken);

				if (!Cert.IsValid())
				{
					Cert = FDTLSCertStore::Get().ImportCert(CertPath, EncryptionToken);
				}
			}

			if (Cert.IsValid())
			{
				if (Tenkaichi::bTestDTLSFingerprint)
				{
					// 指纹应该发布到一个安全的 web 服务用于发现
					// 本地测试就写到磁盘
					TArrayView<const uint8> Fingerprint = Cert->GetFingerprint();

					FString DebugFile = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("DTLS")) / FPaths::MakeValidFileName(EncryptionToken) + TEXT("_server.txt");

					FString FingerprintStr = BytesToHex(Fingerprint.GetData(), Fingerprint.Num());
					FFileHelper::SaveStringToFile(FingerprintStr, *DebugFile);
				}

				// 服务器目前只需要标识符
				Response.EncryptionData.Identifier = EncryptionToken;
				Response.EncryptionData.Key = DebugTestEncryptionKey;

				Response.Response = EEncryptionResponse::Success;
			}
			else
			{
				Response.Response = EEncryptionResponse::Failure;
				Response.ErrorMsg = TEXT("Unable to obtain certificate.");
			}
		}
		else
#endif // UE_WITH_DTLS
		{
			Response.Response = EEncryptionResponse::Success;
			Response.EncryptionData.Key = DebugTestEncryptionKey;
		}
	}

	Delegate.ExecuteIfBound(Response);
}

// 收到网络加密确认（演示代码）。
void UTenkaichiGameInstance::ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate)
{
	// 这是一个简单实现，演示用硬编码密钥对游戏流量加密。
	// 完整实现应该从一个安全来源获取加密密钥（比如 HTTPS 的 web 服务）。
	// 这可以在这个函数里做，甚至异步——只要密钥知道了就调用传入的响应委托。

	FEncryptionKeyResponse Response;

#if UE_WITH_DTLS
	if (Tenkaichi::bUseDTLSEncryption)
	{
		Response.Response = EEncryptionResponse::Failure;

		APlayerController* const PlayerController = GetFirstLocalPlayerController();

		if (PlayerController && PlayerController->PlayerState && PlayerController->PlayerState->GetUniqueId().IsValid())
		{
			const FUniqueNetIdRepl& PlayerUniqueId = PlayerController->PlayerState->GetUniqueId();

			// 理想情况下加密令牌是直接传进来的，而不是尝试重建它
			const FString EncryptionToken = PlayerUniqueId.ToString();

			Response.EncryptionData.Identifier = EncryptionToken;

			// 服务器的指纹应该从一个安全服务拉取
			if (Tenkaichi::bTestDTLSFingerprint)
			{
				// 但为了测试……
				FString DebugFile = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("DTLS")) / FPaths::MakeValidFileName(EncryptionToken) + TEXT("_server.txt");
				FString FingerprintStr;
				FFileHelper::LoadFileToString(FingerprintStr, *DebugFile);

				Response.EncryptionData.Fingerprint.AddUninitialized(FingerprintStr.Len() / 2);
				HexToBytes(FingerprintStr, Response.EncryptionData.Fingerprint.GetData());
			}
			else
			{
				// 从磁盘拉取期望的指纹用于测试，这应该来自安全服务
				const FString CertPath = FPaths::ProjectContentDir() / TEXT("DTLS") / TEXT("TenkaichiTest.pem");

				TSharedPtr<FDTLSCertificate> Cert = FDTLSCertStore::Get().GetCert(EncryptionToken);
				if (!Cert.IsValid())
				{
					Cert = FDTLSCertStore::Get().ImportCert(CertPath, EncryptionToken);
				}

				if (Cert.IsValid())
				{
					TArrayView<const uint8> Fingerprint = Cert->GetFingerprint();

					Response.EncryptionData.Fingerprint = Fingerprint;
				}
				else
				{
					Response.Response = EEncryptionResponse::Failure;
					Response.ErrorMsg = TEXT("Unable to obtain certificate.");
				}
			}

			Response.EncryptionData.Key = DebugTestEncryptionKey;

			Response.Response = EEncryptionResponse::Success;
		}
	}
	else
#endif // UE_WITH_DTLS
	{
		Response.Response = EEncryptionResponse::Success;
		Response.EncryptionData.Key = DebugTestEncryptionKey;
	}

	Delegate.ExecuteIfBound(Response);
}

// 客户端传送到会话前（加加密令牌）。
void UTenkaichiGameInstance::OnPreClientTravelToSession(FString& URL)
{
	// 如果需要就加调试加密令牌。
	if (Tenkaichi::bTestEncryption)
	{
#if UE_WITH_DTLS
		if (Tenkaichi::bUseDTLSEncryption)
		{
			APlayerController* const PlayerController = GetFirstLocalPlayerController();

			if (PlayerController && PlayerController->PlayerState && PlayerController->PlayerState->GetUniqueId().IsValid())
			{
				const FUniqueNetIdRepl& PlayerUniqueId = PlayerController->PlayerState->GetUniqueId();
				const FString EncryptionToken = PlayerUniqueId.ToString();

				URL += TEXT("?EncryptionToken=") + EncryptionToken;
			}
		}
		else
#endif // UE_WITH_DTLS
		{
			// 这只是测试/调试用的值，不管令牌值是什么服务器都用同一个密钥。
			// 但令牌可以是用户 ID 和/或会话 ID，用来生成每个用户/会话唯一的密钥（如果需要）。
			URL += TEXT("?EncryptionToken=1");
		}
	}
}