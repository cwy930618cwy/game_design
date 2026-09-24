#pragma once

#include "Blueprint/UserWidget.h"
#include "TenkaichiMainMenuWidget.generated.h"

class UButton;

UCLASS()
class UTenkaichiMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 界面显示时触发，用来绑按钮点击（对应 UserWidget.h 第 1572 行 NativeConstruct）
	virtual void NativeConstruct() override;

	// 按钮成员：标 BindWidget，蓝图里同名按钮会自动绑上来
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> StartButton;

	// 按钮点击的回调函数（P03-2 里实现：调 OpenLevel 跳关卡）
	UFUNCTION()
	void HandleStartClicked();
};