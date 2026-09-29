# 02-4-2 — 第 02 课 · 第 4 步【2/3】：写 `TenkaichiCharacterWithAbilities.h`（角色类头文件骨架）

> **对应 Lyra**：`Source/LyraGame/Character/LyraCharacterWithAbilities.h`（第 17-41 行）
>
> **一句话**：给"ASC + 属性集"找一个身体——建角色类 `ATenkaichiCharacterWithAbilities` 的 `.h` 骨架（**类名一比一对应 Lyra 的 `ALyraCharacterWithAbilities`**）：声明两个 `UPROPERTY` 成员（ASC + HealthSet）、构造函数、`PostInitializeComponents`，并实现 `IAbilitySystemInterface` 接口 + `GetAbilitySystemComponent()` 写 `override`。本小步只写 `.h`，`.cpp` 在 4-3。

---

## 一、这一小步要解决什么问题

上一步（4-1）结论：**ASC 和属性集是组件，必须挂在一个活着的 Actor 身上才有"宿主"**。

这一步就是去建这个"身体"。本小步只搭 `.h` 头文件骨架——声明"角色身上挂哪两个零件"，`.cpp`（真正用 `CreateDefaultSubobject` 造零件）留到 4-3。

**验收**：`.h` 建好、能编译（配合 4-3 的 `.cpp` 后整体通过）。

---

## 二、命名：一比一对应 Lyra（重要）

| Lyra 真实 | Tenkaichi 对应 |
|-----------|---------------|
| `ALyraCharacterWithAbilities` | `ATenkaichiCharacterWithAbilities` |
| `LyraCharacterWithAbilities.h/.cpp` | `TenkaichiCharacterWithAbilities.h/.cpp` |
| 路径 `Source/LyraGame/Character/` | `Source/Tenkaichi/Character/` |

> ⚠️ **纠错（2026-09-24）**：本小步最初把类名擅自简化成 `ATenkaichiCharacter`（砍掉了 `WithAbilities`），违反 01/10 号铁律（一比一还原、禁止擅自简化）。已改回 `ATenkaichiCharacterWithAbilities`，与 Lyra 一一对应。**Lyra 叫什么名，我们就叫什么名，只换前缀。**

---

## 三、三个关键点（先讲清为什么，再给代码）

### 关键点 ①：实现 `IAbilitySystemInterface` + 写 `override`（照 Lyra）

Lyra 的 `ALyraCharacterWithAbilities` **实现了 `IAbilitySystemInterface` 且写了 `override`**（`.h` 第 27 行）。我们一比一还原。

先核实接口签名（引擎源码）：

```25:26:Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AbilitySystemInterface.h
	/** Returns the ability system component to use for this actor. It may live on another actor, such as a Pawn using the PlayerState's component */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const = 0;
```

末尾 `= 0` 是**纯虚函数**：实现该接口者**必须**实现此方法且写 `override`。

**前提核对（都已实际打开确认）**：
- 接口头文件：`#include "AbilitySystemInterface.h"`（在 `GameplayAbilities` 插件 Public 目录）。
- 模块依赖：`GameplayAbilities` 已在 `Tenkaichi.Build.cs` 第 17 行，**无需改 Build.cs**。
- 接口标记 `CannotImplementInterfaceInBlueprint`（第 15 行）→ 只能 C++ 实现，我们正是 C++。

### 关键点 ②：两个 `UPROPERTY()` 成员（ASC + HealthSet）

对应 Lyra `.h` 第 32-37 行。作用不是装饰，是"**持有引用防止 GC**"：属性集靠 ASC 的 `InitializeComponent` 自动检测进来，但得有人持有引用保命（4-1 讲过）。所以 `UPROPERTY()` 不能省。

### 关键点 ③：`PostInitializeComponents()` 声明

引擎 `AActor` 的 `virtual` 函数（`Actor.h` 第 3114 行），我们 `override` 它，将来在 `.cpp` 里调 `InitAbilityActorInfo`（4-3 的事）。

---

## 四、你要写的代码（`Source/Tenkaichi/Character/TenkaichiCharacterWithAbilities.h`）

