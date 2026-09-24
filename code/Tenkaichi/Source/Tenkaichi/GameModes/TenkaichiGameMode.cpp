#include "TenkaichiGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Character/TenkaichiCharacterWithAbilities.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameMode)

ATenkaichiGameMode::ATenkaichiGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 玩家的默认身体 = 我们 P01 建好的带 GAS 角色类（对应 LyraGameMode.cpp 第 41 行）
	DefaultPawnClass = ATenkaichiCharacterWithAbilities::StaticClass();
}

void ATenkaichiGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 只在本地玩家的客户端显示主菜单（GameMode 在服务器/客户端都有，UI 只需本地一份）
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (MenuWidgetClass)
		{
			UTenkaichiMainMenuWidget* MenuWidget = CreateWidget<UTenkaichiMainMenuWidget>(PC, MenuWidgetClass);
			if (MenuWidget)
			{
				MenuWidget->AddToViewport();
			}
		}
	}
}