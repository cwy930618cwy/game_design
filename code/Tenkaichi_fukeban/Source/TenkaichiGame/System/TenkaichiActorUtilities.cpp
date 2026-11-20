// Copyright Epic Games, Inc. All Rights Reserved.

// 先包含自己的 .h（里面声明了 SwitchOnNetMode）
#include "TenkaichiActorUtilities.h"

// 用到 AActor / UActorComponent 的 GetNetMode()，这是它们定义的地方
#include "GameFramework/Actor.h"

// UHT 生成的 .cpp 实现（对应 TenkaichiActorUtilities.generated.h）
#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiActorUtilities)

// 根据传入的"世界上下文对象"判断当前是哪种网络模式，并返回蓝图友好的枚举
EBlueprintExposedNetMode UTenkaichiActorUtilities::SwitchOnNetMode(const UObject* WorldContextObject)
{
	// 先默认单机
	ENetMode NetMode = NM_Standalone;

	// 从传入对象一路向上找 Outer（外层宿主），直到找到能给出网络模式的 Actor 或 Component
	for (const UObject* TestObject = WorldContextObject; TestObject != nullptr; TestObject = TestObject->GetOuter())
	{
		// 如果是 Component：直接拿它的网络模式
		if (const UActorComponent* Component = Cast<const UActorComponent>(WorldContextObject))
		{
			NetMode = Component->GetNetMode();
			break;
		}
		// 否则如果是 Actor：拿 Actor 的网络模式
		else if (const AActor* Actor = Cast<const AActor>(WorldContextObject))
		{
			NetMode = Actor->GetNetMode();
			break;
		}
	}

	// 把引擎的 ENetMode 翻译成蓝图友好的 EBlueprintExposedNetMode
	switch (NetMode)
	{
	case NM_Client:
		return EBlueprintExposedNetMode::Client;
	case NM_Standalone:
		return EBlueprintExposedNetMode::Standalone;
	case NM_DedicatedServer:
		return EBlueprintExposedNetMode::DedicatedServer;
	case NM_ListenServer:
		return EBlueprintExposedNetMode::ListenServer;
	default:
		ensure(false);  // 理论到不了这，到了就断言提醒
		return EBlueprintExposedNetMode::Standalone;
	}
}