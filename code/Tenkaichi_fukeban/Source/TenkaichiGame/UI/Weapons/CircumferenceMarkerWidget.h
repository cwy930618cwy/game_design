// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Components/Widget.h"
#include "UI/Weapons/SCircumferenceMarkerWidget.h"

#include "CircumferenceMarkerWidget.generated.h"

class SWidget;
class UObject;
struct FFrame;

// 圆周标记的 UMG 包装（编辑器可配参数，内部用 Slate 绘制）。
UCLASS()
class UCircumferenceMarkerWidget : public UWidget
{
	GENERATED_BODY()

public:
	// 构造函数：设默认可见性 + 易变标记。
	UCircumferenceMarkerWidget(const FObjectInitializer& ObjectInitializer);

	//~UWidget 接口开始
public:
	// 属性同步（把参数转发给 Slate 控件）。
	virtual void SynchronizeProperties() override;
protected:
	// 重建 Slate 控件（创建 S 层实例）。
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~UWidget 接口结束

	//~UVisual 接口开始
public:
	// 释放 Slate 资源（控件销毁时重置指针）。
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~UVisual 接口结束
	
public:
	/** 要绘制标记的位置/朝向列表。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	TArray<FCircumferenceMarkerEntry> MarkerList;

	/** 圆的半径。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance, meta=(ClampMin=0.0))
	float Radius = 48.0f;

	/** 绕圆周放置的标记图片。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FSlateBrush MarkerImage;

	/** 准星角标是否放在散布半径之外。 */
	//@TODO: 把它改成一个 0~1 的浮点对齐值？(比如：在内/在圆上/在外)
	UPROPERTY(EditAnywhere, Category=Corner)
	uint8 bReticleCornerOutsideSpreadRadius : 1;

public:
	/** 设置圆的半径。 */
	UFUNCTION(BlueprintCallable, Category = "Appearance")
	void SetRadius(float InRadius);

private:
	/** 内部 Slate 控件，代表实际的标记视觉。 */
	TSharedPtr<SCircumferenceMarkerWidget> MyMarkerWidget;
};