// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"

#include "TenkaichiTaggedWidget.generated.h"

class UObject;

/**
 * 布局里一个带 tag 的控件（可以通过拥有玩家身上的 tag 来隐藏或显示）
 */
UCLASS(Abstract, Blueprintable)
class UTenkaichiTaggedWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 构造函数：空实现。
	UTenkaichiTaggedWidget(const FObjectInitializer& ObjectInitializer);

	//~UWidget 接口开始
	// 设置可见性（会记住调用者的意图，再按 tag 决定实际可见性）。
	virtual void SetVisibility(ESlateVisibility InVisibility) override;
	//~UWidget 接口结束

	//~UUserWidget 接口开始
	// 控件构造完成（初始化可见性）。
	virtual void NativeConstruct() override;
	// 控件销毁。
	virtual void NativeDestruct() override;
	//~UUserWidget 接口结束

protected:
	/** 如果拥有玩家有这些 tag 里的任意一个，这个控件就会被隐藏（用 HiddenVisibility） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	FGameplayTagContainer HiddenByTags;

	/** 这个控件显示时（没被 gameplay tag 隐藏）用的可见性。 */
	UPROPERTY(EditAnywhere, Category = "HUD")
	ESlateVisibility ShownVisibility = ESlateVisibility::Visible;

	/** 这个控件被 gameplay tag 隐藏时用的可见性。 */
	UPROPERTY(EditAnywhere, Category = "HUD")
	ESlateVisibility HiddenVisibility = ESlateVisibility::Collapsed;

	/** 我们想不想可见（忽略 tag）？ */
	bool bWantsToBeVisible = true;

private:
	// 监听的 tag 变化时的回调。
	void OnWatchedTagsChanged();
};