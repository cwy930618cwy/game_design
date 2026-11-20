// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "TenkaichiTeamStatics.generated.h"

class UTenkaichiTeamDisplayAsset;
class UObject;
class UTexture;
struct FFrame;

/**
 * 给队伍相关 Actor（如 Pawn 或 PlayerState）提供队伍信息的便捷访问（蓝图工具库）
 */
UCLASS()
class UTenkaichiTeamStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// 返回这个对象属于哪个队伍，若不属于任何队伍则返回 INDEX_NONE
	UFUNCTION(BlueprintCallable, Category=Teams, meta=(Keywords="GetTeamFromObject", DefaultToSelf="Agent", AdvancedDisplay="bLogIfNotSet"))
	static void FindTeamFromObject(const UObject* Agent, bool& bIsPartOfTeam, int32& TeamId, UTenkaichiTeamDisplayAsset*& DisplayAsset, bool bLogIfNotSet = false);

	// 返回指定队伍 ID 的外观资产
	UFUNCTION(BlueprintCallable, Category=Teams, meta=(WorldContext="WorldContextObject"))
	static UTenkaichiTeamDisplayAsset* GetTeamDisplayAsset(const UObject* WorldContextObject, int32 TeamId);

	// 从外观资产取浮点参数，取不到则返回默认值
	UFUNCTION(BlueprintCallable, Category = Teams)
	static float GetTeamScalarWithFallback(UTenkaichiTeamDisplayAsset* DisplayAsset, FName ParameterName, float DefaultValue);

	// 从外观资产取颜色参数，取不到则返回默认值
	UFUNCTION(BlueprintCallable, Category = Teams)
	static FLinearColor GetTeamColorWithFallback(UTenkaichiTeamDisplayAsset* DisplayAsset, FName ParameterName, FLinearColor DefaultValue);

	// 从外观资产取贴图参数，取不到则返回默认值
	UFUNCTION(BlueprintCallable, Category = Teams)
	static UTexture* GetTeamTextureWithFallback(UTenkaichiTeamDisplayAsset* DisplayAsset, FName ParameterName, UTexture* DefaultValue);
};