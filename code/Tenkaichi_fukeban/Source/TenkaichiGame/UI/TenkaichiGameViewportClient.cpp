// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiGameViewportClient.h"

#include "CommonUISettings.h"
#include "ICommonUIModule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameViewportClient)

class UGameInstance;

// 视口相关的本地 tag 命名空间。
namespace GameViewportTags
{
	// 定义"平台支持硬件光标"的 trait tag。
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_Input_HardwareCursor, "Platform.Trait.Input.HardwareCursor");
}

// 构造函数。
UTenkaichiGameViewportClient::UTenkaichiGameViewportClient()
	: Super(FObjectInitializer::Get())
{
}

// 初始化视口客户端（按平台设置光标模式）。
void UTenkaichiGameViewportClient::Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice)
{
	Super::Init(WorldContext, OwningGameInstance, bCreateNewAudioDevice);
	
	// 我们在项目设置里为主机/移动端准备了软件光标，但桌面端用标准硬件光标就行。
	const bool UseHardwareCursor = ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(GameViewportTags::TAG_Platform_Trait_Input_HardwareCursor);
	SetUseSoftwareCursorWidgets(!UseHardwareCursor);
}