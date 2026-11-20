// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "GameFramework/GameSession.h"

#include "TenkaichiGameSession.generated.h"

class UObject;


// 游戏会话类（覆盖默认行为，登录交给 GameMode 管）。
UCLASS(Config = Game)
class ATenkaichiGameSession : public AGameSession
{
	GENERATED_BODY()

public:

	// 构造函数：空实现（父类处理初始化）。
	ATenkaichiGameSession(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	// 覆盖以禁用默认行为。
	virtual bool ProcessAutoLogin() override;

	// 比赛已开始的钩子。
	virtual void HandleMatchHasStarted() override;
	// 比赛已结束的钩子。
	virtual void HandleMatchHasEnded() override;
};