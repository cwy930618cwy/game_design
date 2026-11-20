// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayEffectTypes.h"

#include "TenkaichiGameplayEffectContext.generated.h"

class AActor;
class FArchive;
class ITenkaichiAbilitySourceInterface;
class UObject;
class UPhysicalMaterial;

USTRUCT()
struct FTenkaichiGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	FTenkaichiGameplayEffectContext()
		: FGameplayEffectContext()
	{
	}

	FTenkaichiGameplayEffectContext(AActor* InInstigator, AActor* InEffectCauser)
		: FGameplayEffectContext(InInstigator, InEffectCauser)
	{
	}

	// 从 handle 里取出包裹的 FTenkaichiGameplayEffectContext，若不存在或类型不对则返回 nullptr
	static TENKAICHIGAME_API FTenkaichiGameplayEffectContext* ExtractEffectContext(struct FGameplayEffectContextHandle Handle);

	// 设置作为能力来源的对象
	void SetAbilitySource(const ITenkaichiAbilitySourceInterface* InObject, float InSourceLevel);

	// 返回与来源对象关联的能力来源接口。仅在权威端有效。
	const ITenkaichiAbilitySourceInterface* GetAbilitySource() const;

	virtual FGameplayEffectContext* Duplicate() const override
	{
		FTenkaichiGameplayEffectContext* NewContext = new FTenkaichiGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// 对命中结果做深拷贝
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FTenkaichiGameplayEffectContext::StaticStruct();
	}

	// 重写以序列化新增字段
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

	// 若有命中结果则返回其物理材质
	const UPhysicalMaterial* GetPhysicalMaterial() const;

public:
	// 用于标识同一弹匣里多颗子弹的 ID
	UPROPERTY()
	int32 CartridgeID = -1;

protected:
	// 能力来源对象（应实现 ITenkaichiAbilitySourceInterface）。目前不复制。
	UPROPERTY()
	TWeakObjectPtr<const UObject> AbilitySourceObject;
};

template<>
struct TStructOpsTypeTraits<FTenkaichiGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FTenkaichiGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};