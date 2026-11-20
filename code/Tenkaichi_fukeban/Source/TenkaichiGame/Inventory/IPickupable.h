// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "UObject/Interface.h"

#include "UObject/ObjectPtr.h"
#include "IPickupable.generated.h"

// 前置声明：脚本接口模板（UE 接口的智能指针容器）
template <typename InterfaceType> class TScriptInterface;

class AActor;
class UTenkaichiInventoryItemDefinition;
class UTenkaichiInventoryItemInstance;
class UTenkaichiInventoryManagerComponent;
class UObject;
struct FFrame;

// 掉落模板：捡到"一堆同种物品"（一个定义 + 数量）
USTRUCT(BlueprintType)
struct FPickupTemplate
{
	GENERATED_BODY()

public:
	// 数量（默认 1 个）
	UPROPERTY(EditAnywhere)
	int32 StackCount = 1;

	// 物品定义（这是什么物品）
	UPROPERTY(EditAnywhere)
	TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef;
};

// 掉落实例：捡到"一个现成的、已存在的物品实例"
USTRUCT(BlueprintType)
struct FPickupInstance
{
	GENERATED_BODY()

public:
	// 现成的物品实例
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UTenkaichiInventoryItemInstance> Item = nullptr;
};

// 掉落包：捡一次能拿到的一整包东西（现成实例列表 + 模板列表）
USTRUCT(BlueprintType)
struct FInventoryPickup
{
	GENERATED_BODY()

public:
	// 现成实例列表
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FPickupInstance> Instances;

	// 模板列表（按定义 + 数量批量生成）
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FPickupTemplate> Templates;
};

// 接口外壳：反射系统登记"这是一个接口"（不能直接在蓝图中实现）
UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UPickupable : public UInterface
{
	GENERATED_BODY()
};

// 拾取接口（C++ 接口体）：实现它的 Actor 声明"我能被捡"
class IPickupable
{
	GENERATED_BODY()

public:
	// 纯虚函数：返回这个掉落物的"掉落包"（子类必须实现）
	UFUNCTION(BlueprintCallable)
	virtual FInventoryPickup GetPickupInventory() const = 0;
};

// 拾取工具类：静态函数库，负责"找能捡的" + "把捡到的塞进背包"
UCLASS()
class UPickupableStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 构造函数
	UPickupableStatics();

public:
	// 从 Actor 上找出第一个"能捡的"接口（先看 Actor 自己，再看它的组件）
	UFUNCTION(BlueprintPure)
	static TScriptInterface<IPickupable> GetFirstPickupableFromActor(AActor* Actor);

	// 把掉落包里的东西塞进背包管理器（服务器权限）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (WorldContext = "Ability"))
	static void AddPickupToInventory(UTenkaichiInventoryManagerComponent* InventoryComponent, TScriptInterface<IPickupable> Pickup);
};