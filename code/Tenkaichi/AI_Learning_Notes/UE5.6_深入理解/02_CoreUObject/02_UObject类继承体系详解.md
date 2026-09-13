# 02 — UObject 类继承体系详解

> **定位**：理解 UE 里"对象是怎么分层的"——从 `UObject` 到 `AActor`、`UActorComponent`，再到你的自定义类。
>
> **一句话**：UE 的对象不是一锅粥，而是**一条清晰的继承链**。从最底层的 `UObject` 往上，一层层加上"场景里的位置"、"能交互的能力"、"驱动游戏逻辑的组件"。
>
> **文件**：`Engine/Source/Runtime/CoreUObject/Public/UObject/`、`Engine/Source/Runtime/Engine/Classes/`

---

## 一、整棵继承树（先看全貌）

```
UObject（最老祖宗，所有游戏对象的根）
├── UClass（类本身）
├── UFunction / UProperty（函数/属性的描述）
├── UField（字段系统）
│
├── AActor（能在场景里存在、有位置的对象）
│   ├── APawn（能被控制）
│   │   ├── ACharacter（有移动组件的人形角色）
│   │   └── ...
│   ├── AController（控制器，操作 Pawn）
│   ├── AVolume（体积）
│   ├── ALight（光源）
│   ├── ...（几乎所有场景里的东西）
│
├── UActorComponent（挂在 Actor 上的组件）
│   ├── USceneComponent（有位置的组件）
│   │   ├── UPrimitiveComponent（可渲染）
│   │   │   ├── UStaticMeshComponent（静态网格）
│   │   │   └── ...
│   │   ├── USkeletalMeshComponent（骨骼网格）
│   │   └── ...
│   └──（无位置的组件）
│       ├── UAbilitySystemComponent（技能）
│       └── ...
│
├── UGameInstance（整个游戏的全局实例）
├── UWorld（关卡世界）
├── ULevel（关卡）
└── ...（各种 UObject 子类）
```

---

## 一.4、继承树全类清单（说明 + 已讲/待讲）

> 这张继承树里提到的**所有类**，一个个列出来，说明干嘛的，并标注本系列是否已深入讲解。

### ① UObject 直接子类（反射系统）

| 类 | 干嘛的 | 状态 |
|------|--------|:---:|
| `UClass` | 类的描述（反射元信息） | ✅ 已讲 |
| `UFunction` | 函数的描述 | ✅ 已讲 |
| `UProperty` | 属性的描述 | ✅ 已讲 |
| `UField` | 字段系统基类 | ✅ 已讲 |

### ② AActor 派生子类（场景对象）

| 类 | 干嘛的 | 状态 |
|------|--------|:---:|
| `APawn` | 能被控制的 Actor | ✅ 已讲（06） |
| `ACharacter` | 人形角色（内置移动） | ⏳ 未深入 |
| `AController` | 控制器（大脑） | ⏳ 未深入 |
| `APlayerController` | 玩家控制器 | ⏳ 未深入 |
| `AAIController` | AI 控制器 | ⏳ 未深入 |
| `AVolume` | 体积（触发器/区域） | ⏳ 未讲 |
| `ALight` | 光源 | ⏳ 未讲 |
| `AGameModeBase` | 游戏规则 | ✅ 已讲（08） |
| `AGameStateBase` | 同步游戏状态 | ⏳ 未深入 |
| `APlayerState` | 玩家状态（多人） | ⏳ 未深入 |

### ③ UActorComponent 派生子类（组件）

| 类 | 干嘛的 | 状态 |
|------|--------|:---:|
| `USceneComponent` | 有位置的组件 | ✅ 已讲（07） |
| `UPrimitiveComponent` | 可渲染 + 碰撞 | ✅ 已讲（07） |
| `UStaticMeshComponent` | 静态模型 | ✅ 已讲（07） |
| `USkeletalMeshComponent` | 动画模型 | ✅ 已讲（07） |
| `UAbilitySystemComponent` | 技能（GAS） | ✅ 已讲（07） |

### ④ 全局/框架类

| 类 | 干嘛的 | 状态 |
|------|--------|:---:|
| `UGameInstance` | 全局实例（跨关卡） | ✅ 已讲（08） |
| `UWorld` | 关卡世界 | ✅ 已讲（09） |
| `ULevel` | 关卡 | ✅ 已讲（09） |

