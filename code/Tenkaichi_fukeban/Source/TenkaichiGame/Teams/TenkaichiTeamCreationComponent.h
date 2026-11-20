// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/GameStateComponent.h"

#include "TenkaichiTeamCreationComponent.generated.h"

class UTenkaichiExperienceDefinition;
class ATenkaichiTeamPublicInfo;
class ATenkaichiTeamPrivateInfo;
class ATenkaichiPlayerState;
class AGameModeBase;
class APlayerController;
class UTenkaichiTeamDisplayAsset;

UCLASS(Blueprintable)
class UTenkaichiTeamCreationComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	// 构造函数：给两个队伍信息类填上默认类型
	UTenkaichiTeamCreationComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UObject 接口开始
#if WITH_EDITOR
	// 编辑器里校验数据是否合法（当前只调用父类，具体校验待补）
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~UObject 接口结束

	//~UActorComponent 接口开始
	// 组件开始运行时调用：等待体验（Experience）加载完成后再开始建队
	virtual void BeginPlay() override;
	//~UActorComponent 接口结束

private:
	// 体验加载完成的回调：在服务器端创建所有队伍并给已有玩家分配队伍
	void OnExperienceLoaded(const UTenkaichiExperienceDefinition* Experience);

protected:
	// 要创建的队伍列表（队号 → 外观资产的映射，外观资产可以留空）
	UPROPERTY(EditDefaultsOnly, Category = Teams)
	TMap<uint8, TObjectPtr<UTenkaichiTeamDisplayAsset>> TeamsToCreate;

	// 生成"公开队伍信息"Actor 所用的类
	UPROPERTY(EditDefaultsOnly, Category=Teams)
	TSubclassOf<ATenkaichiTeamPublicInfo> PublicTeamInfoClass;

	// 生成"私有队伍信息"Actor 所用的类
	UPROPERTY(EditDefaultsOnly, Category=Teams)
	TSubclassOf<ATenkaichiTeamPrivateInfo> PrivateTeamInfoClass;

#if WITH_SERVER_CODE
protected:
	// 服务器端：按配置表逐个创建队伍
	virtual void ServerCreateTeams();
	// 服务器端：给当前已存在的玩家分配队伍，并监听之后新登录的玩家
	virtual void ServerAssignPlayersToTeams();

	/** 为给定的 PlayerState 设置队伍 ID。仅观战的 PlayerState 会被解除所有队伍关联。 */
	virtual void ServerChooseTeamForPlayer(ATenkaichiPlayerState* PS);

private:
	// 新玩家初始化完成的回调：为其分配队伍
	void OnPlayerInitialized(AGameModeBase* GameMode, AController* NewPlayer);
	// 服务器端：为指定队号生成一对队伍信息 Actor（公开 + 私有）并设置外观
	void ServerCreateTeam(int32 TeamId, UTenkaichiTeamDisplayAsset* DisplayAsset);

	/** 返回当前人数最少的队伍 ID，若没有任何有效队伍则返回 INDEX_NONE */
	int32 GetLeastPopulatedTeamID() const;
#endif
};