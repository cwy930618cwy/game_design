// 版权声明（可保留或省略，不影响编译）
#include "SHitMarkerConfirmationWidget.h"

#include "Weapons/TenkaichiWeaponStateComponent.h"

class FPaintArgs;

// 默认构造。
SHitMarkerConfirmationWidget::SHitMarkerConfirmationWidget()
{
}

// Construct（多两个自定义参数）。
void SHitMarkerConfirmationWidget::Construct(const FArguments& InArgs, const FLocalPlayerContext& InContext, const TMap<FGameplayTag, FSlateBrush>& ZoneOverrideImages)
{
	PerHitMarkerImage = InArgs._PerHitMarkerImage;
	PerHitMarkerZoneOverrideImages = ZoneOverrideImages;
	AnyHitsMarkerImage = InArgs._AnyHitsMarkerImage;
	bColorAndOpacitySet = InArgs._ColorAndOpacity.IsSet();
	ColorAndOpacity = InArgs._ColorAndOpacity;

	MyContext = InContext;
}

// 绘制：按命中位置画标记。
int32 SHitMarkerConfirmationWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const bool bIsEnabled = ShouldBeEnabled(bParentEnabled);
	const ESlateDrawEffect DrawEffects = bIsEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
	const FVector2D LocalCenter = AllottedGeometry.GetLocalPositionAtCoordinates(FVector2D(0.5f, 0.5f));

	const bool bDrawMarkers = (HitNotifyOpacity > KINDA_SMALL_NUMBER);

	if (bDrawMarkers)
	{
		// 检查是否要用"屏幕空间命中位置"
		TArray<FTenkaichiScreenSpaceHitLocation> LastWeaponDamageScreenLocations;
		if (APlayerController* PC = MyContext.IsInitialized() ? MyContext.GetPlayerController() : nullptr)
		{
			if (UTenkaichiWeaponStateComponent* WeaponStateComponent = PC->FindComponentByClass<UTenkaichiWeaponStateComponent>())
			{
				WeaponStateComponent->GetLastWeaponDamageScreenLocations(/*out*/ LastWeaponDamageScreenLocations);
			}
		}

		if ((LastWeaponDamageScreenLocations.Num() > 0) && (PerHitMarkerImage != nullptr))
		{
			const FVector2D HalfAbsoluteSize = AllottedGeometry.GetAbsoluteSize() * 0.5f;

			for (const FTenkaichiScreenSpaceHitLocation& Hit : LastWeaponDamageScreenLocations)
			{
				// 按命中部位找覆盖图标（没有就用默认）
				const FSlateBrush* LocationMarkerImage = PerHitMarkerZoneOverrideImages.Find(Hit.HitZone);
				if (LocationMarkerImage == nullptr)
				{
					LocationMarkerImage = PerHitMarkerImage;
				}

				FLinearColor MarkerColor = bColorAndOpacitySet ?
					ColorAndOpacity.Get().GetColor(InWidgetStyle) :
					(InWidgetStyle.GetColorAndOpacityTint() * LocationMarkerImage->GetTint(InWidgetStyle));
				MarkerColor.A *= HitNotifyOpacity;

				const FVector2D WindowSSLocation = Hit.Location + MyCullingRect.GetTopLeft(); // 非全屏时补偿窗口边框
				const FSlateRenderTransform DrawPos(AllottedGeometry.AbsoluteToLocal(WindowSSLocation));

				const FPaintGeometry Geometry(AllottedGeometry.ToPaintGeometry(LocationMarkerImage->ImageSize, FSlateLayoutTransform(-(LocationMarkerImage->ImageSize * 0.5f)), DrawPos));
				FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Geometry, LocationMarkerImage, DrawEffects, MarkerColor);
			}
		}
		
		if (AnyHitsMarkerImage != nullptr)
		{
			FLinearColor MarkerColor = bColorAndOpacitySet ?
				ColorAndOpacity.Get().GetColor(InWidgetStyle) :
				(InWidgetStyle.GetColorAndOpacityTint() * AnyHitsMarkerImage->GetTint(InWidgetStyle));
			MarkerColor.A *= HitNotifyOpacity;

			// 否则在准星中心显示命中通知
			const FPaintGeometry Geometry(AllottedGeometry.ToPaintGeometry(AnyHitsMarkerImage->ImageSize, FSlateLayoutTransform(LocalCenter - (AnyHitsMarkerImage->ImageSize * 0.5f))));
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Geometry, AnyHitsMarkerImage, DrawEffects, MarkerColor);
		}
	}

	return LayerId;
}

// 期望尺寸。
FVector2D SHitMarkerConfirmationWidget::ComputeDesiredSize(float) const
{
	return FVector2D(100.0f, 100.0f);
}

// 每帧更新淡出透明度。
void SHitMarkerConfirmationWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	HitNotifyOpacity = 0.0f;
	if (APlayerController* PC = MyContext.IsInitialized() ? MyContext.GetPlayerController() : nullptr)
	{
		if (UTenkaichiWeaponStateComponent* DamageMarkerComponent = PC->FindComponentByClass<UTenkaichiWeaponStateComponent>())
		{
			const double TimeSinceLastHitNotification = DamageMarkerComponent->GetTimeSinceLastHitNotification();
			if (TimeSinceLastHitNotification < HitNotifyDuration)
			{
				// 线性淡出：1.0 → 0.0
				HitNotifyOpacity = FMath::Clamp(1.0f - (float)(TimeSinceLastHitNotification / HitNotifyDuration), 0.0f, 1.0f);
			}
		}
	}
}