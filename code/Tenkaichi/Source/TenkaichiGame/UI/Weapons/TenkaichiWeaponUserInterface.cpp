// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiWeaponUserInterface.h"

#include "Equipment/TenkaichiEquipmentManagerComponent.h"
#include "GameFramework/Pawn.h"
#include "Weapons/TenkaichiWeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiWeaponUserInterface)

struct FGeometry;

// 构造函数：空实现。
UTenkaichiWeaponUserInterface::UTenkaichiWeaponUserInterface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 控件构造完成。
void UTenkaichiWeaponUserInterface::NativeConstruct()
{
	Super::NativeConstruct();
}

// 控件销毁。
void UTenkaichiWeaponUserInterface::NativeDestruct()
{
	Super::NativeDestruct();
}

// 每帧 Tick（检测武器变化）。
void UTenkaichiWeaponUserInterface::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		if (UTenkaichiEquipmentManagerComponent* EquipmentManager = Pawn->FindComponentByClass<UTenkaichiEquipmentManagerComponent>())
		{
			if (UTenkaichiWeaponInstance* NewInstance = EquipmentManager->GetFirstInstanceOfType<UTenkaichiWeaponInstance>())
			{
				// 换武器了（且新武器有 owner/instigator，说明已完全初始化）
				if (NewInstance != CurrentInstance && NewInstance->GetInstigator() != nullptr)
				{
					UTenkaichiWeaponInstance* OldWeapon = CurrentInstance;
					CurrentInstance = NewInstance;
					RebuildWidgetFromWeapon();
					OnWeaponChanged(OldWeapon, CurrentInstance);
				}
			}
		}
	}
}

// 按新武器重建控件（基类留空，子类可覆写）。
void UTenkaichiWeaponUserInterface::RebuildWidgetFromWeapon()
{
	
}