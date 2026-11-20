#pragma once

#include "TenkaichiTabListWidgetBase.h"
#include "UI/Foundation/TenkaichiButtonBase.h"

#include "TenkaichiTabButtonBase.generated.h"

#define UE_API TENKAICHIGAME_API

class UCommonLazyImage;
class UObject;
struct FFrame;
struct FSlateBrush;

// 标签按钮基类：既是按钮，又能显示标签图标。
UCLASS(MinimalAPI, Abstract, Blueprintable, meta = (DisableNativeTick))
class UTenkaichiTabButtonBase : public UTenkaichiButtonBase, public ITenkaichiTabButtonInterface
{
	GENERATED_BODY()

public:
	// 从懒加载对象设置图标（软引用，按需加载）。
	UE_API void SetIconFromLazyObject(TSoftObjectPtr<UObject> LazyObject);
	// 直接设置图标画刷。
	UE_API void SetIconBrush(const FSlateBrush& Brush);

protected:
	// 实现接口：接收标签描述（填文字 + 图标）。
	UFUNCTION()
	UE_API virtual void SetTabLabelInfo_Implementation(const FTenkaichiTabDescriptor& TabLabelInfo) override;

private:
	// 图标（懒加载图片控件，可选绑定）。
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonLazyImage> LazyImage_Icon;
};

#undef UE_API