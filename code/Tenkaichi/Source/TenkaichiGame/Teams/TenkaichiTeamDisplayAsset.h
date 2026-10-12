// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "TenkaichiTeamDisplayAsset.generated.h"

struct FPropertyChangedEvent;

class UMaterialInstanceDynamic;
class UMeshComponent;
class UNiagaraComponent;
class AActor;
class UTexture;

// 队伍的显示信息（颜色、显示名、贴图等）
UCLASS(BlueprintType)
class UTenkaichiTeamDisplayAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 标量参数表：名字 → 数值（如自发光强度、透明度）
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, float> ScalarParameters;

	// 颜色参数表：名字 → 颜色（如阵营主色、描边色）
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, FLinearColor> ColorParameters;

	// 贴图参数表：名字 → 贴图（如阵营图标、纹理）
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, TObjectPtr<UTexture>> TextureParameters;

	// 队伍短名（UI 显示用，如 "RED"）
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText TeamShortName;

public:
	// 把这份队伍的标量/颜色/贴图参数，全部刷到一个动态材质上
	UFUNCTION(BlueprintCallable, Category=Teams)
	void ApplyToMaterial(UMaterialInstanceDynamic* Material);

	// 把这份队伍的标量/颜色/贴图参数，刷到网格组件的所有材质上
	UFUNCTION(BlueprintCallable, Category=Teams)
	void ApplyToMeshComponent(UMeshComponent* MeshComponent);

	// 把这份队伍的标量/颜色/贴图参数，刷到一个 Niagara 粒子组件上
	UFUNCTION(BlueprintCallable, Category=Teams)
	void ApplyToNiagaraComponent(UNiagaraComponent* NiagaraComponent);

	// 遍历目标 Actor 的所有组件，把队伍外观刷到每个可用的组件上（可含子组件）
	UFUNCTION(BlueprintCallable, Category=Teams, meta=(DefaultToSelf="TargetActor"))
	void ApplyToActor(AActor* TargetActor, bool bIncludeChildActors = true);

public:

	//~UObject interface
#if WITH_EDITOR
	// 编辑器里改了属性时调用：通知队伍子系统刷新（依赖 TeamSubsystem，阶段五解挂）
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~End of UObject interface
};