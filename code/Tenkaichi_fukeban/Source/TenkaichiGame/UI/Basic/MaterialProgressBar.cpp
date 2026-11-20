// 版权声明（可保留或省略，不影响编译）
#include "MaterialProgressBar.h"

#include "Animation/WidgetAnimation.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MaterialProgressBar)

// 同步属性（材质/参数变化时重建 MID）。
void UMaterialProgressBar::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (Image_Bar)
	{
		// 按是否描边选材质
		Image_Bar->SetBrushFromMaterial(bUseStroke ? StrokeMaterial : NoStrokeMaterial);
		// 清空缓存（材质换了，MID 要重建）
		CachedMID = nullptr;
		CachedProgress = -1.0f;
		CachedStartProgress = -1.0f;

#if WITH_EDITORONLY_DATA
		if (IsDesignTime())
		{
			SetProgress_Internal(DesignTime_Progress);
		}
#endif

		// 把编辑器里配置的参数写进材质
		if (UMaterialInstanceDynamic* MID = GetBarDynamicMaterial())
		{
			if (bOverrideDefaultSegmentEdge)
			{
				MID->SetScalarParameterValue(TEXT("SegmentEdge"), SegmentEdge);
			}

			if (bOverrideDefaultSegments)
			{
				MID->SetScalarParameterValue(TEXT("Segments"), (float)Segments);
			}

			if (bOverrideDefaultFillEdgeSoftness)
			{
				MID->SetScalarParameterValue(TEXT("FillEdgeSoftness"), FillEdgeSoftness);
			}

			if (bOverrideDefaultGlowEdge)
			{
				MID->SetScalarParameterValue(TEXT("GlowEdge"), GlowEdge);
			}

			if (bOverrideDefaultGlowSoftness)
			{
				MID->SetScalarParameterValue(TEXT("GlowSoftness"), GlowSoftness);
			}

			if (bOverrideDefaultOutlineScale)
			{
				MID->SetScalarParameterValue(TEXT("OutlineScale"), OutlineScale);
			}
		}

		if (bOverrideDefaultColorA)
		{
			SetColorA_Internal(CachedColorA);
		}

		if (bOverrideDefaultColorB)
		{
			SetColorB_Internal(CachedColorB);
		}

		if (bOverrideDefaultColorBackground)
		{
			SetColorBackground_Internal(CachedColorBackground);
		}
	}
}

#if WITH_EDITOR
// 编辑器：控件重建时（从材质读取当前参数填入面板）。
void UMaterialProgressBar::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();

	if (IsDesignTime() && Image_Bar)
	{
		if (UMaterialInstanceDynamic* MID = GetBarDynamicMaterial())
		{
			// 没覆盖的项 → 从材质读默认值回填到面板，方便查看
			if (!bOverrideDefaultColorA)
			{
				MID->GetVectorParameterValue(TEXT("ColorA"), CachedColorA);
			}

			if (!bOverrideDefaultColorB)
			{
				MID->GetVectorParameterValue(TEXT("ColorB"), CachedColorB);
			}

			if (!bOverrideDefaultColorBackground)
			{
				MID->GetVectorParameterValue(TEXT("Unfilled Color"), CachedColorBackground);
			}

			if (!bOverrideDefaultSegmentEdge)
			{
				MID->GetScalarParameterValue(TEXT("SegmentEdge"), SegmentEdge);
			}

			if (!bOverrideDefaultSegments)
			{
				float SegmentsFloat;
				MID->GetScalarParameterValue(TEXT("Segments"), SegmentsFloat);
				Segments = FMath::TruncToInt(SegmentsFloat);
			}

			if (!bOverrideDefaultFillEdgeSoftness)
			{
				MID->GetScalarParameterValue(TEXT("FillEdgeSoftness"), FillEdgeSoftness);
			}

			if (!bOverrideDefaultGlowEdge)
			{
				MID->GetScalarParameterValue(TEXT("GlowEdge"), GlowEdge);
			}

			if (!bOverrideDefaultGlowSoftness)
			{
				MID->GetScalarParameterValue(TEXT("GlowSoftness"), GlowSoftness);
			}

			if (!bOverrideDefaultOutlineScale)
			{
				MID->GetScalarParameterValue(TEXT("OutlineScale"), OutlineScale);
			}
		}
	}
}
#endif

