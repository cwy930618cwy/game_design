// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"

#include "TenkaichiAbilitySourceInterface.generated.h"

class UObject;
class UPhysicalMaterial;
struct FGameplayTagContainer;

/** 作为「技能计算来源」的任何东西都要实现的基础接口 */
UINTERFACE()
class UTenkaichiAbilitySourceInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class ITenkaichiAbilitySourceInterface
{
	GENERATED_IINTERFACE_BODY()

	/**
	 * 计算「距离导致的技能衰减」系数
	 *
	 * @param Distance			技能计算中，从来源到目标的距离（例如子弹飞行的距离）
	 * @param SourceTags		来源身上聚合的标签
	 * @param TargetTags		目标身上当前聚合的标签
	 *
	 * @return 因距离而要乘到「基础属性值」上的系数
	 */
	virtual float GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const = 0;

	/**
	 * 计算「物理材质导致的技能衰减」系数
	 *
	 * @param PhysicalMaterial	目标表面的物理材质
	 * @param SourceTags		来源身上聚合的标签
	 * @param TargetTags		目标身上当前聚合的标签
	 *
	 * @return 因物理材质而要乘到「基础属性值」上的系数
	 */
	virtual float GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const = 0;
};