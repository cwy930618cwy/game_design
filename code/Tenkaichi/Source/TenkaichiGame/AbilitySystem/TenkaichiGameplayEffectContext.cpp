// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiGameplayEffectContext.h"

#include "AbilitySystem/TenkaichiAbilitySourceInterface.h"
#include "Engine/HitResult.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
#include "Serialization/GameplayEffectContextNetSerializer.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameplayEffectContext)

class FArchive;

// 从 handle 里取出包裹的 FTenkaichiGameplayEffectContext，若不存在或类型不对则返回 nullptr
FTenkaichiGameplayEffectContext* FTenkaichiGameplayEffectContext::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseEffectContext = Handle.Get();
	if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(FTenkaichiGameplayEffectContext::StaticStruct()))
	{
		return (FTenkaichiGameplayEffectContext*)BaseEffectContext;
	}

	return nullptr;
}

// 网络序列化：先序列化基类，再序列化新增字段
bool FTenkaichiGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

	// 目前不序列化以下字段（用于激活后）：
	// CartridgeID

	return true;
}

#if UE_WITH_IRIS
namespace UE::Net
{
	// 转发给 FGameplayEffectContextNetSerializer
	// 注意：如果修改了 FTenkaichiGameplayEffectContext::NetSerialize()，就需要实现一个自定义 NetSerializer，
	// 因为当前的 fallback 就不够用了。
	UE_NET_IMPLEMENT_FORWARDING_NETSERIALIZER_AND_REGISTRY_DELEGATES(TenkaichiGameplayEffectContext, FGameplayEffectContextNetSerializer);
}
#endif

// 设置作为能力来源的对象
void FTenkaichiGameplayEffectContext::SetAbilitySource(const ITenkaichiAbilitySourceInterface* InObject, float InSourceLevel)
{
	AbilitySourceObject = MakeWeakObjectPtr(Cast<const UObject>(InObject));
	//SourceLevel = InSourceLevel;
}

// 返回与来源对象关联的能力来源接口。仅在权威端有效。
const ITenkaichiAbilitySourceInterface* FTenkaichiGameplayEffectContext::GetAbilitySource() const
{
	return Cast<ITenkaichiAbilitySourceInterface>(AbilitySourceObject.Get());
}

// 若有命中结果则返回其物理材质
const UPhysicalMaterial* FTenkaichiGameplayEffectContext::GetPhysicalMaterial() const
{
	if (const FHitResult* HitResultPtr = GetHitResult())
	{
		return HitResultPtr->PhysMaterial.Get();
	}
	return nullptr;
}