// 动画播放完成的回调（填充动画结束时广播）。
void UMaterialProgressBar::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);

	if (BoundAnim_FillBar == Animation)
	{
		OnFillAnimationFinished.Broadcast();
	}
}

// 设置当前进度（0~1）。
void UMaterialProgressBar::SetProgress(float Progress)
{
	if (CachedProgress != Progress)
	{
		SetProgress_Internal(Progress);
	}
}

// 设置起始进度。
void UMaterialProgressBar::SetStartProgress(float StartProgress)
{
	if (CachedStartProgress != StartProgress)
	{
		SetStartProgress_Internal(StartProgress);
	}
}

// 设置填充色 A。
void UMaterialProgressBar::SetColorA(FLinearColor ColorA)
{
	if (CachedColorA != ColorA)
	{
		SetColorA_Internal(ColorA);
	}
}

// 设置填充色 B。
void UMaterialProgressBar::SetColorB(FLinearColor ColorB)
{
	if (CachedColorB != ColorB)
	{
		SetColorB_Internal(ColorB);
	}
}

// 设置背景色。
void UMaterialProgressBar::SetColorBackground(FLinearColor ColorBackground)
{
	if (CachedColorBackground != ColorBackground)
	{
		SetColorBackground_Internal(ColorBackground);
	}
}

// 从指定起点动画到终点。
void UMaterialProgressBar::AnimateProgressFromStart(float Start, float End, float AnimSpeed)
{
	SetStartProgress(Start);
	SetProgress(End);
	PlayAnimation(BoundAnim_FillBar, 0.0f, 1, EUMGSequencePlayMode::Forward, AnimSpeed);
}

// 从当前填充位置动画到终点。
void UMaterialProgressBar::AnimateProgressFromCurrent(float End, float AnimSpeed)
{
	if (UMaterialInstanceDynamic* MID = GetBarDynamicMaterial())
	{
		// 读材质当前的三个参数，算出当前实际填充位置
		const float CurrentStart = MID->K2_GetScalarParameterValue(TEXT("StartProgress"));
		const float CurrentEnd = MID->K2_GetScalarParameterValue(TEXT("Progress"));
		const float CurrentFill = MID->K2_GetScalarParameterValue(TEXT("FillAmount"));
		const float NewStart = FMath::Lerp(CurrentStart, CurrentEnd, CurrentFill);
		AnimateProgressFromStart(NewStart, End, AnimSpeed);
	}
}

// 内部：设置进度。
void UMaterialProgressBar::SetProgress_Internal(float Progress)
{
	if (UMaterialInstanceDynamic* MID = GetBarDynamicMaterial())
	{
		CachedProgress = Progress;
		MID->SetScalarParameterValue(TEXT("Progress"), CachedProgress);
	}
}

// 内部：设置起始进度。
void UMaterialProgressBar::SetStartProgress_Internal(float StartProgress)
{
	if (UMaterialInstanceDynamic* MID = GetBarDynamicMaterial())
	{
		CachedStartProgress = StartProgress;
		MID->SetScalarParameterValue(TEXT("StartProgress"), CachedStartProgress);
	}
}

// 内部：设置填充色 A。
void UMaterialProgressBar::SetColorA_Internal(FLinearColor ColorA)
{
	if (UMaterialInstanceDynamic* MID = GetBarDynamicMaterial())
	{
		CachedColorA = ColorA;
		MID->SetVectorParameterValue(TEXT("ColorA"), CachedColorA);
	}
}

// 内部：设置填充色 B。
void UMaterialProgressBar::SetColorB_Internal(FLinearColor ColorB)
{
	if (UMaterialInstanceDynamic* MID = GetBarDynamicMaterial())
	{
		CachedColorB = ColorB;
		MID->SetVectorParameterValue(TEXT("ColorB"), CachedColorB);
	}
}

// 内部：设置背景色。
void UMaterialProgressBar::SetColorBackground_Internal(FLinearColor ColorBackground)
{
	if (UMaterialInstanceDynamic* MID = GetBarDynamicMaterial())
	{
		CachedColorBackground = ColorBackground;
		MID->SetVectorParameterValue(TEXT("Unfilled Color"), CachedColorBackground);
	}
}

// 取（或懒创建）进度条的动态材质实例。
UMaterialInstanceDynamic* UMaterialProgressBar::GetBarDynamicMaterial() const
{
	if (!CachedMID)
	{
		CachedMID = Image_Bar->GetDynamicMaterial();
	}

	return CachedMID;
}