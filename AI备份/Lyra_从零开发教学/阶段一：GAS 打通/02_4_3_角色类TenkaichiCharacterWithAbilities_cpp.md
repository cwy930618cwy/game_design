# 02-4-3 — 第 02 课 · 第 4 步【3/3】：写 `TenkaichiCharacterWithAbilities.cpp`（把 ASC + HealthSet 造出来挂上）

> **对应 Lyra**：`Source/LyraGame/Character/LyraCharacterWithAbilities.cpp`（第 12-38 行）
>
> **一句话**：在构造函数里用 `CreateDefaultSubobject` 把 ASC 和 HealthSet 真的造出来挂上（对应 Lyra 第 15-24 行），在 `PostInitializeComponents` 里调 `InitAbilityActorInfo(this, this)`（第 27-33 行），并实现 `GetAbilitySystemComponent()` 返回 ASC 成员（第 35-38 行）。做完即第 4 步验收：PIE 里角色身上能看到 ASC + HealthSet 子对象。

---

## 一、这一小步要解决什么问题

上一步（4-2）建好 `.h`——**声明**了角色身上挂 ASC + HealthSet，但零件还没造出来。

本步写 `.cpp`，干三件事：
1. **构造函数**：`CreateDefaultSubobject` 造 ASC + HealthSet 并挂上，设网络复制。
2. **`PostInitializeComponents`**：`InitAbilityActorInfo` 告诉 ASC"宿主是我"。
3. **`GetAbilitySystemComponent`**：返回 ASC 成员（实现接口纯虚函数）。

**验收**：PIE 里角色身上能看到 ASC + HealthSet 两个子对象。

---

## 二、三个函数为什么这么写（先讲清再给代码）

### 函数 ① 构造函数——为什么用 `CreateDefaultSubobject`

`CreateDefaultSubobject` 是 UE"在构造函数里造子对象组件"的**标准做法**：new 对象 → 挂到 `this` → 纳入序列化/复制/GC 体系。Lyra 第 15、20 行就这么造。

- ASC 用 `ObjectInitializer.CreateDefaultSubobject<...>`（第 15 行）：支持蓝图/编辑器的组件覆盖。
- 属性集用 `CreateDefaultSubobject<...>`（第 20 行，不带 ObjectInitializer）：属性集不需要覆盖机制。
- 造完 ASC 立刻设：`SetIsReplicated(true)` + `SetReplicationMode(Mixed)`（第 16-17 行）。

### 函数 ② `PostInitializeComponents`——为什么在这里调 `InitAbilityActorInfo`

`PostInitializeComponents` 是引擎在"所有组件初始化完之后"调的回调（`Actor.h` 第 3114 行）。此时 ASC 已造好、属性集已被 ASC 在 `InitializeComponent` 阶段检测进来。这时调 `InitAbilityActorInfo(this, this)`——告诉 ASC"Owner 和 Avatar 都是我"，ASC 才能开始工作。

> `InitAbilityActorInfo(Owner, Avatar)` 两参都传 `this` = 4-1 讲的"②self-contained 自带"挂法。

### 函数 ③ `GetAbilitySystemComponent`——为什么直接返回成员

4-2 实现接口的那个纯虚函数的**定义**。Lyra 第 35-38 行就是直接 `return AbilitySystemComponent;`（ASC 挂在角色自己身上）。

---

## 三、减法说明（配合 16/17 号铁律）

Lyra 真实 `.cpp` 还有 `CombatSet = CreateDefaultSubobject<ULyraCombatSet>(...)`（第 21 行）+ 它的 include。**这里做减法删掉**：
- **删了什么**：`CombatSet` 创建 + `#include "AbilitySystem/Attributes/LyraCombatSet.h"`。
- **为什么能删**：CombatSet 是战斗/伤害阶段才用，阶段一只有 Health。
- **将来怎么补**：到"伤害/战斗"课补回 `TenkaichiCombatSet` + 这行。

其余**全部照抄 Lyra**（含 `#include "Async/TaskGraphInterfaces.h"`，Lyra 有就保留）。

---

## 四、你要写的代码（`Source/Tenkaichi/Character/TenkaichiCharacterWithAbilities.cpp`）

```cpp
#include "Character/TenkaichiCharacterWithAbilities.h"

#include "AbilitySystem/Attributes/TenkaichiHealthSet.h"
#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiCharacterWithAbilities)

ATenkaichiCharacterWithAbilities::ATenkaichiCharacterWithAbilities(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UTenkaichiAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
	HealthSet = CreateDefaultSubobject<UTenkaichiHealthSet>(TEXT("HealthSet"));

	// AbilitySystemComponent needs to be updated at a high frequency.
	SetNetUpdateFrequency(100.0f);
}

void ATenkaichiCharacterWithAbilities::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent* ATenkaichiCharacterWithAbilities::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
```

---

## 五、逐段回扣"为什么"

| 代码 | 为什么这么写 |
|------|-------------|
| `#include "Character/..."` + 两个属性集/ASC 头 | 对应 Lyra 的 include（减法：去掉 CombatSet 的 include） |
| `UE_INLINE_GENERATED_CPP_BY_NAME(...)` | 引擎宏（`ObjectMacros.h` 第 734 行），内联 UHT 生成的 `.gen.cpp`，每个 `.cpp` 都要有 |
| 构造函数 `CreateDefaultSubobject` 造 ASC + 设复制 | 对应 Lyra 第 15-17、20 行，照抄 |
| `PostInitializeComponents` 里 `InitAbilityActorInfo(this, this)` | 对应 Lyra 第 32 行；宿主=角色自己（self-contained） |
| `GetAbilitySystemComponent() const` 返回成员 | 对应 Lyra 第 35-38 行 |

---

## 六、核对②（.h ↔ .cpp 成对）

4-2 的 `.h` 声明了 3 个函数，本 `.cpp` **一一实现**，名字/参数/`const` 完全一致：
1. `ATenkaichiCharacterWithAbilities(const FObjectInitializer&)`
2. `PostInitializeComponents()`
3. `GetAbilitySystemComponent() const`

---

## 七、核实过的符号（铁律 12，都实际打开确认）

| 符号 | 出处 |
|------|------|
| `InitAbilityActorInfo(AActor*, AActor*)` | `AbilitySystemComponent.h` 第 1551 行 |
| `SetReplicationMode(EGameplayEffectReplicationMode)` | `AbilitySystemComponent.h` 第 259 行 |
| `enum class EGameplayEffectReplicationMode` | `AbilitySystemComponent.h` 第 81 行 |
| `UE_INLINE_GENERATED_CPP_BY_NAME(name)` | `ObjectMacros.h` 第 734 行 |

---

## 八、一句话结论

**`.cpp` = 构造函数用 `CreateDefaultSubobject` 造 ASC（设复制+Mixed 模式）和 HealthSet 挂上 + `PostInitializeComponents` 调 `InitAbilityActorInfo(this,this)` + `GetAbilitySystemComponent()` 返回 ASC 成员。除删掉 CombatSet（做减法）外，全部照抄 Lyra。**

---

## 九、下一步

第 4 步完成。验收：编译后 PIE，角色身上能看到 `TenkaichiAbilitySystemComponent` + `TenkaichiHealthSet` 两个子对象。**第 5 步**将教"授予属性集 / 初始化血量"（对应 Lyra 后续初始化流程），继续按 Lyra 真实代码一比一推进。
