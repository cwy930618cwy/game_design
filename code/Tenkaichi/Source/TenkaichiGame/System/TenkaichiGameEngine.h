// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 引擎类头文件：提供 UGameEngine 基类
#include "Engine/GameEngine.h"

// 生成头文件（UHT 用，放最后）
#include "TenkaichiGameEngine.generated.h"

class IEngineLoop;
class UObject;


UCLASS()
class UTenkaichiGameEngine : public UGameEngine
{
	GENERATED_BODY()

public:

	// 构造函数（带默认参数，可传入 FObjectInitializer）
	UTenkaichiGameEngine(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	// 重写引擎初始化钩子：引擎启动时被调用，可在这里插自定义逻辑
	virtual void Init(IEngineLoop* InEngineLoop) override;
};