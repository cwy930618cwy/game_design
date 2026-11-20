# 07 — Modular Gameplay 框架在 Lyra 中的应用

> **定位**：讲清 Lyra 里那些 `ModularXxx` 派生类（`AModularCharacter` / `AModularPawn` / `AModularActor`）到底是怎么工作的，以及 Lyra 如何用这套框架把角色、装备、物品"拼"起来。
>
> **和 `UE5.6_源码分析/.../03_ModularGameplay组件化.md` 的区别**：
> - 那份讲**引擎插件本身**（ModularGameplay 插件有哪些类、接口怎么定义）
> - 本篇讲 **Lyra 项目怎么用这套框架**（派生体系、组合方式、初始化流程）
>
> **一句话**：Modular Gameplay 是 Lyra "组合优于继承" 的落地机制——理解它，才能看懂 Lyra 为什么满屏都是 Component 和 DataAsset。

---

## 一、为什么 Lyra 要用 Modular 框架

传统 UE 项目用**继承**堆功能，越往后越痛苦：

```cpp
// 传统：每个角色都继承 ACharacter，功能越加越多 → God Class
class AMyHero : public ACharacter { /* 500 行 */ };
class AMyEnemy : public ACharacter { /* 400 行，还大量重复 */ };
```

Lyra 改用**组合**：角色本身是个"空壳"，功能全由**组件拼装**：

```cpp
ALyraCharacter (几乎空的)
  ├── ULyraPawnExtensionComponent  ← 协调初始化
  ├── ULyraHeroComponent           ← 输入 + 相机
  ├── ULyraHealthComponent         ← 生命值
  └── ULyraCameraComponent         ← 相机
```

好处：职责分离、灵活组合、易测试、避免上帝类。

---

## 二、Modular 派生体系（核心类）

Lyra 的"空壳"基类全部来自 ModularGameplay 插件，分三个层级：

| Lyra 用到的基类 | 父类 | 用途 |
|----------------|------|------|
| `AModularActor` | `AActor` | 最底层：任何可放场景的东西 |
| `AModularPawn` | `APawn` | 可被控制的实体（载具、简单单位） |
| `AModularCharacter` | `ACharacter` | 带移动组件的角色（玩家/敌人） |

它们的共同点：**自己几乎不加功能，只提供"组件挂载 + 动态扩展"的能力**。

```cpp
// LyraCharacter.h —— 真实源码
UCLASS()
class ALyraCharacter : public AModularCharacter,          // ← 关键：继承空壳
    public IAbilitySystemInterface,                       // 技能系统接口
    public IGameplayCueInterface,                         // 技能特效接口
    public IGameplayTagAssetInterface,                    // Tag 接口
    public ILyraTeamAgentInterface                        // 队伍接口
{
    GENERATED_BODY()
public:
    // 只挂 3 个核心组件，其余功能全靠业务组件拼
    UPROPERTY() TObjectPtr<ULyraPawnExtensionComponent> PawnExtComponent;
    UPROPERTY() TObjectPtr<ULyraHealthComponent> HealthComponent;
    UPROPERTY() TObjectPtr<ULyraCameraComponent> CameraComponent;
};
```

> ⚠️ 注意：Lyra 的角色类**除了接口声明，几乎不写逻辑**。这就是"空壳"设计的体现。

---

## 三、组合方式：数据驱动拼装

Lyra 不是硬编码组件，而是通过 **DataAsset 配置要拼哪些组件**。

### 3.1 UPawnDataAsset（角色配置单）

```cpp
// BP_LyraPawnData（蓝图数据资产）
UPROPERTY()
TArray<FModularPawnComponentEntry> Components;   // 要添加哪些组件
UPROPERTY()
TArray<UAbilitySet*> AbilitySets;                // 授予哪些技能
UPROPERTY()
ULyraInputConfig* InputConfig;                   // 用什么输入配置
```

策划改这个资产就能换角色的能力，**不用动一行 C++**。

### 3.2 拼装流程

```
1. 关卡 Spawn 一个 AModularCharacter（空壳）
2. PawnExtensionComponent 读取 PawnDataAsset
3. 遍历 Components 数组
4. 动态创建并注册每个组件
5. 每个组件收到 OnModularPawnRegistered 回调
6. 组件自行初始化（绑输入、订阅消息）
```

---

## 四、组件通信：不互相引用，靠消息

