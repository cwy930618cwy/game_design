#pragma once

#include "GameFramework/GameMode.h"
#include "TenkaichiGameMode.generated.h"

class APawn;
class AController;
class UUserWidget;

UCLASS()
class ATenkaichiGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ATenkaichiGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// 游戏开始时，创建并显示主菜单 Widget
	virtual void BeginPlay() override;

	// 主菜单 Widget 类（编辑器里指定为 WBP_MainMenu）
	UPROPERTY(EditAnywhere, Category = "Tenkaichi|UI")
	TSubclassOf<UUserWidget> MenuWidgetClass;
};