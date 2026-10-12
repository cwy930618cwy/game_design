// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "TenkaichiTeamInfoBase.h"

#include "TenkaichiTeamPublicInfo.generated.h"

class UTenkaichiTeamCreationComponent;
class UTenkaichiTeamDisplayAsset;
class UObject;
struct FFrame;

// 队伍的"公开"信息：除了基类的队号/标签，还多一份"队伍外观数据"（所有人可见）
UCLASS()
class ATenkaichiTeamPublicInfo : public ATenkaichiTeamInfoBase
{
	GENERATED_BODY()

	// 允许"队伍创建组件"设置公开外观数据（私有 SetTeamDisplayAsset）
	friend UTenkaichiTeamCreationComponent;

public:
	ATenkaichiTeamPublicInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 获取这支队伍的公开外观数据
	UTenkaichiTeamDisplayAsset* GetTeamDisplayAsset() const { return TeamDisplayAsset; }

private:
	// 客户端收到外观数据复制时触发
	UFUNCTION()
	void OnRep_TeamDisplayAsset();

	// 设置队伍的公开外观数据（服务器端，只能设一次）
	void SetTeamDisplayAsset(TObjectPtr<UTenkaichiTeamDisplayAsset> NewDisplayAsset);

private:
	// 队伍的公开外观数据（只复制一次，客户端收到触发 OnRep）
	UPROPERTY(ReplicatedUsing=OnRep_TeamDisplayAsset)
	TObjectPtr<UTenkaichiTeamDisplayAsset> TeamDisplayAsset;
};