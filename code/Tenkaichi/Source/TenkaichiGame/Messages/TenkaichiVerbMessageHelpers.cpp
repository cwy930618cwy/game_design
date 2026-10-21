// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiVerbMessageHelpers.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Messages/TenkaichiVerbMessage.h"

// ⚠️ 注释挂起：下面这行依赖 GAS 模块（GameplayEffectTypes.h），GAS 核心在 P05 才教，到那时放开
// #include "GameplayEffectTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiVerbMessageHelpers)

//////////////////////////////////////////////////////////////////////
// FTenkaichiVerbMessage

// 把消息体序列化成一行可读字符串（用引擎反射 ExportText）
FString FTenkaichiVerbMessage::ToString() const
{
	FString HumanReadableMessage;
	FTenkaichiVerbMessage::StaticStruct()->ExportText(/*out*/ HumanReadableMessage, this, /*Defaults=*/ nullptr, /*OwnerObject=*/ nullptr, PPF_None, /*ExportRootScope=*/ nullptr);
	return HumanReadableMessage;
}

//////////////////////////////////////////////////////////////////////
//

// 从任意对象（Controller/PlayerState/Pawn 等）反查它所属的 PlayerState
APlayerState* UTenkaichiVerbMessageHelpers::GetPlayerStateFromObject(UObject* Object)
{
	if (APlayerController* PC = Cast<APlayerController>(Object))
	{
		return PC->PlayerState;
	}

	if (APlayerState* TargetPS = Cast<APlayerState>(Object))
	{
		return TargetPS;
	}
	
	if (APawn* TargetPawn = Cast<APawn>(Object))
	{
		if (APlayerState* TargetPS = TargetPawn->GetPlayerState())
		{
			return TargetPS;
		}
	}
	return nullptr;
}

// 从任意对象（Controller/PlayerState/Pawn 等）反查它所属的 PlayerController
APlayerController* UTenkaichiVerbMessageHelpers::GetPlayerControllerFromObject(UObject* Object)
{
	if (APlayerController* PC = Cast<APlayerController>(Object))
	{
		return PC;
	}

	if (APlayerState* TargetPS = Cast<APlayerState>(Object))
	{
		return TargetPS->GetPlayerController();
	}

	if (APawn* TargetPawn = Cast<APawn>(Object))
	{
		return Cast<APlayerController>(TargetPawn->GetController());
	}

	return nullptr;
}

// ⚠️ 以下两段依赖 GAS 的 FGameplayCueParameters，GAS 核心（P05）就位后把 #if 0 改成 #if 1 或删除即可解开
#if 0
// 把一条 VerbMessage 转换成 GameplayCue 参数（消息 → GAS）
FGameplayCueParameters UTenkaichiVerbMessageHelpers::VerbMessageToCueParameters(const FTenkaichiVerbMessage& Message)
{
	FGameplayCueParameters Result;

	Result.OriginalTag = Message.Verb;
	Result.Instigator = Cast<AActor>(Message.Instigator);
	Result.EffectCauser = Cast<AActor>(Message.Target);
	Result.AggregatedSourceTags = Message.InstigatorTags;
	Result.AggregatedTargetTags = Message.TargetTags;
	//@TODO: = Message.ContextTags;
	Result.RawMagnitude = Message.Magnitude;

	return Result;
}

// 把 GameplayCue 参数转换回一条 VerbMessage（GAS 参数 → 消息）
FTenkaichiVerbMessage UTenkaichiVerbMessageHelpers::CueParametersToVerbMessage(const FGameplayCueParameters& Params)
{
	FTenkaichiVerbMessage Result;
	
	Result.Verb = Params.OriginalTag;
	Result.Instigator = Params.Instigator.Get();
	Result.Target = Params.EffectCauser.Get();
	Result.InstigatorTags = Params.AggregatedSourceTags;
	Result.TargetTags = Params.AggregatedTargetTags;
	//@TODO: Result.ContextTags = ???;
	Result.Magnitude = Params.RawMagnitude;

	return Result;
}
#endif