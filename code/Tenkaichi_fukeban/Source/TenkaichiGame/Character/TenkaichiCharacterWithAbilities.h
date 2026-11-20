#include "Character/TenkaichiCharacter.h"

#include "TenkaichiCharacterWithAbilities.generated.h"

#define UE_API TENKAICHIGAME_API

class UAbilitySystemComponent;
class UTenkaichiAbilitySystemComponent;
class UObject;

// 基础角色（ATenkaichiCharacter）的 ASC 一般从"占用它的 PlayerState"那继承。
// 这个类代表"自带一套完整能力系统组件"的角色。
UCLASS(MinimalAPI, Blueprintable)
class ATenkaichiCharacterWithAbilities : public ATenkaichiCharacter
{
	GENERATED_BODY()

public:
	// 构造函数（初始化 ASC 和属性集）
	UE_API ATenkaichiCharacterWithAbilities(const FObjectInitializer& ObjectInitializer);

	// 组件初始化完成：把 ASC 和本角色绑定（InitAbilityActorInfo）
	UE_API virtual void PostInitializeComponents() override;

	// 覆盖基类的接口方法：返回本角色自己的 ASC
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:

	// 玩家角色用的能力系统组件子对象（本角色自带）
	UPROPERTY(VisibleAnywhere, Category = "Tenkaichi|PlayerState")
	TObjectPtr<UTenkaichiAbilitySystemComponent> AbilitySystemComponent;

	// 本角色用的血量属性集
	UPROPERTY()
	TObjectPtr<const class UTenkaichiHealthSet> HealthSet;

	// 本角色用的攻击属性集
	UPROPERTY()
	TObjectPtr<const class UTenkaichiCombatSet> CombatSet;
}