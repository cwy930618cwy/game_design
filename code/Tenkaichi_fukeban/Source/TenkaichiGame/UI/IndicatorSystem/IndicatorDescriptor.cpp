// 版权声明（可保留或省略，不影响编译）
#include "IndicatorDescriptor.h"

#include "Engine/LocalPlayer.h"
#include "SceneView.h"
#include "UI/IndicatorSystem/TenkaichiIndicatorManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IndicatorDescriptor)

// 执行投影：把指示器的世界位置换算成屏幕位置（含深度）。
bool FIndicatorProjection::Project(const UIndicatorDescriptor& IndicatorDescriptor, const FSceneViewProjectionData& InProjectionData, const FVector2f& ScreenSize, FVector& OutScreenPositionWithDepth)
{
	if (USceneComponent* Component = IndicatorDescriptor.GetSceneComponent())
	{
		// ① 拿世界位置（有 socket 名就用 socket 变换，否则用组件位置）
		TOptional<FVector> WorldLocation;
		if (IndicatorDescriptor.GetComponentSocketName() != NAME_None)
		{
			WorldLocation = Component->GetSocketTransform(IndicatorDescriptor.GetComponentSocketName()).GetLocation();
		}
		else
		{
			WorldLocation = Component->GetComponentLocation();
		}

		// ② 加上世界偏移
		const FVector ProjectWorldLocation = WorldLocation.GetValue() + IndicatorDescriptor.GetWorldPositionOffset();
		const EActorCanvasProjectionMode ProjectionMode = IndicatorDescriptor.GetProjectionMode();
		
		switch (ProjectionMode)
		{
			case EActorCanvasProjectionMode::ComponentPoint:
			{
				if (WorldLocation.IsSet())
				{
					FVector2D OutScreenSpacePosition;
					// ③ 世界点 → 屏幕点
					const bool bInFrontOfCamera = ULocalPlayer::GetPixelPoint(InProjectionData, ProjectWorldLocation, OutScreenSpacePosition, &ScreenSize);

					// ④ 加屏幕偏移（在相机背后时 X 取反，让标记往另一侧偏）
					OutScreenSpacePosition.X += IndicatorDescriptor.GetScreenSpaceOffset().X * (bInFrontOfCamera ? 1 : -1);
					OutScreenSpacePosition.Y += IndicatorDescriptor.GetScreenSpaceOffset().Y;

					// ⑤ 相机背后的点会被投影到屏幕内，需要推到屏幕边缘
					if (!bInFrontOfCamera && FBox2f(FVector2f::Zero(), ScreenSize).IsInside((FVector2f)OutScreenSpacePosition))
					{
						const FVector2f CenterToPosition = (FVector2f(OutScreenSpacePosition) - (ScreenSize / 2)).GetSafeNormal();
						OutScreenSpacePosition = FVector2D((ScreenSize / 2) + CenterToPosition * ScreenSize);
					}

					// ⑥ 输出（Z = 到相机距离，用于排序）
					OutScreenPositionWithDepth = FVector(OutScreenSpacePosition.X, OutScreenSpacePosition.Y, FVector::Dist(InProjectionData.ViewOrigin, ProjectWorldLocation));

					return true;
				}

				return false;
			}
			case EActorCanvasProjectionMode::ComponentScreenBoundingBox:
			case EActorCanvasProjectionMode::ActorScreenBoundingBox:
			{
				// ⑦ 先拿包围盒（组件包围盒 or 整个 Actor 包围盒）
				FBox IndicatorBox;
				if (ProjectionMode == EActorCanvasProjectionMode::ActorScreenBoundingBox)
				{
					IndicatorBox = Component->GetOwner()->GetComponentsBoundingBox();
				}
				else
				{
					IndicatorBox = Component->Bounds.GetBox();
				}

				// ⑧ 包围盒 → 屏幕矩形（左下 LL / 右上 UR）
				FVector2D LL, UR;
				const bool bInFrontOfCamera = ULocalPlayer::GetPixelBoundingBox(InProjectionData, IndicatorBox, LL, UR, &ScreenSize);
			
				const FVector& BoundingBoxAnchor = IndicatorDescriptor.GetBoundingBoxAnchor();
				const FVector2D& ScreenSpaceOffset = IndicatorDescriptor.GetScreenSpaceOffset();

				// ⑨ 按锚点在这两个角之间插值
				FVector ScreenPositionWithDepth;
				ScreenPositionWithDepth.X = FMath::Lerp(LL.X, UR.X, BoundingBoxAnchor.X) + ScreenSpaceOffset.X * (bInFrontOfCamera ? 1 : -1);
				ScreenPositionWithDepth.Y = FMath::Lerp(LL.Y, UR.Y, BoundingBoxAnchor.Y) + ScreenSpaceOffset.Y;
				ScreenPositionWithDepth.Z = FVector::Dist(InProjectionData.ViewOrigin, ProjectWorldLocation);

				// ⑩ 相机背后处理（同 ⑤）
				const FVector2f ScreenSpacePosition = FVector2f(FVector2D(ScreenPositionWithDepth));
				if (!bInFrontOfCamera && FBox2f(FVector2f::Zero(), ScreenSize).IsInside(ScreenSpacePosition))
				{
					const FVector2f CenterToPosition = (ScreenSpacePosition - (ScreenSize / 2)).GetSafeNormal();
					const FVector2f ScreenPositionFromBehind = (ScreenSize / 2) + CenterToPosition * ScreenSize;
					ScreenPositionWithDepth.X = ScreenPositionFromBehind.X;
					ScreenPositionWithDepth.Y = ScreenPositionFromBehind.Y;
				}
				
				OutScreenPositionWithDepth = ScreenPositionWithDepth;
				return true;
			}
			case EActorCanvasProjectionMode::ActorBoundingBox:
			case EActorCanvasProjectionMode::ComponentBoundingBox:
			{
				// ⑪ 先拿包围盒（同上）
				FBox IndicatorBox;
				if (ProjectionMode == EActorCanvasProjectionMode::ActorBoundingBox)
				{
					IndicatorBox = Component->GetOwner()->GetComponentsBoundingBox();
				}
				else
				{
					IndicatorBox = Component->Bounds.GetBox();
				}

				// ⑫ 包围盒里按锚点算一个点（锚点 - 0.5 后乘尺寸）
				const FVector ProjectBoxPoint = IndicatorBox.GetCenter() + (IndicatorBox.GetSize() * (IndicatorDescriptor.GetBoundingBoxAnchor() - FVector(0.5)));

				FVector2D OutScreenSpacePosition;
				const bool bInFrontOfCamera = ULocalPlayer::GetPixelPoint(InProjectionData, ProjectBoxPoint, OutScreenSpacePosition, &ScreenSize);
				OutScreenSpacePosition.X += IndicatorDescriptor.GetScreenSpaceOffset().X * (bInFrontOfCamera ? 1 : -1);
				OutScreenSpacePosition.Y += IndicatorDescriptor.GetScreenSpaceOffset().Y;

				// ⑬ 相机背后处理（同 ⑤）
				if (!bInFrontOfCamera && FBox2f(FVector2f::Zero(), ScreenSize).IsInside((FVector2f)OutScreenSpacePosition))
				{
					const FVector2f CenterToPosition = (FVector2f(OutScreenSpacePosition) - (ScreenSize / 2)).GetSafeNormal();
					OutScreenSpacePosition = FVector2D((ScreenSize / 2) + CenterToPosition * ScreenSize);
				}

				OutScreenPositionWithDepth = FVector(OutScreenSpacePosition.X, OutScreenSpacePosition.Y, FVector::Dist(InProjectionData.ViewOrigin, ProjectBoxPoint));
					
				return true;
			}
		}
	}

	return false;
}

// 设置所属的指示器管理器（只能设一次）。
void UIndicatorDescriptor::SetIndicatorManagerComponent(UTenkaichiIndicatorManagerComponent* InManager)
{
	// 确保没人设过这个
	if (ensure(ManagerPtr.IsExplicitlyNull()))
	{
		ManagerPtr = InManager;
	}
}

// 从管理器注销自己（移除指示器）。
void UIndicatorDescriptor::UnregisterIndicator()
{
	if (UTenkaichiIndicatorManagerComponent* Manager = ManagerPtr.Get())
	{
		Manager->RemoveIndicator(this);
	}
}