模块化最怕"组件之间互相强引用"（又变成隐式耦合）。Lyra 的做法是**组件间只靠消息/事件通信**：

```cpp
// HeroComponent 需要生命值？不直接找 HealthComponent
// 而是广播一条消息
MessageSystem->BroadcastMessage(MSGKEY("Pawn.Ready"), FPawnReadyMessage{this});

// HealthComponent 监听后自行处理
void ULyraHealthComponent::OnPawnReady(const FPawnReadyMessage& Msg)
{
    // 初始化生命值...
}
```

这样组件完全解耦，删掉任何一个都不影响其他组件编译。

---

## 五、初始化状态机（InitState）⭐

这是 Lyra Modular 框架最容易踩坑的地方。组件初始化有严格的**四阶段顺序**：

```cpp
enum class ELyraPawnInitState
{
    Spawned,           // 刚生成，啥都没有
    DataAvailable,     // PawnData 已加载（组件列表已知）
    DataInitialized,   // 各组件数据初始化完成
    GameplayReady      // 全部就绪，可以开始玩
};
```

> ⚠️ **常见坑**：在 `BeginPlay` 里访问其他组件经常拿到空指针——因为此时对方可能还没初始化完。**正确做法**是订阅 `PawnExtensionComponent` 的状态变化，在 `GameplayReady` 之后再干活。

---

## 六、Modular 思想不止于角色

"组合优于继承"贯穿 Lyra 多个系统，不只是角色：

| 系统 | Modular 体现 |
|------|-------------|
| **角色** | `AModularCharacter` + 业务组件拼装 |
| **装备** | `EquipmentDefinition` → 生成 Actor + 授予 AbilitySet |
| **物品** | `InventoryItemDefinition` + Fragment 组合（DefaultToInstanced） |
| **武器** | `WeaponDefinition` 组合射击/瞄准能力 |
| **GameFeature** | 整个玩法以插件形式按需加载（见 `02_GameFeature详解`） |

> 看到 `XxxDefinition`（DataAsset）+ `XxxInstance`（运行时实例）的模式，就是 Lyra 在用 Modular 思想做数据与行为组合。

---

## 七、目录结构

```
ModularGameplay/                      （引擎插件，见 UE5.6_源码分析）
├── Public/
│   ├── ModularPawn.h                 ← 模块化 Pawn 基类
│   ├── ModularCharacter.h            ← 模块化角色基类
│   └── ModularPawnInterface.h        ← 组件识别接口
└── ...

LyraGame/Source/LyraGame/Character/   （Lyra 项目里的用法）
├── LyraCharacter.h/.cpp              ← 继承 AModularCharacter 的空壳
├── LyraPawnExtensionComponent.*      ← 初始化协调中枢
├── LyraHeroComponent.*               ← 输入/相机
├── LyraHealthComponent.*             ← 生命值
└── LyraCameraComponent.*             ← 相机
```

---

## 八、与传统做法对比

| 方面 | 传统继承 | Lyra Modular |
|------|---------|--------------|
| 代码复用 | 难（重复多） | 易（组件复用） |
| 添加新功能 | 改角色类 | 加新组件 / 改 DataAsset |
| 测试 | 难（耦合重） | 易（组件独立） |
| 网络同步 | 复杂 | 各组件各自处理 |
| 策划可调 | 否（硬编码） | 是（DataAsset 配置） |
| 学习曲线 | 低 | 中高（需理解架构） |

---

## 九、学习建议

1. **先看 `ALyraCharacter`** — 体会"空壳 + 接口"的设计
2. **再看 `LyraPawnExtensionComponent`** — 理解初始化协调与 InitState
3. **跟踪一个完整角色组装** — 从 PawnDataAsset 到组件全部就绪
4. **动手实践** — 创建一个自定义组件，通过 DataAsset 挂到角色上
5. **对照源码** — 引擎层看 `UE5.6_源码分析/.../03_ModularGameplay组件化.md`

---

## 十、下一步

- [02_Character角色系统详解](./06_Lyra功能总览/02_Character角色系统详解.md) — 角色系统全貌
- [02_GameFeature详解](./02_GameFeature详解.md) — 插件化加载
- [03_Experience详解](./03_Experience详解.md) — PawnData 从哪来
- [UE5.6_源码分析/.../03_ModularGameplay组件化](../UE5.6_源码分析/02_Runtime插件详解/03_ModularGameplay组件化.md) — 引擎插件源码
