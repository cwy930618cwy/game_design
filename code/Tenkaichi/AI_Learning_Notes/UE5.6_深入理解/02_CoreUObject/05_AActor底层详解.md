# 05 — AActor 底层详解（真实源码实拍）

> **定位**：AActor 是"能放在场景里的对象"的基类。这篇直接看 `Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h` 的真实源码，拆解它到底有什么。
>
> **一句话**：AActor 继承 UObject，额外加了三样东西——**能放场景**（有 Transform）、**能挂组件**（OwnedComponents）、**有生命周期**（BeginPlay/Tick/Destroy）。所有能放在世界里的东西都从它派生子类。
>
> **文件**：`Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h`（253 KB，非常大的头文件）

---

## 一、AActor 的真实声明（从源码抄来的）

```cpp
// Actor.h 第 252 行（真实源码）
UCLASS(BlueprintType, Blueprintable, MigratingAsset, config=Engine,
       meta=(ShortTooltip="An Actor is an object that can be placed or spawned in the world."), MinimalAPI)
class AActor : public UObject
{
    GENERATED_BODY()
public:
    /** Default constructor for AActor */
    ENGINE_API AActor();

    /** Primary Actor tick function */
    UPROPERTY(EditDefaultsOnly, Category=Tick)
    struct FActorTickFunction PrimaryActorTick;
    ...
};
```

**关键点**：
1. `AActor : public UObject` —— **继承 UObject**（所以有反射/GC/序列化）
2. `GENERATED_BODY()` —— 生成反射代码（你刚学的）
3. 带一堆 UPROPERTY/UFUNCTION/委托（网络、碰撞、输入）

---

## 二、AActor 比 UObject 多了什么（核心区别）

UObject 只是"被 UE 认识的对象"，AActor 加了三样东西：

### ① 能放场景（有位置 Transform）
Actor 能在世界里有个位置（但它不自带 Transform 成员，通过 RootComponent 获取，见下）。

### ② 能挂组件（OwnedComponents）
Actor 可以挂一堆组件（模型、移动、碰撞），这是 Actor 最核心的"组装"能力。

```cpp
// Actor.h 里：Actor 持有的组件（真实源码，简化）
UPROPERTY()
TArray<TObjectPtr<UActorComponent>> OwnedComponents;   // 挂着的所有组件
```

### ③ 有生命周期（BeginPlay / Tick / Destroy）
Actor 有完整的"出生 → 运行 → 销毁"流程（见第五节）。

### ④ 网络相关（一堆复制标志）
```cpp
// Actor.h 真实源码：网络复制标志
uint8 bNetTemporary:1;    // 临时网络 actor
uint8 bNetStartup:1;      // 从关卡加载的 actor
UPROPERTY() uint8 bOnlyRelevantToOwner:1;  // 只对 owner 相关
UPROPERTY() uint8 bAlwaysRelevant:1;       // 总是相关
```

---

## 三、Actor 的真实生命周期（源码注释里写的）

Actor.h 开头有**一大段官方生命周期注释**（第 220-250 行），我翻译成容易懂的流程：

```
Actor 初始化顺序（真实源码注释）：
1. UObject::PostLoad          — 关卡里放置的 actor 加载时
2. UActorComponent::OnComponentCreated — 原生组件创建
3. AActor::PreRegisterAllComponents    — 组件注册前
4. UActorComponent::RegisterComponent  — 组件注册（建立物理/视觉）
5. AActor::PostRegisterAllComponents   — 组件注册后
6. AActor::PostActorCreated           — actor 创建后
7. AActor::OnConstruction             — 构造（蓝图 Construction Script）
8. AActor::PreInitializeComponents    — 组件初始化前
9. UActorComponent::InitializeComponent — 组件初始化
10. AActor::PostInitializeComponents  — 组件初始化后
11. AActor::BeginPlay                 — ★ 游戏开始（你重写这个）
12. AActor::Tick                     — ★ 每帧调用（你重写这个）
```

**你日常最关心的是后两个**：
- `BeginPlay()` —— 游戏开始时调用一次（初始化逻辑）
- `Tick()` —— 每帧调用（持续逻辑）

```cpp
// 你写游戏逻辑时重写的两个函数
UCLASS()
class AMyActor : public AActor {
    GENERATED_BODY()
protected:
    virtual void BeginPlay() override {
        Super::BeginPlay();
        // 游戏开始时执行（初始化血量、找目标等）
    }
    virtual void Tick(float DeltaTime) override {
        Super::Tick(DeltaTime);
        // 每帧执行（移动、检测、更新）
    }
};
```

