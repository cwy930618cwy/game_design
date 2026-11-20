// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Components/GameStateComponent.h"

#include "TenkaichiBotCreationComponent.generated.h"

class UTenkaichiExperienceDefinition;
class UTenkaichiPawnData;
class AAIController;

// 按配置自动生成机器人的组件（挂在 GameState 上）。
UCLASS(Blueprintable, Abstract)
class UTenkaichiBotCreationComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	// 构造函数：空实现（父类处理初始化）。
	UTenkaichiBotCreationComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent 接口开始
	// 组件开始游戏：监听"经验加载完成"。
	virtual void BeginPlay() override;
	//~UActorComponent 接口结束

private:
	// 经验加载完成的回调（最低优先级）。
	void OnExperienceLoaded(const UTenkaichiExperienceDefinition* Experience);

protected:
	// 要生成的 Bot 数量。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Gameplay)
	int32 NumBotsToCreate = 5;

	// Bot 用的 AI 控制器类。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Gameplay)
	TSubclassOf<AAIController> BotControllerClass;

	// 随机 Bot 名字池。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Gameplay)
	TArray<FString> RandomBotNames;

	// 还没用完的 Bot 名字池。
	TArray<FString> RemainingBotNames;

protected:
	// 已经生成的 Bot 控制器列表。
	UPROPERTY(Transient)
	TArray<TObjectPtr<AAIController>> SpawnedBotList;

	// 生成单个 Bot。
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Gameplay)
	virtual void SpawnOneBot();

	// 如果可能，删除最后生成的一个 Bot。
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Gameplay)
	virtual void RemoveOneBot();

	// 生成最多 NumBotsToCreate 个 Bot。
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category=Gameplay)
	void ServerCreateBots();

#if WITH_SERVER_CODE
public:
	// 作弊：加一个 Bot。
	void Cheat_AddBot() { SpawnOneBot(); }
	// 作弊：删一个 Bot。
	void Cheat_RemoveBot() { RemoveOneBot(); }

	// 按玩家索引生成一个 Bot 名字。
	FString CreateBotName(int32 PlayerIndex);
#endif
};