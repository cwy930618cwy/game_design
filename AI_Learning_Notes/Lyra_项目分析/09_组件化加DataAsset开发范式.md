# 09 — "功能拆组件 + 数据用 DataAsset" 开发范式详解

> **定位**：讲透 Lyra 最核心的开发范式——**"功能拆成组件（Component），配置数据用 DataAsset 存"**。这是理解整个 Lyra 架构的钥匙。
>
> **关联**：
> - [07_ModularGameplay框架在Lyra中的应用](./07_ModularGameplay框架在Lyra中的应用.md) —— 组件化的技术实现
> - [04_UPrimaryDataAsset详解](./04_UPrimaryDataAsset详解.md) —— DataAsset 这个类的细节
> - 本篇把两者**合起来**，讲"为什么要这么设计 + 实际怎么组合"
>
> **一句话**：Lyra 把"代码逻辑"和"配置数据"彻底分开——**代码拆成一个个组件，数据存进一个个 DataAsset，运行时由 DataAsset 把组件拼起来**。

---

## 一、先对比：传统做法 vs Lyra 做法

### 1.1 传统做法：一个角色类塞满功能

```cpp
// 传统：所有功能堆在一个角色类里
class AMyHero : public ACharacter
{
    // 血量逻辑
    float Health;
    void TakeDamage(float Dmg) { Health -= Dmg; }

    // 输入逻辑
    void SetupInput() { /* 绑输入 */ }

    // 相机逻辑
    void UpdateCamera() { /* 切相机 */ }

    // 技能逻辑
    void CastAbility() { /* 放技能 */ }

    // ... 越加越多，最后几千行
};
```

**问题**：
- ❌ 一个类干太多事（违反单一职责）
- ❌ 改一处怕影响全局
- ❌ 不同角色大量重复代码
- ❌ 策划想改配置得求程序员改代码
- ❌ 难测试（耦合太重）

### 1.2 Lyra 做法：功能拆组件 + 数据用 DataAsset

```
代码层（组件 Component）              数据层（DataAsset）
┌──────────────────────┐          ┌──────────────────────┐
│ ULyraHealthComponent │ ←绑定→   │  ULyraPawnData        │
│ （只管血量）          │          │  ├─ 初始血量 100       │
├──────────────────────┤          │  ├─ 初始技能集         │
│ ULyraHeroComponent   │ ←读取→   │  └─ 输入配置           │
│ （只管输入+相机）     │          └──────────────────────┘
├──────────────────────┤          ┌──────────────────────┐
│ ULyraCameraComponent │          │  UAbilitySet          │
│ （只管相机模式）      │          │  ├─ 授予哪些 GA        │
└──────────────────────┘          │  └─ 授予哪些 GE        │
                                  └──────────────────────┘
```

**好处**：
- ✅ 每个组件只干一件事（职责清晰）
- ✅ 改血量不影响输入（互不干扰）
- ✅ 组件可复用（英雄敌人共用 HealthComponent）
- ✅ 策划改 DataAsset 就能调配置（不用动代码）
- ✅ 组件可单独测试

---

## 二、两大支柱分别是什么

这套范式有**两根支柱**，缺一不可：

### 支柱一：功能拆组件（Component）

把功能从角色类里拆出来，做成独立的 `UActorComponent`：

| Lyra 组件 | 只负责一件事 |
|----------|-------------|
| `ULyraPawnExtensionComponent` | 协调其他组件的初始化（中枢） |
| `ULyraHeroComponent` | 输入绑定 + 相机管理 |
| `ULyraHealthComponent` | 生命值计算、死亡处理 |
| `ULyraCameraComponent` | 相机模式切换 |

> 详见 [07_ModularGameplay框架](./07_ModularGameplay框架在Lyra中的应用.md)

### 支柱二：数据用 DataAsset

把"配置数值"从代码里抽出来，存进 DataAsset：

| Lyra DataAsset | 存什么数据 |
|---------------|-----------|
| `ULyraPawnData` | 角色的初始配置（用哪些组件、哪些技能、什么输入） |
| `UAbilitySet` | 一组要授予的技能/效果/属性 |
| `ULyraExperienceDefinition` | 一局游戏的规则配置 |
| `ULyraInputConfig` | 输入按键映射表 |

> 详见 [04_UPrimaryDataAsset详解](./04_UPrimaryDataAsset详解.md)

---

## 三、关键：DataAsset 如何把组件"拼"起来

这才是精髓——**不是硬编码组件，而是 DataAsset 里写"我要这些组件"，运行时照着拼装**。

### 3.1 PawnData：角色的"装配清单"

