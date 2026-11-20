// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "IPickupable.h"

#include "GameFramework/Actor.h"
#include "TenkaichiInventoryManagerComponent.h"
#include "UObject/ScriptInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IPickupable)

class UActorComponent;

// 构造函数：调用父类（蓝图函数库）的默认构造
UPickupableStatics::UPickupableStatics()
	: Super(FObjectInitializer::Get())
{
}

// 从 Actor 上找出第一个"能捡的"接口（先看 Actor 自己，再看它的组件）
TScriptInterface<IPickupable> UPickupableStatics::GetFirstPickupableFromActor(AActor* Actor)
{
	// 先看 Actor 自己是不是就能捡（直接把它转成接口试试）
	TScriptInterface<IPickupable> PickupableActor(Actor);
	if (PickupableActor)
	{
		return PickupableActor;
	}

	// 如果 Actor 自己不能捡，它可能有某个组件实现了拾取接口
	TArray<UActorComponent*> PickupableComponents = Actor ? Actor->GetComponentsByInterface(UPickupable::StaticClass()) : TArray<UActorComponent*>();
	if (PickupableComponents.Num() > 0)
	{
		// 取第一个能捡的组件（需要更精细的区分逻辑时，留到别处解决）
		return TScriptInterface<IPickupable>(PickupableComponents[0]);
	}

	// 都没有 → 返回空接口
	return TScriptInterface<IPickupable>();
}

// 把掉落包里的东西塞进背包管理器（服务器权限）
void UPickupableStatics::AddPickupToInventory(UTenkaichiInventoryManagerComponent* InventoryComponent, TScriptInterface<IPickupable> Pickup)
{
	if (InventoryComponent && Pickup)
	{
		// 拿到掉落包
		const FInventoryPickup& PickupInventory = Pickup->GetPickupInventory();

		// 第一类：模板（按定义 + 数量批量造）
		for (const FPickupTemplate& Template : PickupInventory.Templates)
		{
			InventoryComponent->AddItemDefinition(Template.ItemDef, Template.StackCount);
		}

		// 第二类：现成实例（直接塞）
		for (const FPickupInstance& Instance : PickupInventory.Instances)
		{
			InventoryComponent->AddItemInstance(Instance.Item);
		}
	}
}