---

## 四、Actor 最重要的能力：组件系统

Actor 是"主体"，组件是"零件"。Actor 通过挂组件获得能力。

### 4.1 挂组件（CreateDefaultSubobject）

```cpp
UCLASS()
class AMyCharacter : public ACharacter {
    GENERATED_BODY()
public:
    UPROPERTY()
    UStaticMeshComponent* MeshComp;   // 模型组件

    AMyCharacter() {
        // 构造函数里创建组件（真实场景：给角色挂模型）
        MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
        RootComponent = MeshComp;     // 设为根组件（决定 Actor 的位置）
    }
};
```

### 4.2 Actor 和组件的分工（回顾 02）

```
AActor（东西，能放场景）
  └─ 挂 UActorComponent（零件）
       ├─ USceneComponent（有位置）
       │    └─ UPrimitiveComponent（可渲染）
       │         └─ UStaticMeshComponent（显示模型）
       └─ UCharacterMovementComponent（移动能力）
```

---

## 五、Actor 的真实成员（从源码看它有什么字段）

我看到的真实 UPROPERTY（源码片段）：

```cpp
// Actor.h 真实源码（部分成员）
UPROPERTY(EditDefaultsOnly, Category=Tick)
struct FActorTickFunction PrimaryActorTick;   // 每帧 Tick 的配置

// 组件相关
UPROPERTY()
TArray<TObjectPtr<UActorComponent>> OwnedComponents;  // 拥有的组件

// 网络标志
uint8 bNetTemporary:1;
uint8 bNetStartup:1;
UPROPERTY() uint8 bOnlyRelevantToOwner:1;
UPROPERTY() uint8 bAlwaysRelevant:1;

// 生命周期委托（你之前学的动态多播委托）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActorDestroyedSignature, AActor*, OnDestroyed, AActor*, DestroyedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FActorEndPlaySignature, AActor*, OnEndPlay, AActor*, Actor, EEndPlayReason::Type, EndPlayReason);
```

**所以 AActor 真实内容**：
- 组件数组（OwnedComponents）
- Tick 配置（PrimaryActorTick）
- 网络复制标志
- 生命周期委托（OnDestroyed / OnEndPlay）

---

## 六、常用成员函数（你写游戏会用到）

| 函数 | 作用 |
|------|------|
| `SpawnActor<APawn>(Class, Loc, Rot)` | 生成 Actor（World 的静态函数） |
| `Actor->Destroy()` | 销毁 Actor |
| `GetActorLocation()` / `SetActorLocation()` | 位置 |
| `GetActorRotation()` / `SetActorRotation()` | 旋转 |
| `GetRootComponent()` | 根组件 |
| `GetComponents()` | 所有组件 |
| `FindComponentByClass<UXX>()` | 找某类组件 |
| `IsA<UXX>()` | 是不是某类（反射，已学） |
| `AddActorLocalOffset()` | 相对移动 |
| `GetWorld()` | 所在世界 |

```cpp
// 具体场景：生成一个敌人，并设置位置
FVector SpawnLoc(100.f, 0.f, 0.f);
FRotator SpawnRot(0.f, 0.f, 0.f);
AEnemy* Enemy = GetWorld()->SpawnActor<AEnemy>(AEnemy::StaticClass(), SpawnLoc, SpawnRot);
```

---

## 七、AActor 底层总结

```
UObject（被 UE 认识：反射/GC/序列化）
  └─ AActor（能放场景 + 挂组件 + 生命周期 + 网络）
       ├─ 能放世界（有位置）
       ├─ 挂组件（OwnedComponents 数组）
       ├─ 生命周期（BeginPlay/Tick/Destroy）
       ├─ 网络复制（bReplicated 等标志）
       └─ 生命周期委托（OnDestroyed/OnEndPlay）

你重写的两个函数：
  BeginPlay()   — 游戏开始（初始化）
  Tick(dt)      — 每帧（持续逻辑）

组件系统：Actor = 东西，Component = 零件
```

**一句话**：AActor 在 UObject 基础上，加了"**能放场景、能挂组件、有生命周期、能网络复制**"。写游戏逻辑你重写 `BeginPlay()`（开始）和 `Tick()`（每帧），用 `SpawnActor` 生成、`Destroy()` 销毁，通过挂组件组装能力。

---

## 八、下一步

AActor 是 Engine 模块的入口。下一篇可以深入 **Actor 组件系统**（USceneComponent / UActorComponent 如何工作）或 **GameMode/PlayerController 游戏框架**（谁来管规则、谁来控制角色）。
