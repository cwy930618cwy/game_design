// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ControllerComponent.h"
#include "Inventory/TenkaichiInventoryItemInstance.h"

#include "TenkaichiQuickBarComponent.generated.h"

class AActor;
class UTenkaichiEquipmentInstance;
class UTenkaichiEquipmentManagerComponent;
class UObject;
struct FFrame;

// 快捷栏组件：挂在 Controller 上，管理装备槽位
UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class UTenkaichiQuickBarComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	// 构造函数
	UTenkaichiQuickBarComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 向前循环切换激活槽
	UFUNCTION(BlueprintCallable, Category="Tenkaichi")
	void CycleActiveSlotForward();

	// 向后循环切换激活槽
	UFUNCTION(BlueprintCallable, Category="Tenkaichi")
	void CycleActiveSlotBackward();

	// 设置激活槽下标（服务器 RPC，只在服务器执行）
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Tenkaichi")
	void SetActiveSlotIndex(int32 NewIndex);

	// 取所有槽位
	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	TArray<UTenkaichiInventoryItemInstance*> GetSlots() const
	{
		return Slots;
	}

	// 取当前激活槽下标
	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

	// 取当前激活槽的物品
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	UTenkaichiInventoryItemInstance* GetActiveSlotItem() const;

	// 取下一个空槽下标（没有返回 INDEX_NONE）
	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	int32 GetNextFreeItemSlot() const;

	// 往某槽放物品（服务器权限）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddItemToSlot(int32 SlotIndex, UTenkaichiInventoryItemInstance* Item);

	// 从某槽取物品（服务器权限）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	UTenkaichiInventoryItemInstance* RemoveItemFromSlot(int32 SlotIndex);

	// 组件开始运行时调用
	virtual void BeginPlay() override;

private:
	// 脱下当前槽的装备
	void UnequipItemInSlot();
	// 穿上当前槽的装备
	void EquipItemInSlot();

	// 找角色的装备管理器
	UTenkaichiEquipmentManagerComponent* FindEquipmentManager() const;

protected:
	// 槽位数量（默认 3）
	UPROPERTY()
	int32 NumSlots = 3;

	// 槽位变化复制回调
	UFUNCTION()
	void OnRep_Slots();

	// 激活槽变化复制回调
	UFUNCTION()
	void OnRep_ActiveSlotIndex();

private:
	// 槽位数组（复制时触发 OnRep_Slots）
	UPROPERTY(ReplicatedUsing=OnRep_Slots)
	TArray<TObjectPtr<UTenkaichiInventoryItemInstance>> Slots;

	// 当前激活槽下标（-1 = 无，复制时触发 OnRep_ActiveSlotIndex）
	UPROPERTY(ReplicatedUsing=OnRep_ActiveSlotIndex)
	int32 ActiveSlotIndex = -1;

	// 当前穿上的装备（不复制，本地记录）
	UPROPERTY()
	TObjectPtr<UTenkaichiEquipmentInstance> EquippedItem;
};


// 槽位变化消息：槽位数组变了时广播
USTRUCT(BlueprintType)
struct FTenkaichiQuickBarSlotsChangedMessage
{
	GENERATED_BODY()

	// 谁的快捷栏
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	TObjectPtr<AActor> Owner = nullptr;

	// 最新槽位数组
	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TArray<TObjectPtr<UTenkaichiInventoryItemInstance>> Slots;
};


// 激活槽变化消息：激活槽下标变了时广播
USTRUCT(BlueprintType)
struct FTenkaichiQuickBarActiveIndexChangedMessage
{
	GENERATED_BODY()

	// 谁的快捷栏
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	TObjectPtr<AActor> Owner = nullptr;

	// 最新激活槽下标
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 ActiveIndex = 0;
};