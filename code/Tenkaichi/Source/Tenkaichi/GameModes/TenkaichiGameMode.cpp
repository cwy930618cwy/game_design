#include "TenkaichiGameMode.h"
#include "Character/TenkaichiCharacterWithAbilities.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameMode)

ATenkaichiGameMode::ATenkaichiGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 玩家的默认身体 = 我们 P01 建好的带 GAS 角色类（对应 LyraGameMode.cpp 第 41 行）
	DefaultPawnClass = ATenkaichiCharacterWithAbilities::StaticClass();
}