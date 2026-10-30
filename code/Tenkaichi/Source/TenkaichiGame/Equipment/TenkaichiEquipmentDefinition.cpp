// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiEquipmentDefinition.h"
#include "TenkaichiEquipmentInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiEquipmentDefinition)

// 构造函数：把默认实例类设为 UTenkaichiEquipmentInstance
UTenkaichiEquipmentDefinition::UTenkaichiEquipmentDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstanceType = UTenkaichiEquipmentInstance::StaticClass();
}