```cpp
// ULyraPawnData（简化后的真实结构）
UCLASS()
class ULyraPawnData : public UPrimaryDataAsset
{
    // 要挂载哪些组件 ← 这就是"功能拆组件"的落地
    UPROPERTY(EditAnywhere)
    TArray<FModularPawnComponentEntry> Components;

    // 要授予哪些技能集 ← 这就是"数据用 DataAsset"的落地
    UPROPERTY(EditAnywhere)
    TArray<UAbilitySet*> AbilitySets;

    // 用什么输入配置
    UPROPERTY(EditAnywhere)
    ULyraInputConfig* InputConfig;

    // 技能互斥关系映射
    UPROPERTY(EditAnywhere)
    ULyraAbilityTagRelationshipMapping* TagRelationshipMapping;
};
```

**看这个结构**：PawnData 本身是个 DataAsset（数据），但它里面**列出了要用的组件清单**（Components）。这就把"数据"和"组件"通过一份配置连起来了。

### 3.2 运行时拼装流程

```
1. GameMode 生成角色时，指定一份 PawnData
       ↓
2. PawnExtensionComponent 读取 PawnData.Components
       ↓
3. 遍历数组，动态创建并注册每个组件
       ↓
4. 每个组件收到"我上线了"的回调，自行初始化
       ↓
5. 同时按 PawnData.AbilitySets 授予技能
       ↓
6. 角色组装完成 → GameplayReady
```

**核心思想**：同一个角色基类（空壳），配不同的 PawnData，就能拼出完全不同的角色——**代码不变，换数据就换角色**。

### 3.3 一个具体例子：两个角色共享同一套代码

```
ALyraCharacter（空壳，所有角色共用）
        │
        ├── 配 PawnData_Hero     → 挂 HeroComponent + 高血量 + 射击技能
        │
        └── 配 PawnData_Enemy    → 挂 EnemyComponent + 低血量 + 近战技能
```

代码完全一样，只是 DataAsset 不同，行为就完全不同。这就是"数据驱动"的威力。

---

## 四、为什么这样设计？（设计哲学）

### 4.1 分离关注点（Separation of Concerns）

```
程序员的活：写组件（怎么做）     策划的活：配 DataAsset（做什么/数值多少）
     ↓                                ↓
  HealthComponent                PawnData.Health = 100
  怎么算伤害、怎么触发死亡        血量设多少、用哪个技能集
```

两条线并行，互不阻塞。策划调数值不用等程序员。

### 4.2 组合优于继承（Composition over Inheritance）

```
继承思维：要个"会飞的敌人" → 继承 Enemy → 加飞行代码 → 类爆炸
组件思维：要个"会飞的敌人" → 给敌人挂个 FlightComponent → 完事
```

新功能 = 加新组件，不动老代码。符合"开闭原则"（对扩展开放，对修改关闭）。

### 4.3 数据驱动（Data-Driven）

游戏内容（多少个角色、什么技能、什么数值）由**数据**决定，而不是写死在代码里。这意味着：
- 加一个新角色 = 新建一份 PawnData，不改代码
- 平衡性调整 = 改 DataAsset 数值
- 玩法切换 = 换 Experience（详见 [03_Experience详解](./03_Experience详解.md)）

---

## 五、Fragment：更细粒度的数据组合（进阶）

Lyra 在 DataAsset 内部还用了 **Fragment（片段）** 模式，让数据也能像组件一样组合：

```cpp
// 物品定义（InventoryItemDefinition）里，属性靠 Fragment 拼
ULyraInventoryItemDefinition
  └── Fragments[]  ← 各种数据片段
        ├── LyraItemFragment_Stats     （基础属性）
        ├── LyraItemFragment_Equipment （装备能力）
        └── LyraItemFragment_QuickBar  （快捷栏行为）
```

**类比**：如果说组件是"代码层面的乐高"，那 Fragment 就是"数据层面的乐高"。两者思路一致：**用组合代替堆砌**。

---

## 六、动手前必须建立的认知

要按 Lyra 写法开发，这几条要刻进脑子里：

| 旧习惯（要改） | Lyra 做法（要适应） |
|--------------|-------------------|
| 功能写在 Character 里 | 拆成独立 Component |
| 数值硬编码在代码里 | 存进 DataAsset |
| 用 bool/枚举区分状态 | 用 GameplayTag |
| 角色之间复制粘贴代码 | 复用组件 + 换 DataAsset |
| 策划改配置找程序员 | 策划直接改 DataAsset |
| 一个巨大蓝图/类 | 小组件 + 数据装配 |

---

## 七、常见误区

