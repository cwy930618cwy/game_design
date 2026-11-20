// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiGameSession.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameSession)


// 构造函数：空实现（父类处理初始化）。
ATenkaichiGameSession::ATenkaichiGameSession(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 覆盖以禁用默认行为（实际登录由 GameMode 管）。
bool ATenkaichiGameSession::ProcessAutoLogin()
{
	// 这实际上由 TenkaichiGameMode::TryDedicatedServerLogin 处理
	return true;
}

// 比赛已开始的钩子。
void ATenkaichiGameSession::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

// 比赛已结束的钩子。
void ATenkaichiGameSession::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
}