### ⑤ 未深入的类（可继续学）

| 类 | 干嘛的 | 建议 |
|------|--------|------|
| `ACharacter` | 人形角色 | 写玩家/敌人天天用，**建议深入** |
| `AController` 家族 | 玩家输入 / AI 控制 | **建议深入** |
| `AGameStateBase` / `APlayerState` | 多人同步状态 | 学联机再学 |
| `AVolume` / `ALight` | 场景特殊类 | 简单，了解即可 |

**总结**：已掌握核心骨架（反射 + 游戏框架 + 组件系统），待深入的是 `ACharacter`、`AController` 家族、多人框架。

---

## 一.5、AActor 常用派生类清单（写游戏选类速查）

> 这是 AActor 派生的**最常用**类，按用途分组。写游戏时"该继承哪个"直接查这张表。

### ① 可控制 / 可移动类

| 类 | 干什么 | 什么时候用 |
|------|--------|-----------|
| `APawn` | 能被控制的 Actor | 任何"可操控的角色"基类 |
| `ACharacter` | 人形角色（内置移动/碰撞） | **玩家角色、NPC、敌人**（天天用） |
| `AController` | 大脑，控制 Pawn | 玩家控制 / AI 控制 |
| `APlayerController` | 玩家控制器 | 处理玩家输入 |
| `AAIController` | AI 控制器 | AI 行为 |

### ② 游戏规则 / 框架类

| 类 | 干什么 | 什么时候用 |
|------|--------|-----------|
| `AGameModeBase` | 游戏规则（谁生成、胜负） | **每个游戏模式的核心** |
| `AGameStateBase` | 同步给所有客户端的游戏状态 | 多人游戏 |
| `APlayerState` | 单个玩家状态（分数/名字） | 多人游戏 |

### ③ 场景 / 放置类

| 类 | 干什么 | 什么时候用 |
|------|--------|-----------|
| `AActor` | 通用场景对象 | **任何能放场景的东西**基类 |
| `ATriggerVolume` | 触发器（进入就触发事件） | 关卡事件（玩家走进某区域） |
| `ALight` | 光源 | 场景照明 |
| `ACameraActor` | 摄像机 | 过场/固定视角 |

### ④ 特效 / 表现类

| 类 | 干什么 | 什么时候用 |
|------|--------|-----------|
| `ANiagaraActor` | Niagara 特效 | 现代粒子（爆炸/火焰） |
| `APostProcessVolume` | 后期处理（滤镜/色调） | 全屏特效 |
| `AAmbientSound` | 环境音效 | 场景声音 |

### 最最常用的（重点记）

| 优先级 | 类 | 用途 |
|:---:|------|------|
| ⭐⭐⭐ | `ACharacter` | **玩家/NPC/敌人**（写游戏天天用） |
| ⭐⭐⭐ | `APlayerController` | 玩家输入控制 |
| ⭐⭐⭐ | `AGameModeBase` | 游戏规则 |
| ⭐⭐ | `AActor` | 通用场景对象基类 |
| ⭐⭐ | `ATriggerVolume` | 关卡触发器 |
| ⭐⭐ | `APawn` | 可控制角色基类 |
| ⭐⭐ | `AAIController` | AI 控制 |

**一句话记法**：
```
写玩家/敌人   → ACharacter
写输入        → APlayerController
写游戏规则    → AGameModeBase
写场景触发    → ATriggerVolume
写通用场景物  → AActor
写 AI         → AAIController + APawn
```

> **实战规律**：写游戏时，**90% 的 Actor 继承 `ACharacter`（角色）或直接 `AActor`（场景物）**，然后 `AGameModeBase` 管规则、`APlayerController` 管输入。

---

## 二、一条链，三个关键层（重点理解）

UE 对象继承的核心就三层：

### 第一层：UObject —— 一切的根本

```
UObject
  ├── 有名字、有类型、有反射
  ├── 能被 GC 管、能序列化、能被蓝图用
  └── 但！没有"位置"，不能放在场景里
```

**UObject = 一个"被 UE 认识的对象"**，但没有位置、没有渲染、不能出现在游戏世界里。它是最基础的"有名字有类型的东西"。

