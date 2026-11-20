// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiEquipmentInstance.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "TenkaichiEquipmentDefinition.h"
#include "Net/UnrealNetwork.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiEquipmentInstance)

class FLifetimeProperty;
class UClass;
class USceneComponent;

// 构造函数：只调用父类构造
UTenkaichiEquipmentInstance::UTenkaichiEquipmentInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 取世界：返回穿戴 Pawn 的世界（没有就返回空）
UWorld* UTenkaichiEquipmentInstance::GetWorld() const
{
	if (APawn* OwningPawn = GetPawn())
	{
		return OwningPawn->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

// 网络复制注册：Instigator + SpawnedActors 两个字段要复制
void UTenkaichiEquipmentInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Instigator);
	DOREPLIFETIME(ThisClass, SpawnedActors);
}

#if UE_WITH_IRIS
// Iris 网络复制系统：为这个对象注册复制片段（默认关闭，不执行）
void UTenkaichiEquipmentInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	// 构建描述符并分配属性复制片段
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

// 取穿戴的 Pawn：装备实例的 Outer 就是 Pawn
APawn* UTenkaichiEquipmentInstance::GetPawn() const
{
	return Cast<APawn>(GetOuter());
}

// 取穿戴的 Pawn（带类型检查）：外层必须是指定类型才返回
APawn* UTenkaichiEquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
	APawn* Result = nullptr;
	if (UClass* ActualPawnType = PawnType)
	{
		if (GetOuter()->IsA(ActualPawnType))
		{
			Result = Cast<APawn>(GetOuter());
		}
	}

	return Result;
}

// 按定义生成装备 Actor：逐个生成、挂到角色身上
void UTenkaichiEquipmentInstance::SpawnEquipmentActors(const TArray<FTenkaichiEquipmentActorToSpawn>& ActorsToSpawn)
{
	if (APawn* OwningPawn = GetPawn())
	{
		// 默认挂到根组件；如果是角色，挂到它的网格体（Mesh）
		USceneComponent* AttachTarget = OwningPawn->GetRootComponent();
		if (ACharacter* Char = Cast<ACharacter>(OwningPawn))
		{
			AttachTarget = Char->GetMesh();
		}

		for (const FTenkaichiEquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
		{
			// 延迟生成（先拿对象、再设变换、最后完成生成）
			AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
			NewActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
			// 设置相对偏移（定义里配的 AttachTransform）
			NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
			// 挂到目标组件上（插槽 AttachSocket，保持相对变换）
			NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);

			// 记录到 SpawnedActors（随网络复制）
			SpawnedActors.Add(NewActor);
		}
	}
}

// 销毁生成的装备 Actor
void UTenkaichiEquipmentInstance::DestroyEquipmentActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
}

// 装备钩子：触发蓝图事件 K2_OnEquipped
void UTenkaichiEquipmentInstance::OnEquipped()
{
	K2_OnEquipped();
}

// 卸下钩子：触发蓝图事件 K2_OnUnequipped
void UTenkaichiEquipmentInstance::OnUnequipped()
{
	K2_OnUnequipped();
}

// Instigator 复制回调（当前空实现，留扩展位）
void UTenkaichiEquipmentInstance::OnRep_Instigator()
{
}