// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "GameFeatureAction_WorldActionBase.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/TenkaichiAbilitySet.h"

#include "GameFeatureAction_AddAbilities.generated.h"

struct FWorldContext;
class UInputAction;
class UAttributeSet;
class UDataTable;
struct FComponentRequestHandle;
class UTenkaichiAbilitySet;

// 一个要授予的能力。
USTRUCT(BlueprintType)
struct FTenkaichiAbilityGrant
{
	GENERATED_BODY()

	// 要授予的能力类型
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AssetBundles="Client,Server"))
	TSoftClassPtr<UGameplayAbility> AbilityType;

	// 要绑定到这个能力的输入动作（如果有，可以不设）
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly)
// 	TSoftObjectPtr<UInputAction> InputAction;
};

// 一个要授予的属性集。
USTRUCT(BlueprintType)
struct FTenkaichiAttributeSetGrant
{
	GENERATED_BODY()

	// 要授予的属性集
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AssetBundles="Client,Server"))
	TSoftClassPtr<UAttributeSet> AttributeSetType;

	// 用来初始化属性的数据表（如果有，可以不设）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AssetBundles="Client,Server"))
	TSoftObjectPtr<UDataTable> InitializationData;
};

// 一条配置：给某个类 Actor 加什么。
USTRUCT()
struct FGameFeatureAbilitiesEntry
{
	GENERATED_BODY()

	// 要加到的基类 Actor 类
	UPROPERTY(EditAnywhere, Category="Abilities")
	TSoftClassPtr<AActor> ActorClass;

	// 要授予给指定类 Actor 的能力列表
	UPROPERTY(EditAnywhere, Category="Abilities")
	TArray<FTenkaichiAbilityGrant> GrantedAbilities;

	// 要授予给指定类 Actor 的属性集列表
	UPROPERTY(EditAnywhere, Category="Attributes")
	TArray<FTenkaichiAttributeSetGrant> GrantedAttributes;

	// 要授予给指定类 Actor 的能力集列表
	UPROPERTY(EditAnywhere, Category="Attributes", meta=(AssetBundles="Client,Server"))
	TArray<TSoftObjectPtr<const UTenkaichiAbilitySet>> GrantedAbilitySets;
};

//////////////////////////////////////////////////////////////////////
// UGameFeatureAction_AddAbilities

/**
 * 负责给指定类型的 Actor 授予能力（和属性）的 GameFeatureAction。
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Abilities"))
class UGameFeatureAction_AddAbilities final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction 接口开始
	// 游戏特性激活时调用。
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	// 游戏特性停用时调用。
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~UGameFeatureAction 接口结束

	//~UObject 接口开始
#if WITH_EDITOR
	// 编辑器数据校验。
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~UObject 接口结束

	/**  */
	UPROPERTY(EditAnywhere, Category="Abilities", meta=(TitleProperty="ActorClass", ShowOnlyInnerProperties))
	TArray<FGameFeatureAbilitiesEntry> AbilitiesList;

private:
	// 某个 Actor 已经授予的扩展（能力句柄 / 属性集实例 / 能力集句柄）。
	struct FActorExtensions
	{
		TArray<FGameplayAbilitySpecHandle> Abilities;
		TArray<UAttributeSet*> Attributes;
		TArray<FTenkaichiAbilitySet_GrantedHandles> AbilitySetHandles;
	};

	// 每个上下文（一次激活）的数据。
	struct FPerContextData
	{
		TMap<AActor*, FActorExtensions> ActiveExtensions;
		TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequests;
	};

	// 上下文 → 该上下文的运行时数据。
	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	//~UGameFeatureAction_WorldActionBase 接口开始
	// 在某个 world 里执行：注册类扩展处理器。
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~UGameFeatureAction_WorldActionBase 接口结束

	// 重置某个上下文的数据（移除所有已授予的扩展）。
	void Reset(FPerContextData& ActiveData);
	// Actor 扩展事件回调（出现/消失时授予/移除）。
	void HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex, FGameFeatureStateChangeContext ChangeContext);
	// 给 Actor 授予能力/属性/能力集。
	void AddActorAbilities(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData);
	// 从 Actor 移除已授予的能力/属性/能力集。
	void RemoveActorAbilities(AActor* Actor, FPerContextData& ActiveData);

	// 模板版：按组件类型找或请求组件。
	template<class ComponentType>
	ComponentType* FindOrAddComponentForActor(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData)
	{
		//@TODO: 只找，不加？
		return Cast<ComponentType>(FindOrAddComponentForActor(ComponentType::StaticClass(), Actor, AbilitiesEntry, ActiveData));
	}

	// 非模板版：按组件 UClass 找或请求组件。
	UActorComponent* FindOrAddComponentForActor(UClass* ComponentType, AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData);
};