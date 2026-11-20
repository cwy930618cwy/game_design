// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiPawn.h"

#include "GameFramework/Controller.h"
#include "TenkaichiLogChannels.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ScriptInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiPawn)

class FLifetimeProperty;
class UObject;

ATenkaichiPawn::ATenkaichiPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ATenkaichiPawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MyTeamID);
}

void ATenkaichiPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ATenkaichiPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ATenkaichiPawn::PossessedBy(AController* NewController)
{
	const FGenericTeamId OldTeamID = MyTeamID;

	Super::PossessedBy(NewController);

	// 从控制器抄当前队伍，并监听它未来的变化
	if (ITenkaichiTeamAgentInterface* ControllerAsTeamProvider = Cast<ITenkaichiTeamAgentInterface>(NewController))
	{
		MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	}
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ATenkaichiPawn::UnPossessed()
{
	AController* const OldController = GetController();

	// 停止监听旧控制器的队伍变化
	const FGenericTeamId OldTeamID = MyTeamID;
	if (ITenkaichiTeamAgentInterface* ControllerAsTeamProvider = Cast<ITenkaichiTeamAgentInterface>(OldController))
	{
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	Super::UnPossessed();

	// 决定释放后新队伍是什么
	MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ATenkaichiPawn::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (GetController() == nullptr)
	{
		if (HasAuthority())
		{
			const FGenericTeamId OldTeamID = MyTeamID;
			MyTeamID = NewTeamID;
			ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
		}
		else
		{
			UE_LOG(LogTenkaichiTeams, Error, TEXT("You can't set the team ID on a pawn (%s) except on the authority"), *GetPathNameSafe(this));
		}
	}
	else
	{
		UE_LOG(LogTenkaichiTeams, Error, TEXT("You can't set the team ID on a possessed pawn (%s); it's driven by the associated controller"), *GetPathNameSafe(this));
	}
}

FGenericTeamId ATenkaichiPawn::GetGenericTeamId() const
{
	return MyTeamID;
}

FOnTenkaichiTeamIndexChangedDelegate* ATenkaichiPawn::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

void ATenkaichiPawn::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	const FGenericTeamId MyOldTeamID = MyTeamID;
	MyTeamID = IntegerToGenericTeamId(NewTeam);
	ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
}

void ATenkaichiPawn::OnRep_MyTeamID(FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}