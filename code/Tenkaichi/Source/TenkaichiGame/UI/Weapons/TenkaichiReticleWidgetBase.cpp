// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiReticleWidgetBase.h"

#include "Inventory/TenkaichiInventoryItemInstance.h"
#include "Weapons/TenkaichiRangedWeaponInstance.h"
#include "Weapons/TenkaichiWeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiReticleWidgetBase)

// 构造函数：空实现。
UTenkaichiReticleWidgetBase::UTenkaichiReticleWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 用指定武器初始化准星。
void UTenkaichiReticleWidgetBase::InitializeFromWeapon(UTenkaichiWeaponInstance* InWeapon)
{
	WeaponInstance = InWeapon;
	InventoryInstance = nullptr;
	if (WeaponInstance)
	{
		// 武器的 Instigator 实际是"库存物品实例"
		InventoryInstance = Cast<UTenkaichiInventoryItemInstance>(WeaponInstance->GetInstigator());
	}
	OnWeaponInitialized();
}


// 返回当前武器的直径散布角度（度）。
float UTenkaichiReticleWidgetBase::ComputeSpreadAngle() const
{
	if (const UTenkaichiRangedWeaponInstance* RangedWeapon = Cast<const UTenkaichiRangedWeaponInstance>(WeaponInstance))
	{
		const float BaseSpreadAngle = RangedWeapon->GetCalculatedSpreadAngle();
		const float SpreadAngleMultiplier = RangedWeapon->GetCalculatedSpreadAngleMultiplier();
		const float ActualSpreadAngle = BaseSpreadAngle * SpreadAngleMultiplier;

		return ActualSpreadAngle;
	}
	else
	{
		return 0.0f;
	}
}

// 是否处于"第一次射击精准"状态。
bool UTenkaichiReticleWidgetBase::HasFirstShotAccuracy() const
{
	if (const UTenkaichiRangedWeaponInstance* RangedWeapon = Cast<const UTenkaichiRangedWeaponInstance>(WeaponInstance))
	{
		return RangedWeapon->HasFirstShotAccuracy();
	}
	else
	{
		return false;
	}
}

// 返回当前武器的最大散布半径（屏幕空间像素）。
float UTenkaichiReticleWidgetBase::ComputeMaxScreenspaceSpreadRadius() const
{
	const float LongShotDistance = 10000.f;

	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->PlayerCameraManager)
	{
		// 武器的散布可以想象成一个圆锥。要找到准星可视化的屏幕空间散布，
		// 我们在很远距离处圆锥边缘上创建一个点。那个点落在圆锥截面的边缘上。
		// 然后把它投影回屏幕。它到屏幕中心的距离就是散布半径。

		// 这不完全精确，因为相机位置和枪口之间有一些距离。

		const float SpreadRadiusRads = FMath::DegreesToRadians(ComputeSpreadAngle() * 0.5f);
		const float SpreadRadiusAtDistance = FMath::Tan(SpreadRadiusRads) * LongShotDistance;

		FVector CamPos;
		FRotator CamOrient;
		PC->PlayerCameraManager->GetCameraViewPoint(CamPos, CamOrient);

		FVector CamForwDir = CamOrient.RotateVector(FVector::ForwardVector);
		FVector CamUpDir   = CamOrient.RotateVector(FVector::UpVector);

		FVector OffsetTargetAtDistance = CamPos + (CamForwDir * LongShotDistance) + (CamUpDir * SpreadRadiusAtDistance);

		FVector2D OffsetTargetInScreenspace;

		if (PC->ProjectWorldLocationToScreen(OffsetTargetAtDistance, OffsetTargetInScreenspace, true))
		{
			int32 ViewportSizeX(0), ViewportSizeY(0);
			PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

			const FVector2D ScreenSpaceCenter(FVector::FReal(ViewportSizeX) * 0.5f, FVector::FReal(ViewportSizeY) * 0.5f);

			return (OffsetTargetInScreenspace - ScreenSpaceCenter).Length();
		}
	}
	
	return 0.0f;
}