**具体场景**：技能、存档数据、游戏规则这类**不需要在场景里有位置**的东西，直接继承 UObject：

```cpp
UCLASS()
class UInventoryData : public UObject {   // 背包数据，不需要位置
    GENERATED_BODY()
public:
    UPROPERTY() TArray<FItem> Items;
};
```

### 第二层：AActor —— 能在场景里存在的东西

```
AActor（继承 UObject）
  ├── 有了"世界位置"（能放在场景里）
  ├── 有生命周期（Spawn/BeginPlay/Destroy）
  ├── 能被放置到关卡、能渲染
  └── 但！功能靠组件，自己不直接干活
```

**AActor = "场景里的一个东西"**（一棵树、一扇门、一个触发器）。它有位置、有生命周期，但**具体功能靠挂组件**。

**具体场景**：场景里能放的物体继承 AActor：

```cpp
UCLASS()
class AChest : public AActor {   // 宝箱，能放到场景里
    GENERATED_BODY()
public:
    // 打开宝箱的逻辑
    void Open() { /* 掉出奖励 */ }
};
```

### 第三层：UActorComponent —— 挂在 Actor 上的"零件"

```
UActorComponent（继承 UObject）
  ├── 不自己存在，挂在一个 Actor 上
  ├── 给 Actor 提供一项能力
  └── 比如：移动组件、渲染组件、技能组件
```

**UActorComponent = "一个功能零件"**，挂在 Actor 上给它加能力。**Actor 是主体，组件是零件**。

**具体场景**：给角色挂技能组件：

```cpp
UCLASS()
class UAbilityComponent : public UActorComponent {   // 技能组件
    GENERATED_BODY()
public:
    void CastAbility() { /* 施放技能 */ }
};
```

---

## 三、AActor vs UActorComponent —— 最核心的区别

这是新手最容易混的。记住：**Actor 是"东西"，Component 是"能力"**。

| | AActor | UActorComponent |
|---|---|---|
| 是什么 | 场景里的**一个东西** | 挂在上面的**一个零件** |
| 能单独存在 | ✅（能放场景） | ❌（必须挂 Actor） |
| 例子 | 角色、宝箱、灯、敌人 | 移动组件、技能组件、碰撞组件 |
| 数量 | 一个"东西"一个 Actor | 一个 Actor 可挂多个组件 |

**具体场景：理解"角色 = Actor + 多个组件"**

```
一个 ACharacter（Actor）
  ├── 挂：USkeletalMeshComponent（显示模型）
  ├── 挂：UCharacterMovementComponent（移动）
  ├── 挂：UAbilityComponent（技能）
  └── 挂：UCapsuleComponent（碰撞）
```

```cpp
// 在 Actor 里创建组件
UCLASS()
class AMyCharacter : public ACharacter {
    GENERATED_BODY()
public:
    UPROPERTY()
    UAbilityComponent* AbilityComp;   // 挂一个技能组件

    AMyCharacter() {
        // 创建组件（用 CreateDefaultSubobject）
        AbilityComp = CreateDefaultSubobject<UAbilityComponent>(TEXT("AbilityComp"));
    }
};
```

---

## 四、关键子类逐个讲（配具体场景）

### 4.1 ACharacter —— 人形角色

`ACharacter`（继承 APawn）内置了**移动**（CharacterMovement）和**碰撞**（Capsule）组件，专门做"人形可移动角色"。

```cpp
UCLASS()
class APlayerCharacter : public ACharacter {
    GENERATED_BODY()
public:
    virtual void Jump() override;   // 自带跳跃能力
    void MoveForward(float Val);    // 用 ACharacter 的移动组件
};
```

**场景**：玩家控制的角色、NPC、敌人（能走能跳的都继承 ACharacter）。

### 4.2 AController —— 控制器（操作 Pawn）

`AController` 不自己存在，它**控制一个 Pawn**。玩家控制器（PlayerController）、AI 控制器（AIController）都继承它。

```cpp
// AI 控制器：控制一个 AI Pawn 的行动
UCLASS()
class AMonsterAIController : public AController {
    GENERATED_BODY()
public:
    virtual void Possess(APawn* InPawn) override;
};
```

**场景**：玩家的 PlayerController、AI 的 AIController——都是"谁来操控这个角色"。

