// Copyright Epic Games, Inc. All Rights Reserved.

// 先包含自己的 .h
#include "TenkaichiGameEngine.h"

// UHT 生成的实现（对应 TenkaichiGameEngine.generated.h）
#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameEngine)

class IEngineLoop;


// 构造函数：调用父类初始化
UTenkaichiGameEngine::UTenkaichiGameEngine(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 引擎初始化钩子：调用父类的 Init，暂时不额外加逻辑（留空壳，为将来插自定义启动代码）
void UTenkaichiGameEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);
}