```cpp
#pragma once

#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "TenkaichiCharacterWithAbilities.generated.h"

class UAbilitySystemComponent;
class UTenkaichiAbilitySystemComponent;
class UTenkaichiHealthSet;

UCLASS()
class ATenkaichiCharacterWithAbilities : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATenkaichiCharacterWithAbilities(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	// 一比一还原 Lyra：实现 IAbilitySystemInterface 的纯虚函数，必须写 override
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	// 自带 ASC（对应 LyraCharacterWithAbilities.h 第 32-33 行）
	UPROPERTY(VisibleAnywhere, Category = "Tenkaichi|Character")
	TObjectPtr<UTenkaichiAbilitySystemComponent> AbilitySystemComponent;

	// 血量属性集（对应第 36-37 行；我们不含 CombatSet）
	UPROPERTY()
	TObjectPtr<UTenkaichiHealthSet> HealthSet;
};
```

---

## 五、逐段回扣"为什么"

| 代码 | 为什么这么写 |
|------|-------------|
| `#include "GameFramework/Character.h"` | 要继承 `ACharacter`，得先引进基类头文件 |
| `#include "AbilitySystemInterface.h"` | 要实现 `IAbilitySystemInterface`，得先引进接口头文件 |
| 三个前置声明（`class UAbilitySystemComponent;` 等） | 头文件里只用到**指针**，前置声明即可，避免 `#include` 整类增加编译依赖（同 Lyra `.h` 第 11-13 行做法） |
| `class ATenkaichiCharacterWithAbilities : public ACharacter, public IAbilitySystemInterface` | 类名一比一对应 Lyra 的 `ALyraCharacterWithAbilities`；基类降到引擎 `ACharacter`（不接 Lyra 角色继承链），但**同样实现 `IAbilitySystemInterface`** |
| `GetAbilitySystemComponent() const override` | 实现接口的纯虚函数，必须写 override（对应 Lyra `.h` 第 27 行） |
| 两个 `UPROPERTY()` 成员 | 持有引用防 GC，不能省（对应 Lyra 第 32-37 行） |

---

## 六、核对②预告（.h ↔ .cpp 成对）

这个 `.h` 声明了 **3 个函数**：
1. 构造函数 `ATenkaichiCharacterWithAbilities(const FObjectInitializer&)`
2. `PostInitializeComponents()`
3. `GetAbilitySystemComponent() const`

下一小步 4-3 的 `.cpp` 会**一一实现**这 3 个，名字 / 参数 / `const` 完全一致（配合 11 号铁律）。

---

## 七、我们这版 vs Lyra 的差异

| 项 | Lyra | 我们 | 说明 |
|---|------|------|------|
| 类名 | `ALyraCharacterWithAbilities` | `ATenkaichiCharacterWithAbilities` | 一比一对应，只换前缀 |
| 基类 | `ALyraCharacter`（实现 `IAbilitySystemInterface`） | `ACharacter`（同样实现 `IAbilitySystemInterface`） | 基类降到引擎 `ACharacter`（不接 Lyra 角色继承链），但接口保持一致 |
| `GetAbilitySystemComponent()` | `override` | `override` | 一比一还原 |
| 属性集数量 | `HealthSet` + `CombatSet` | 仅 `HealthSet` | CombatSet 依赖战斗阶段，后续再加（非遗漏） |
| `Category` | `"Lyra\|PlayerState"` | `"Tenkaichi\|Character"` | 仅编辑器分类显示 |

---

## 八、一句话结论

**角色类 `.h` 骨架 = 类名 `ATenkaichiCharacterWithAbilities`（一比一对应 Lyra）+ 继承 `ACharacter` 并实现 `IAbilitySystemInterface` + 两个 `UPROPERTY` 成员（ASC/HealthSet）+ 构造函数 + `PostInitializeComponents` + `GetAbilitySystemComponent() const override`。**

---

## 九、下一步

**02-4-3：写 `Source/Tenkaichi/Character/TenkaichiCharacterWithAbilities.cpp`**——在构造函数里用 `CreateDefaultSubobject` 把 ASC 和 HealthSet 造出来挂上（对应 Lyra `.cpp` 第 15-24 行），并实现 `PostInitializeComponents` 里的 `InitAbilityActorInfo`（第 27-33 行）。实现完，第 4 步验收：PIE 里角色身上能看到 ASC + HealthSet 子对象。
