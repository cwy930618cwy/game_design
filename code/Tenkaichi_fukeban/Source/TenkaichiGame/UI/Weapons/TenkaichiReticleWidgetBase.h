// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonUserWidget.h"

#include "TenkaichiReticleWidgetBase.generated.h"

class UTenkaichiInventoryItemInstance;
class UTenkaichiWeaponInstance;
class UObject;
struct FFrame;

// 准星基类：从武器实例算出散布角度 / 屏幕散布半径 / 精准首枪。
UCLASS(Abstract)
class UTenkaichiReticleWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 构造函数：空实现。
	UTenkaichiReticleWidgetBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 武器初始化完成的蓝图事件（子类实现）。
	UFUNCTION(BlueprintImplementableEvent)
	void OnWeaponInitialized();

	// 用指定武器初始化准星。
	UFUNCTION(BlueprintCallable)
	void InitializeFromWeapon(UTenkaichiWeaponInstance* InWeapon);

	/** 返回当前武器的直径散布角度（度） */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float ComputeSpreadAngle() const;

	/** 返回当前武器的最大散布半径（屏幕空间像素） */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float ComputeMaxScreenspaceSpreadRadius() const;

	/**
	 * 如果当前武器处于"第一次射击精准"状态就返回 true
	 * （武器允许它且处于最小散布）
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool HasFirstShotAccuracy() const;

protected:
	// 当前武器实例。
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTenkaichiWeaponInstance> WeaponInstance;

	// 当前武器的库存物品实例（武器的 Owner）。
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTenkaichiInventoryItemInstance> InventoryInstance;
};