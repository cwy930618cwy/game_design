// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/SoftObjectPtr.h"

#include "GameUIManagerSubsystem.generated.h"

#define UE_API COMMONGAME_API

class FSubsystemCollectionBase;
class UCommonLocalPlayer;
class UGameUIPolicy;
class UObject;

/**
 * 这个管理器预期会被你游戏实际需要创建的东西替换，所以这个类是抽象的，以防止它被直接创建。
 * 
 * 如果你只需要基础功能，可以开始在你的游戏里继承这个子系统。
 */
UCLASS(MinimalAPI, Abstract, config = Game)
class UGameUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UGameUIManagerSubsystem() { }
	
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	const UGameUIPolicy* GetCurrentUIPolicy() const { return CurrentPolicy; }
	UGameUIPolicy* GetCurrentUIPolicy() { return CurrentPolicy; }

	UE_API virtual void NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer);
	UE_API virtual void NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer);
	UE_API virtual void NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer);

protected:
	UE_API void SwitchToPolicy(UGameUIPolicy* InPolicy);

private:
	UPROPERTY(Transient)
	TObjectPtr<UGameUIPolicy> CurrentPolicy = nullptr;

	UPROPERTY(config, EditAnywhere)
	TSoftClassPtr<UGameUIPolicy> DefaultUIPolicyClass;
};

#undef UE_API
