// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonUserWidget.h"

#include "MaterialProgressBar.generated.h"

class UImage;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UWidgetAnimation;

// 材质进度条：用材质参数控制填充/颜色/分段/光晕。
UCLASS(Abstract, meta = (DisableNativeTick))
class UMaterialProgressBar : public UCommonUserWidget
{
	GENERATED_BODY()

protected:

	// 同步属性（材质/参数变化时重建 MID）。
	virtual void SynchronizeProperties() override;

#if WITH_EDITOR
	// 编辑器：控件重建时（从材质读取当前参数填入面板）。
	virtual void OnWidgetRebuilt() override;
#endif

	// 动画播放完成的回调（填充动画结束时广播）。
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

public:

	// 设置当前进度（0~1）。
	UFUNCTION(BlueprintCallable)
	void SetProgress(float Progress);

	// 设置起始进度（进度从哪开始填充）。
	UFUNCTION(BlueprintCallable)
	void SetStartProgress(float StartProgress);

	// 设置填充色 A。
	UFUNCTION(BlueprintCallable)
	void SetColorA(FLinearColor ColorA);

	// 设置填充色 B。
	UFUNCTION(BlueprintCallable)
	void SetColorB(FLinearColor ColorB);

	// 设置背景色。
	UFUNCTION(BlueprintCallable)
	void SetColorBackground(FLinearColor ColorBackground);

	// 从指定起点动画到终点。
	UFUNCTION(BlueprintCallable)
	void AnimateProgressFromStart(float Start, float End, float AnimSpeed = 1.0f);

	// 从当前填充位置动画到终点。
	UFUNCTION(BlueprintCallable)
	void AnimateProgressFromCurrent(float End, float AnimSpeed = 1.0f);

	// 填充动画完成的委托。
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFillAnimationFinished);

	// 填充动画完成时广播。
	UPROPERTY(BlueprintAssignable)
	FOnFillAnimationFinished OnFillAnimationFinished;

private:

	// 内部：设置进度。
	void SetProgress_Internal(float Progress);
	// 内部：设置起始进度。
	void SetStartProgress_Internal(float StartProgress);
	// 内部：设置填充色 A。
	void SetColorA_Internal(FLinearColor ColorA);
	// 内部：设置填充色 B。
	void SetColorB_Internal(FLinearColor ColorB);
	// 内部：设置背景色。
	void SetColorBackground_Internal(FLinearColor ColorBackground);

	// 取（或懒创建）进度条的动态材质实例。
	UMaterialInstanceDynamic* GetBarDynamicMaterial() const;

	// 是否覆盖默认填充色 A。
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "CachedColorA"))
	bool bOverrideDefaultColorA = false;

	// 填充色 A（勾选覆盖时使用）。
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Color A", EditCondition = "bOverrideDefaultColorA"))
	FLinearColor CachedColorA;

	// 是否覆盖默认填充色 B。
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "CachedColorB"))
	bool bOverrideDefaultColorB = false;

	// 填充色 B（勾选覆盖时使用）。
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Color B", EditCondition = "bOverrideDefaultColorB"))
	FLinearColor CachedColorB;

	// 是否覆盖默认背景色。
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "CachedColorBackground"))
	bool bOverrideDefaultColorBackground;

	// 背景色（勾选覆盖时使用）。
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Color Background", EditCondition = "bOverrideDefaultColorBackground"))
	FLinearColor CachedColorBackground;

	// 是否覆盖默认分段数。
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "Segments"))
	bool bOverrideDefaultSegments = false;

	// 分段数（勾选覆盖时使用）。
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideDefaultSegments"))
	int32 Segments;

	// 是否覆盖默认分段边缘。
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "SegmentEdge"))
	bool bOverrideDefaultSegmentEdge = false;

	// 分段边缘（勾选覆盖时使用）。
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideDefaultSegmentEdge"))
	float SegmentEdge;

	// 是否覆盖默认填充边缘柔化。
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "FillEdgeSoftness"))
	bool bOverrideDefaultFillEdgeSoftness;

	// 填充边缘柔化（勾选覆盖时使用）。
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideDefaultFillEdgeSoftness"))
	float FillEdgeSoftness;

	// 是否覆盖默认光晕边缘。
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "GlowEdge"))
	bool bOverrideDefaultGlowEdge = false;

	// 光晕边缘（勾选覆盖时使用）。
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideDefaultGlowEdge"))
	float GlowEdge;

	// 是否覆盖默认光晕柔化。
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "GlowSoftness"))
	bool bOverrideDefaultGlowSoftness = false;

	// 光晕柔化（勾选覆盖时使用）。
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideDefaultGlowSoftness"))
	float GlowSoftness;

	// 是否覆盖默认描边比例。
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "OutlineScale"))
	bool bOverrideDefaultOutlineScale = false;

	// 描边比例（勾选覆盖时使用）。
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideDefaultOutlineScale"))
	float OutlineScale;

	// 是否使用描边。
	UPROPERTY(EditAnywhere)
	bool bUseStroke = true;

	// 有描边的材质。
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMaterialInterface> StrokeMaterial;

	// 无描边的材质。
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMaterialInterface> NoStrokeMaterial;

#if WITH_EDITORONLY_DATA
	// 设计时进度（编辑器里预览用）。
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Design Time Progress"))
	float DesignTime_Progress = 1.0f;
#endif

	// 进度条图片（必需绑定）。
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess))
	TObjectPtr<UImage> Image_Bar;

	// 填充动画（必需绑定动画）。
	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnim, AllowPrivateAccess))
	TObjectPtr<UWidgetAnimation> BoundAnim_FillBar;

	// 缓存的动态材质实例。
	UPROPERTY(Transient)
	mutable TObjectPtr<UMaterialInstanceDynamic> CachedMID;

	// 缓存的进度（用于比较是否变化）。
	float CachedProgress = -1.0f;
	// 缓存的起始进度。
	float CachedStartProgress = -1.0f;
};