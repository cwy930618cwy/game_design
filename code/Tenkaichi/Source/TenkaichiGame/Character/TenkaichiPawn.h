// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModularPawn.h"
#include "Teams/TenkaichiTeamAgentInterface.h"
#include "TenkaichiPawn.generated.h"

#define UE_API TENKAICHIGAME_API

class AController;
class UObject;
struct FFrame;

/**
 * ATenkaichiPawn
 */
UCLASS(MinimalAPI)
class ATenkaichiPawn : public AModularPawn, public ITenkaichiTeamAgentInterface
{
	GENERATED_BODY()

public:

	UE_API ATenkaichiPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor 接口开始
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~AActor 接口结束

	//~APawn 接口开始
	UE_API virtual void PossessedBy(AController* NewController) override;
	UE_API virtual void UnPossessed() override;
	//~APawn 接口结束

	//~ITenkaichiTeamAgentInterface 接口开始
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnTenkaichiTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~ITenkaichiTeamAgentInterface 接口结束

protected:
	// 决定接管结束后队伍 ID 的去向
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
	{
		// 这里可以改成：想释放后保留旧队就返回 OldTeamID，
		// 或者返回某个中立阵营的 ID 等等……
		return FGenericTeamId::NoTeam;
	}

private:
	UFUNCTION()
	UE_API void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

private:
	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	UPROPERTY()
	FOnTenkaichiTeamIndexChangedDelegate OnTeamChangedDelegate;

private:
	UFUNCTION()
	UE_API void OnRep_MyTeamID(FGenericTeamId OldTeamID);
};

#undef UE_API