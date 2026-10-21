// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 蓝图函数库基类：把静态函数暴露给蓝图
#include "Kismet/BlueprintFunctionLibrary.h"

// 软对象指针（TSoftObjectPtr）类型定义，用于"引用但不加载"资产
#include "UObject/SoftObjectPtr.h"

// 生成头文件（UHT 用，放最后）
#include "TenkaichiSystemStatics.generated.h"

// 模板类 TSubclassOf 的前置声明（类引用类型，这里只声明不展开）
template <typename T> class TSubclassOf;

class AActor;
class UActorComponent;
class UObject;
struct FFrame;

// 蓝图函数库：集中放一批系统级工具函数
UCLASS()
class UTenkaichiSystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** 返回与主资产 ID 关联的软对象引用，即使资产未加载也能用（用 DeterminesOutputType 让蓝图推导输出类型） */
	UFUNCTION(BlueprintPure, Category = "AssetManager", meta=(DeterminesOutputType=ExpectedAssetType))
	static TSoftObjectPtr<UObject> GetTypedSoftObjectReferenceFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId, TSubclassOf<UObject> ExpectedAssetType);

	UFUNCTION(BlueprintCallable)
	static FPrimaryAssetId GetPrimaryAssetIdFromUserFacingExperienceName(const FString& AdvertisedExperienceID);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Tenkaichi", meta = (WorldContext = "WorldContextObject"))
	static void PlayNextGame(const UObject* WorldContextObject);

	// 把 TargetActor 上所有 Mesh 组件的所有 section 的标量参数 ParameterName 设为 ParameterValue
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta=(DefaultToSelf="TargetActor"))
	static void SetScalarParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const float ParameterValue, bool bIncludeChildActors = true);

	// 把 TargetActor 上所有 Mesh 组件的所有 section 的向量参数 ParameterName 设为 ParameterValue
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta=(DefaultToSelf="TargetActor"))
	static void SetVectorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FVector ParameterValue, bool bIncludeChildActors = true);

	// 把 TargetActor 上所有 Mesh 组件的所有 section 的颜色参数 ParameterName 设为 ParameterValue
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta=(DefaultToSelf="TargetActor"))
	static void SetColorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FLinearColor ParameterValue, bool bIncludeChildActors = true);

	// 获取继承自给定类的所有组件
	UFUNCTION(BlueprintCallable, Category = "Actor", meta=(DefaultToSelf="TargetActor", ComponentClass="/Script/Engine.ActorComponent", DeterminesOutputType="ComponentClass"))
	static TArray<UActorComponent*> FindComponentsByClass(AActor* TargetActor, TSubclassOf<UActorComponent> ComponentClass, bool bIncludeChildActors = true);
};