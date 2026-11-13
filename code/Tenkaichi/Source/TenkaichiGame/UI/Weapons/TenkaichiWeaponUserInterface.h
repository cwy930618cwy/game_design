// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonUserWidget.h"

#include "TenkaichiWeaponUserInterface.generated.h"

class UTenkaichiWeaponInstance;
class UObject;
struct FGeometry;

// 武器 UI 基类：每帧检测当前武器，切换时通知蓝图。
UCLASS()
class UTenkaichiWeaponUserInterface : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 构造函数：空实现。
	UTenkaichiWeaponUserInterface(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 控件构造完成。
	virtual void NativeConstruct() override;
	// 控件销毁。
	virtual void NativeDestruct() override;
	// 每帧 Tick（检测武器变化）。
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 武器变化时的蓝图事件（子类实现）。
	UFUNCTION(BlueprintImplementableEvent)
	void OnWeaponChanged(UTenkaichiWeaponInstance* OldWeapon, UTenkaichiWeaponInstance* NewWeapon);

private:
	// 按新武器重建控件（基类留空，子类可覆写）。
	void RebuildWidgetFromWeapon();

private:
	// 当前武器实例。
	UPROPERTY(Transient)
	TObjectPtr<UTenkaichiWeaponInstance> CurrentInstance;
};