// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/TenkaichiPawnData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiPawnData)

// 构造函数：把"Pawn 类 / 输入配置 / 默认相机模式"先置空，等数据资产在编辑器里填
UTenkaichiPawnData::UTenkaichiPawnData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PawnClass = nullptr;
	InputConfig = nullptr;
	DefaultCameraMode = nullptr;
}