### 4.3 USceneComponent 和子类 —— 有位置的组件

```
USceneComponent（有位置）
  ├── UPrimitiveComponent（可渲染）
  │   ├── UStaticMeshComponent（静态网格：房子、石头）
  │   ├── USkeletalMeshComponent（骨骼网格：会动的角色模型）
  │   └── ...
  └── ULightComponent（光源）
```

**场景**：给宝箱挂静态网格显示模型、给角色挂骨骼网格显示动画。

### 4.4 UGameInstance / UWorld / ULevel —— 游戏全局和关卡

| 类 | 作用 |
|------|------|
| `UGameInstance` | 整个游戏的全局实例（跨关卡保存的数据） |
| `UWorld` | 一个游戏世界（包含所有 Actor、关卡） |
| `ULevel` | 一个关卡（World 里的一个场景） |

**场景**：在 GameInstance 里存"跨关卡不消失的玩家数据"：

```cpp
UCLASS()
class UMyGameInstance : public UGameInstance {
    GENERATED_BODY()
public:
    int32 TotalCoins;   // 跨关卡保存的金币数
};
```

---

## 五、怎么判断"该继承哪个"（决策树）

```
你要创建的对象需要"放在场景里"吗？
├─ 需要 → 继承 AActor（能放场景、有位置）
│         └─ 是"人形可移动角色"？ → ACharacter
│         └─ 需要被控制？         → APawn
├─ 不需要，但要有位置 → 继承 USceneComponent（组件）
├─ 不需要位置，是"能力/零件" → 继承 UActorComponent（组件）
└─ 不需要位置，是"数据/纯逻辑" → 继承 UObject（技能、存档、规则）
```

**快速判断**：
- **要放场景**（宝箱、灯、触发器）→ AActor
- **给 Actor 加能力**（移动、技能、渲染）→ UActorComponent
- **纯数据/逻辑**（技能数据、存档、规则）→ UObject
- **人形可移动角色**（玩家、NPC）→ ACharacter

---

## 六、常见陷阱

**① 把"能力"做成 Actor（应该做 Component）**
```cpp
// ❌ 移动能力做成 Actor？错
UCLASS() class AMovement : public AActor { ... };
// ✅ 能力是组件
UCLASS() class UMovementComp : public UActorComponent { ... };
```

**② 把"数据"继承 AActor（不需要位置，应该 UObject）**
```cpp
// ❌ 存档数据不需要放场景
UCLASS() class ASaveData : public AActor { ... };
// ✅ 用 UObject
UCLASS() class USaveData : public UObject { ... };
```

**③ 忘了组件要挂到 Actor 上才能用**
```cpp
// ❌ 只声明组件指针，不创建不挂载
UAbilityComponent* AbilityComp;   // 空指针，没用
// ✅ 构造函数里创建
AbilityComp = CreateDefaultSubobject<UAbilityComponent>(TEXT("AbilityComp"));
```

---

## 七、总结速查

```
UObject（根）── 被 UE 认识的对象（有名字/反射/GC）
├── AActor ── 能放场景的东西（有位置/生命周期）
│   ├── APawn ── 能被控制
│   │   └── ACharacter ── 人形可移动角色（内置移动/碰撞）
│   └── AController ── 控制器
├── UActorComponent ── 挂在 Actor 上的零件（能力）
│   └── USceneComponent ── 有位置 → UPrimitiveComponent → 各种 Mesh
├── UGameInstance / UWorld / ULevel ── 全局/世界/关卡
└── 各种 UObject 子类

选型：
  放场景 → AActor
  加能力 → UActorComponent
  纯数据 → UObject
  人形角色 → ACharacter
```

**一句话**：UE 对象分层是 **UObject（被认识）→ AActor（放场景）→ UActorComponent（加能力）**。**Actor 是"东西"，Component 是"零件"**。选继承类时先问：要不要放场景？要放场景用 AActor，加能力用 Component，纯数据用 UObject。

---

## 八、下一步

理解了继承体系，下一个重点是 **Engine 模块的 AActor**——它的生命周期（BeginPlay/Tick）、组件系统（如何挂组件）、以及 GameMode/PlayerController 这套游戏框架。这是真正"写游戏逻辑"的地方。
