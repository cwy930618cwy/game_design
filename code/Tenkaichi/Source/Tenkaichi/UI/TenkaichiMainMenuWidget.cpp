#include "TenkaichiMainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiMainMenuWidget)

void UTenkaichiMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 界面显示时，把按钮点击绑到回调（OnClicked 是动态委托，用 AddDynamic）
	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UTenkaichiMainMenuWidget::HandleStartClicked);
	}
}

void UTenkaichiMainMenuWidget::HandleStartClicked()
{
	// 点击"开始游戏"→ 跳转到战斗关卡（对应 GameplayStatics.h 第 340 行 OpenLevel）
	UGameplayStatics::OpenLevel(this, FName(TEXT("L_Battle")));
}