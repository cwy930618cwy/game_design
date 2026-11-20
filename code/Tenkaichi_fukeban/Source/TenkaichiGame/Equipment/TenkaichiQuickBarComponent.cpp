// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiQuickBarComponent.h"

#include "Equipment/TenkaichiEquipmentDefinition.h"
#include "Equipment/TenkaichiEquipmentInstance.h"
#include "Equipment/TenkaichiEquipmentManagerComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Inventory/InventoryFragment_EquippableItem.h"
#include "NativeGameplayTags.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiQuickBarComponent)

class FLifetimeProperty;
class UTenkaichiEquipmentDefinition;

// 静态消息标签：槽位变化 / 激活槽变化时广播
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Tenkaichi_QuickBar_Message_SlotsChanged, "Tenkaichi.QuickBar.Message.SlotsChanged");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Tenkaichi_QuickBar_Message_ActiveIndexChanged, "Tenkaichi.QuickBar.Message.ActiveIndexChanged");

// 构造函数：标记默认复制
UTenkaichiQuickBarComponent::UTenkaichiQuickBarComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

// 网络复制注册：Slots + ActiveSlotIndex 两个字段要复制
void UTenkaichiQuickBarComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Slots);
	DOREPLIFETIME(ThisClass, ActiveSlotIndex);
}

// 组件开始运行：如果槽位不够，补满默认空槽
void UTenkaichiQuickBarComponent::BeginPlay()
{
	if (Slots.Num() < NumSlots)
	{
		Slots.AddDefaulted(NumSlots - Slots.Num());
	}

	Super::BeginPlay();
}

// 向前循环切换激活槽
void UTenkaichiQuickBarComponent::CycleActiveSlotForward()
{
	if (Slots.Num() < 2)
	{
		return;
	}

	const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num()-1 : ActiveSlotIndex);
	int32 NewIndex = ActiveSlotIndex;
	do
	{
		NewIndex = (NewIndex + 1) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlotIndex(NewIndex);
			return;
		}
	} while (NewIndex != OldIndex);
}

// 向后循环切换激活槽
void UTenkaichiQuickBarComponent::CycleActiveSlotBackward()
{
	if (Slots.Num() < 2)
	{
		return;
	}

	const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num()-1 : ActiveSlotIndex);
	int32 NewIndex = ActiveSlotIndex;
	do
	{
		NewIndex = (NewIndex - 1 + Slots.Num()) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlotIndex(NewIndex);
			return;
		}
	} while (NewIndex != OldIndex);
}

// 穿上当前槽的装备
void UTenkaichiQuickBarComponent::EquipItemInSlot()
{
	check(Slots.IsValidIndex(ActiveSlotIndex));
	check(EquippedItem == nullptr);

	if (UTenkaichiInventoryItemInstance* SlotItem = Slots[ActiveSlotIndex])
	{
		if (const UInventoryFragment_EquippableItem* EquipInfo = SlotItem->FindFragmentByClass<UInventoryFragment_EquippableItem>())
		{
			TSubclassOf<UTenkaichiEquipmentDefinition> EquipDef = EquipInfo->EquipmentDefinition;
			if (EquipDef != nullptr)
			{
				if (UTenkaichiEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
				{
					EquippedItem = EquipmentManager->EquipItem(EquipDef);
					if (EquippedItem != nullptr)
					{
						EquippedItem->SetInstigator(SlotItem);
					}
				}
			}
		}
	}
}

// 脱下当前槽的装备
void UTenkaichiQuickBarComponent::UnequipItemInSlot()
{
	if (UTenkaichiEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
	{
		if (EquippedItem != nullptr)
		{
			EquipmentManager->UnequipItem(EquippedItem);
			EquippedItem = nullptr;
		}
	}
}

// 找角色的装备管理器
UTenkaichiEquipmentManagerComponent* UTenkaichiQuickBarComponent::FindEquipmentManager() const
{
	if (AController* OwnerController = Cast<AController>(GetOwner()))
	{
		if (APawn* Pawn = OwnerController->GetPawn())
		{
			return Pawn->FindComponentByClass<UTenkaichiEquipmentManagerComponent>();
		}
	}
	return nullptr;
}

// 设置激活槽（服务器执行）：脱下旧装备 → 改下标 → 穿上新装备 → 广播
void UTenkaichiQuickBarComponent::SetActiveSlotIndex_Implementation(int32 NewIndex)
{
	if (Slots.IsValidIndex(NewIndex) && (ActiveSlotIndex != NewIndex))
	{
		UnequipItemInSlot();

		ActiveSlotIndex = NewIndex;

		EquipItemInSlot();

		OnRep_ActiveSlotIndex();
	}
}

// 取当前激活槽的物品
UTenkaichiInventoryItemInstance* UTenkaichiQuickBarComponent::GetActiveSlotItem() const
{
	return Slots.IsValidIndex(ActiveSlotIndex) ? Slots[ActiveSlotIndex] : nullptr;
}

// 取下一个空槽下标（没有返回 INDEX_NONE）
int32 UTenkaichiQuickBarComponent::GetNextFreeItemSlot() const
{
	int32 SlotIndex = 0;
	for (const TObjectPtr<UTenkaichiInventoryItemInstance>& ItemPtr : Slots)
	{
		if (ItemPtr == nullptr)
		{
			return SlotIndex;
		}
		++SlotIndex;
	}

	return INDEX_NONE;
}

// 往某槽放物品（只在空槽放）
void UTenkaichiQuickBarComponent::AddItemToSlot(int32 SlotIndex, UTenkaichiInventoryItemInstance* Item)
{
	if (Slots.IsValidIndex(SlotIndex) && (Item != nullptr))
	{
		if (Slots[SlotIndex] == nullptr)
		{
			Slots[SlotIndex] = Item;
			OnRep_Slots();
		}
	}
}

// 从某槽取物品（如果是激活槽，先脱装备）
UTenkaichiInventoryItemInstance* UTenkaichiQuickBarComponent::RemoveItemFromSlot(int32 SlotIndex)
{
	UTenkaichiInventoryItemInstance* Result = nullptr;

	if (ActiveSlotIndex == SlotIndex)
	{
		UnequipItemInSlot();
		ActiveSlotIndex = -1;
	}

	if (Slots.IsValidIndex(SlotIndex))
	{
		Result = Slots[SlotIndex];

		if (Result != nullptr)
		{
			Slots[SlotIndex] = nullptr;
			OnRep_Slots();
		}
	}

	return Result;
}

// 槽位变化复制回调：广播槽位变化消息
void UTenkaichiQuickBarComponent::OnRep_Slots()
{
	FTenkaichiQuickBarSlotsChangedMessage Message;
	Message.Owner = GetOwner();
	Message.Slots = Slots;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
	MessageSystem.BroadcastMessage(TAG_Tenkaichi_QuickBar_Message_SlotsChanged, Message);
}

// 激活槽变化复制回调：广播激活槽变化消息
void UTenkaichiQuickBarComponent::OnRep_ActiveSlotIndex()
{
	FTenkaichiQuickBarActiveIndexChangedMessage Message;
	Message.Owner = GetOwner();
	Message.ActiveIndex = ActiveSlotIndex;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
	MessageSystem.BroadcastMessage(TAG_Tenkaichi_QuickBar_Message_ActiveIndexChanged, Message);
}