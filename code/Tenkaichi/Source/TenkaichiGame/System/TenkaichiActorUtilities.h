// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 蓝图函数库基类：把静态函数暴露给蓝图
#include "Kismet/BlueprintFunctionLibrary.h"

// 生成头文件（UHT 用，放最后）
#include "TenkaichiActorUtilities.generated.h"

class UObject;
struct FFrame;

// 蓝图友好的网络模式枚举（对应引擎 ENetMode，但值是连续的、可被蓝图 Switch）
UENUM()
enum class EBlueprintExposedNetMode : uint8
{
	/** 单机：无网络，一个或多个本地玩家。因为具备全部服务器功能，所以也看作服务器。 */
	Standalone,

	/** 纯服务器（dedicated server）：没有本地玩家的服务器。 */
	DedicatedServer,

	/** 监听服务器（listen server）：自身带一个本地玩家、同时允许其他网络玩家加入的服务器。 */
	ListenServer,

	/**
	 * 网络客户端：连接远端服务器的客户端。
	 * 注意：所有小于 NM_Client 的模式都算某种服务器，所以"NetMode < NM_Client"恒为服务器。
	 */
	Client
};


UCLASS()
class UTenkaichiActorUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 获取某个 Actor 或 Component 当前的网络模式（纯服务器/客户端/单机等）。
	 */
	UFUNCTION(BlueprintCallable, Category="Tenkaichi", meta=(WorldContext="WorldContextObject", ExpandEnumAsExecs=ReturnValue))
	static EBlueprintExposedNetMode SwitchOnNetMode(const UObject* WorldContextObject);
};