| 误区 | 正确理解 |
|------|---------|
| "组件越多越好" | 按职责拆，一个组件一件事，别为了拆而拆 |
| "DataAsset 万能" | 它存配置数据，运行时逻辑还是组件干 |
| "组件之间随便引用" | 组件间靠消息/事件通信，别强引用（否则又耦合了） |
| "DataAsset 里写逻辑" | DataAsset 只存数据，逻辑放组件里 |
| "改了 DataAsset 要重启" | 大部分配置热加载即可生效（取决于实现） |

---

## 八、实战：手把手写一遍（完整场景）

光看概念还是虚的。这一节用**一个完整场景**，从 0 写出"功能拆组件 + 数据用 DataAsset"的真实代码。

### 8.1 场景需求

> 做一个**敌人角色**：有血量，被打会掉血，血量归零就死亡。
> 而且——**策划要能在编辑器里改初始血量**，不用动代码。

### 8.2 拆解：按 Lyra 范式该建什么

对照范式，我们要建 **3 样东西**：

```
1. 一个 DataAsset      → 存"初始血量多少"（策划改这里）
2. 一个 Component      → 管"血量逻辑"（怎么掉血、怎么死）
3. 一个 Character      → 空壳，把组件挂上去
```

注意分工：**数值在 DataAsset，逻辑在 Component，角色只是容器**。

---

### 8.3 第一步：写 DataAsset（存数据）

新建 `EnemyData.h`——这份资产就是策划以后改的地方：

```cpp
// EnemyData.h
#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyData.generated.h"

// 敌人的配置数据（策划在这里改数值，不用动代码）
UCLASS(BlueprintType)
class UEnemyData : public UPrimaryDataAsset   // ← 继承 PrimaryDataAsset（见 04 篇）
{
    GENERATED_BODY()
public:
    // 初始血量 —— 这就是"数据用 DataAsset"的体现
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    float MaxHealth = 100.f;

    // 再加个名字，方便策划辨认
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    FText DisplayName;
};
```

> 在编辑器里：右键 → Miscellaneous → Data Asset → 选 `EnemyData` → 起名 `DA_Enemy_Default`。
> 以后策划双击它就能改血量，**完全不用碰代码**。

---

### 8.4 第二步：写 Component（做逻辑）

新建 `EnemyHealthComponent.h/.cpp`——**只管血量这一件事**：

```cpp
// EnemyHealthComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyHealthComponent.generated.h"

class UEnemyData;   // 前向声明

// 血量组件：只负责"血量怎么变、怎么死"
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UEnemyHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnemyHealthComponent();

    // 用哪份数据来初始化 ← 由角色在拼装时喂给它
    void InitializeFromData(const UEnemyData* Data);

    // 掉血
    void TakeDamage(float Amount);

private:
    UPROPERTY() float CurrentHealth = 0.f;

    UFUNCTION()
    void OnDeath();   // 血量归零时触发
};
```

```cpp
// EnemyHealthComponent.cpp
#include "EnemyHealthComponent.h"
#include "EnemyData.h"

UEnemyHealthComponent::UEnemyHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;  // 不需要每帧算，省性能
}

void UEnemyHealthComponent::InitializeFromData(const UEnemyData* Data)
{
    if (!Data) return;

    // 关键：数值来自 DataAsset，不是硬编码！
    CurrentHealth = Data->MaxHealth;

    UE_LOG(LogTemp, Log, TEXT("敌人初始化，血量 = %.1f"), CurrentHealth);
}

void UEnemyHealthComponent::TakeDamage(float Amount)
{
    CurrentHealth -= Amount;
    UE_LOG(LogTemp, Log, TEXT("受到 %.1f 伤害，剩余血量 %.1f"), Amount, CurrentHealth);

    if (CurrentHealth <= 0.f)
    {
        OnDeath();   // 血量归零 → 死亡
    }
}

void UEnemyHealthComponent::OnDeath()
{
    UE_LOG(LogTemp, Log, TEXT("敌人死亡！"));
    // 真实项目里这里会：播死亡动画、禁用碰撞、掉落物品、销毁角色等
    GetOwner()->Destroy();   // 简单起见直接销毁
}
```

> 看这个组件：**它只知道"血量逻辑"，完全不知道敌人长啥样、用什么数据**。
> 这就是"单一职责"——以后任何角色都能复用这个组件。

---

### 8.5 第三步：写 Character（空壳容器 + 拼装）

新建 `EnemyCharacter.h/.cpp`——**角色本身几乎不写逻辑，只负责在初始化时把组件和数据拼起来**：

```cpp
// EnemyCharacter.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class UEnemyHealthComponent;
class UEnemyData;

UCLASS()
class AEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyCharacter();

    // 要用的数据资产（在编辑器里给每个敌人实例指定不同 DataAsset）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    TObjectPtr<UEnemyData> EnemyData;

    virtual void BeginPlay() override;

private:
    // 持有血量组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UEnemyHealthComponent> HealthComponent;
};
```

