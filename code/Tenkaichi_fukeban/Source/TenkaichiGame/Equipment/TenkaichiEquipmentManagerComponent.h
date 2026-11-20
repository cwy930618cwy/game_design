// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/TenkaichiAbilitySet.h"
#include "Components/PawnComponent.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "TenkaichiEquipmentManagerComponent.generated.h"

#define UE_API TENKAICHIGAME_API

class UActorComponent;
class UTenkaichiAbilitySystemComponent;
class UTenkaichiEquipmentDefinition;
class UTenkaichiEquipmentInstance;
class UTenkaichiEquipmentManagerComponent;
class UObject;
struct FFrame;
struct FTenkaichiEquipmentList;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;

/** 一条"已穿装备"的记录 */
USTRUCT(BlueprintType)
struct FTenkaichiAppliedEquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FTenkaichiAppliedEquipmentEntry()
	{}

	// 调试用：打印这一格的描述文本
	FString GetDebugString() const;

private:
	// 友元：列表和管理器才能直接访问内部字段
	friend FTenkaichiEquipmentList;
	friend UTenkaichiEquipmentManagerComponent;

	// 穿的哪件装备（定义）
	UPROPERTY()
	TSubclassOf<UTenkaichiEquipmentDefinition> EquipmentDefinition;

	// 装备实例
	UPROPERTY()
	TObjectPtr<UTenkaichiEquipmentInstance> Instance = nullptr;

	// 授予技能的句柄（只服务器记录，用于回收），不参与网络同步
	UPROPERTY(NotReplicated)
	FTenkaichiAbilitySet_GrantedHandles GrantedHandles;
};

/** 已穿装备列表 */
USTRUCT(BlueprintType)
struct FTenkaichiEquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

	// 默认构造：列表还没挂到任何组件上
	FTenkaichiEquipmentList()
		: OwnerComponent(nullptr)
	{
	}

	// 构造：挂到持有它的组件
	FTenkaichiEquipmentList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
	{
	}

public:
	//~FFastArraySerializer 接口：网络同步时自动回调
	void PreReplicateRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicateAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicateChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~FFastArraySerializer 接口结束

	// 增量序列化：只同步变化的格子
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FTenkaichiAppliedEquipmentEntry, FTenkaichiEquipmentList>(Entries, DeltaParms, *this);
	}

	// 按装备定义穿一件（建实例 + 授予技能 + 生成 Actor），返回新实例
	UTenkaichiEquipmentInstance* AddEntry(TSubclassOf<UTenkaichiEquipmentDefinition> EquipmentDefinition);
	// 脱一件装备
	void RemoveEntry(UTenkaichiEquipmentInstance* Instance);

private:
	// 取角色的能力系统组件（ASC）
	UTenkaichiAbilitySystemComponent* GetAbilitySystemComponent() const;

	// 友元：管理器才能直接访问内部列表
	friend UTenkaichiEquipmentManagerComponent;

private:
	// 已穿装备条目数组（真正存储的地方）
	UPROPERTY()
	TArray<FTenkaichiAppliedEquipmentEntry> Entries;

	// 拥有列表的组件，不参与网络同步
	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

// 模板特化：告诉引擎这个结构体用自定义增量序列化（必须显式声明）
template<>
struct TStructOpsTypeTraits<FTenkaichiEquipmentList> : public TStructOpsTypeTraitsBase2<FTenkaichiEquipmentList>
{
	enum { WithNetDeltaSerializer = true };
};

/**
 * 管理穿到角色身上的装备
 */
UCLASS(MinimalAPI, BlueprintType, Const)
class UTenkaichiEquipmentManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	// 构造函数
	UE_API UTenkaichiEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 穿一件装备（授予技能 + 生成 Actor），返回装备实例
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	UE_API UTenkaichiEquipmentInstance* EquipItem(TSubclassOf<UTenkaichiEquipmentDefinition> EquipmentDefinition);

	// 脱一件装备（回收技能 + 销毁 Actor）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	UE_API void UnequipItem(UTenkaichiEquipmentInstance* ItemInstance);

	//~UObject 接口
	// 复制子对象：把每个装备实例作为子对象复制给客户端
	UE_API virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	//~UObject 接口结束

	//~UActorComponent 接口
	// 初始化组件
	UE_API virtual void InitializeComponent() override;
	// 反初始化组件（脱下所有装备）
	UE_API virtual void UninitializeComponent() override;
	// 准备复制（注册所有已有装备实例）
	UE_API virtual void ReadyForReplication() override;
	//~UActorComponent 接口结束

	// 找第一件某类型的装备（没有返回空）
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UE_API UTenkaichiEquipmentInstance* GetFirstInstanceOfType(TSubclassOf<UTenkaichiEquipmentInstance> InstanceType);

	// 找所有某类型的装备（没有返回空数组）
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UE_API TArray<UTenkaichiEquipmentInstance*> GetEquipmentInstancesOfType(TSubclassOf<UTenkaichiEquipmentInstance> InstanceType) const;

	// 模板版本：按类型参数找第一件
	template <typename T>
	T* GetFirstInstanceOfType()
	{
		return (T*)GetFirstInstanceOfType(T::StaticClass());
	}

private:
	// 整个已穿装备列表（网络同步字段）
	UPROPERTY(Replicated)
	FTenkaichiEquipmentList EquipmentList;
};

#undef UE_API