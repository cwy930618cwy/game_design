#pragma once

#include "GameFramework/GameMode.h"
#include "TenkaichiGameMode.generated.h"

class APawn;
class AController;

UCLASS()
class ATenkaichiGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	// 一比一还原 Lyra 的构造函数签名（去掉插件专用的 UE_API 导出宏）
	ATenkaichiGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};