
#include "TenkaichiCharacterWithAbilities.h"

#include "AbilitySystem/Attributes/TenkaichiCombatSet.h"
#include "AbilitySystem/Attributes/TenkaichiHealthSet.h"
#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "Async/TaskGraphInterfaces.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiCharacterWithAbilities)

// 构造函数：创建 ASC + 2 个属性集 + 复制设置 + 高频更新
ATenkaichiCharacterWithAbilities::ATenkaichiCharacterWithAbilities(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 创建能力系统组件子对象，开启复制，用"混合"复制模式
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UTenkaichiAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// 这两个属性集会被 AbilitySystemComponent::InitializeComponent 检测到。
	// 先持有引用，防止在那之前被垃圾回收掉。
	HealthSet = CreateDefaultSubobject<UTenkaichiHealthSet>(TEXT("HealthSet"));
	CombatSet = CreateDefaultSubobject<UTenkaichiCombatSet>(TEXT("CombatSet"));

	// ASC 需要高频率更新（能力/属性变化要及时同步）
	SetNetUpdateFrequency(100.0f);
}

// 组件初始化完成：把 ASC 与角色 Actor 绑定
void ATenkaichiCharacterWithAbilities::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);	// 本角色既当 owner 又当 avatar
}

// 覆盖基类：返回本角色自己的 ASC
UAbilitySystemComponent* ATenkaichiCharacterWithAbilities::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

