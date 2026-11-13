// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Components/Widget.h"
#include "GameplayTagContainer.h"

#include "TenkaichiHitMarkerConfirmationWidget.generated.h"

class SHitMarkerConfirmationWidget;
class SWidget;
class UObject;
struct FGameplayTag;

// 命中标记的 UMG 包装（编辑器可配参数，内部用 Slate 绘制）。
UCLASS()
class UTenkaichiHitMarkerConfirmationWidget : public UWidget
{
	GENERATED_BODY()

public:
	// 构造函数：设默认可见性 + 易变标记 + 默认不画"任意命中"图。
	UTenkaichiHitMarkerConfirmationWidget(const FObjectInitializer& ObjectInitializer);

	//~UWidget 接口开始
protected:
	// 重建 Slate 控件（创建 S 层实例，并传自定义参数）。
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~UWidget 接口结束

	//~UVisual 接口开始
public:
	// 释放 Slate 资源。
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~UVisual 接口结束
	
public:
	/** 命中通知的显示时长（秒），期间淡出到透明 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance, meta=(ClampMin=0.0, ForceUnits=s))
	float HitNotifyDuration = 0.4f;

	/** 单次命中要绘制的标记图片 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FSlateBrush PerHitMarkerImage;

	/** 部位 tag（比如弱点）→ 覆盖用标记图片 的映射 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	TMap<FGameplayTag, FSlateBrush> PerHitMarkerZoneOverrideImages;

	/** 只要有命中就绘制的标记图片 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	FSlateBrush AnyHitsMarkerImage;

private:
	/** 内部 Slate 控件，代表实际的标记视觉 */
	TSharedPtr<SHitMarkerConfirmationWidget> MyMarkerWidget;
};