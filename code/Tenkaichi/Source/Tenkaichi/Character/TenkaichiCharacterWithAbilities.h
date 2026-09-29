#pragma once

#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "TenkaichiCharacterWithAbilities.generated.h"

class UAbilitySystemComponent;
class UTenkaichiAbilitySystemComponent;
class UTenkaichiHealthSet;

// ===== 1. 顶部新增前向声明（class 声明区） =====
class UTenkaichiInputConfig;
struct FInputActionValue;

UCLASS()
class ATenkaichiCharacterWithAbilities : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATenkaichiCharacterWithAbilities(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	// 一比一还原 Lyra：实现 IAbilitySystemInterface 的纯虚函数，必须写 override
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 角色被玩家接管时，引擎会自动调用，在这里把输入绑上（对应 LyraCharacter.h:162）
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	// 自带 ASC（对应 LyraCharacterWithAbilities.h 第 32-33 行）
	UPROPERTY(VisibleAnywhere, Category = "Tenkaichi|Character")
	TObjectPtr<UTenkaichiAbilitySystemComponent> AbilitySystemComponent;

	// 血量属性集（对应第 36-37 行；我们不含 CombatSet）
	UPROPERTY()
	TObjectPtr<UTenkaichiHealthSet> HealthSet;

// ===== 3. protected 区新增：两个输入回调（对应 LyraHeroComponent.h:84-85） =====
protected:
	// 移动输入回调（WASD 前后左右）
	void Input_Move(const FInputActionValue& InputActionValue);

	// 鼠标转视角回调
	void Input_LookMouse(const FInputActionValue& InputActionValue);

	// ===== 4. 新增成员：输入配置资产引用（路线B用编辑器直指，替代 Lyra 的 PawnData） =====
	UPROPERTY(EditDefaultsOnly, Category = "Tenkaichi|Input")
	TObjectPtr<const UTenkaichiInputConfig> InputConfig;

	// 默认输入映射上下文（IMC）：把「按键」接到「InputAction」（对应 LyraHeroComponent.h:106 的 DefaultInputMappings）
	UPROPERTY(EditDefaultsOnly, Category = "Tenkaichi|Input")
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;
};