```cpp
// EnemyCharacter.cpp
#include "EnemyCharacter.h"
#include "EnemyHealthComponent.h"
#include "EnemyData.h"

AEnemyCharacter::AEnemyCharacter()
{
    // 创建组件 —— "功能拆组件"的落地
    HealthComponent = CreateDefaultSubobject<UEnemyHealthComponent>(TEXT("HealthComponent"));
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 关键：把 DataAsset 喂给组件，完成"拼装"
    // 数据（EnemyData）+ 组件（HealthComponent）在此刻结合
    if (HealthComponent && EnemyData)
    {
        HealthComponent->InitializeFromData(EnemyData);
    }
}
```

> 看这个角色类：**它几乎空的**——没有血量逻辑、没有死亡逻辑，那些都在组件里。
> 它只干一件事：`BeginPlay` 时把 DataAsset 喂给组件，让组件自己去初始化。

---

### 8.6 第四步：验证"数据驱动"的威力

现在你有两个敌人实例，想做一个**高血量 Boss**：

```
传统做法：改代码，把 MaxHealth 写大，重新编译……
Lyra 做法：选中 Boss 那个 EnemyCharacter 实例 → 细节面板把 EnemyData 换成 DA_Enemy_Boss
         （DA_Enemy_Boss 里把 MaxHealth 设成 500）→ 完事，不用编译
```

**同一个角色类、同一份组件代码**，只换了一份 DataAsset，就变成了完全不同的敌人。这就是这套范式的终极目标。

---

### 8.7 对比：这段代码体现了哪些 Lyra 范式

| 你写的 | 体现的范式 |
|--------|-----------|
| `UEnemyData : UPrimaryDataAsset` | 数据用 DataAsset |
| `UEnemyHealthComponent` 只管血量 | 功能拆组件（单一职责） |
| `AEnemyCharacter` 几乎空壳 | 角色是容器，不堆逻辑 |
| `InitializeFromData(Data)` | DataAsset 把组件"拼"起来 |
| 换 DataAsset 换角色 | 数据驱动，不改代码 |

### 8.8 升级到 Lyra 真实级别（了解即可）

上面是**教学简化版**。Lyra 真实项目里会更进一步：

| 教学版 | Lyra 真实版 |
|--------|------------|
| 角色自己创建组件 | `AModularCharacter` 空壳 + `PawnData.Components` 动态拼装 |
| `BeginPlay` 里喂数据 | `PawnExtensionComponent` 协调 + InitState 四阶段保证时序 |
| 组件直接调用 | 组件间靠 GameplayMessage 消息通信（不互相强引用） |
| 手写血量 | 走 GAS（AttributeSet + GameplayEffect） |

> 这些进阶内容分别在：
> - [07_ModularGameplay框架](./07_ModularGameplay框架在Lyra中的应用.md) — 动态拼装
> - [08_InitState初始化状态机](./08_InitState初始化状态机详解.md) — 时序保障
> - [04_UPrimaryDataAsset详解](./04_UPrimaryDataAsset详解.md) — DataAsset 细节

**但核心思想完全一样**：先把上面的简化版吃透，再去看 Lyra 真实代码，你会发现只是"多了几层包装"，骨架没变。

---

## 九、总结速查

```
Lyra 开发范式 = 功能拆组件 + 数据用 DataAsset

代码层：Component（组件）
  ├─ 每个组件只干一件事
  ├─ 组件间靠消息通信（不互相强引用）
  └─ 复用组件 = 复用功能

数据层：DataAsset（配置资产）
  ├─ PawnData = 角色装配清单（列出要用哪些组件/技能）
  ├─ AbilitySet = 技能包
  ├─ Experience = 玩法规则
  └─ 策划可直接编辑，不用改代码

运行期：DataAsset 把组件拼起来
  └─ 同一份代码 + 不同 DataAsset = 不同角色/玩法
```

**一句话**：Lyra 把"怎么做"（组件代码）和"做什么/数值多少"（DataAsset 数据）彻底分开，运行时用数据把组件拼装成完整角色。**改数据即改游戏，不用动代码**——这就是数据驱动 + 组件化的力量。

---

## 十、下一步

- [07_ModularGameplay框架在Lyra中的应用](./07_ModularGameplay框架在Lyra中的应用.md) — 组件化的技术实现
- [04_UPrimaryDataAsset详解](./04_UPrimaryDataAsset详解.md) — DataAsset 类细节
- [03_Experience详解](./03_Experience详解.md) — 最高层的数据驱动玩法切换
- [08_InitState初始化状态机详解](./08_InitState初始化状态机详解.md) — 拼装过程的时序保障
