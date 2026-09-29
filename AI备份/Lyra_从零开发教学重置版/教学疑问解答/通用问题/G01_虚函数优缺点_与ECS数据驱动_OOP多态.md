# G01 — 通用问题：虚函数的优缺点？ECS 数据驱动 vs OOP 多态？Lyra 怎么做？

> **定位**：通用概念问题（不绑定某一课）。
>
> **一句话**：虚函数是 OOP 多态的核心，优点是"接口不变、行为可扩展"，代价是"一次间接跳转的运行时开销 + 破坏数据局部性"；ECS 数据驱动是另一种范式，主打"数据与逻辑分离、缓存友好"。**两者不是取代关系，而是各管一摊**。Lyra 主体仍是 OOP + 虚函数多态，同时在"配置/内容"层大量用**数据驱动**（DataAsset），走的是"OOP 骨架 + 数据驱动内容"的混合路线。

---

## 一、虚函数是什么（先讲清再谈优缺点）

**虚函数（virtual function）= 能在运行时根据对象的真实类型，调用到"被覆盖（override）后的版本"的函数。**

C++ 里，基类声明 `virtual`，子类 `override`。调用时通过**虚函数表（vtable）**间接跳转——编译期不知道到底调哪个版本，运行时看对象的真实类型决定。

```cpp
class AActor {
    virtual void PostInitializeComponents();   // 基类声明虚函数
};
class AMyActor : public AActor {
    void PostInitializeComponents() override;  // 子类覆盖
};

AActor* p = new AMyActor();
p->PostInitializeComponents();   // 运行时调到的是 AMyActor 的版本（多态）
```

> 我们刚学的 `PostInitializeComponents`、`GetAbilitySystemComponent` 都是虚函数——引擎调 `PostInitializeComponents()` 时，实际跑的是我们子类 override 的版本，这就是多态。

---

## 二、虚函数的优点

| 优点 | 说明 | 例子 |
|------|------|------|
| **① 接口不变、行为可扩展** | 基类定好函数签名，子类各自实现不同行为，调用方不用改 | 引擎调 `PostInitializeComponents`，每个 Actor 子类干自己的初始化 |
| **② 运行时可替换（晚绑定）** | 用基类指针/引用调用，实际行为由真实类型决定 | `AActor*` 指向任何 Actor，调 `Tick` 都跑到对的版本 |
| **③ 支撑框架设计** | 引擎/框架"反转调用方向"：框架调虚函数，你填实现 | GAS 里引擎调 `NotifyAbilityActivated`，你 override 写自己的逻辑 |

**第③点是虚函数最大的价值**：它让"父类/框架决定什么时候调、子类决定调的时候干什么"。这就是为什么 UE 有那么多 `virtual ... override`——引擎搭骨架，你在骨架上填肉。

---

## 三、虚函数的缺点

| 缺点 | 说明 |
|------|------|
| **① 运行时开销** | 每次调用要查 vtable 间接跳转（一次指针解引用），比直接调用慢。热路径（每帧调上万次）累积起来可观 |
| **② 破坏内联** | 编译器通常无法内联虚函数（因为运行时才知道调哪个版本），失去内联优化 |
| **③ 破坏数据局部性（cache 不友好）** | vtable 指针 + 间接跳转，CPU 缓存预测变差——这正是 ECS 批评 OOP 的核心点 |
| **④ 对象更重** | 每个含虚函数的对象多一个 vptr（指向 vtable 的指针），占内存 |
| **⑤ 调用链难追踪** | 多态让"到底调了哪个函数"在代码里看不出来，调试/阅读要跳到子类 |

> ECS 阵营主要拿**①③**攻击 OOP："每帧大量虚函数调用 = 一堆间接跳转 = CPU 缓存打不中 = 慢"。

---

## 四、ECS 数据驱动 vs OOP 多态（澄清"现在都用 ECS"这个说法）

先纠正一个常见误解：**"现在都用 ECS 了，OOP 多态过时了"——这是不准确的。**

| 维度 | OOP（虚函数多态） | ECS（数据驱动） |
|------|------------------|-----------------|
| **核心思想** | 对象 = 数据 + 行为，封装在一起，用继承/多态组织 | 数据（Component）与逻辑（System）彻底分离 |
| **怎么扩展** | 继承基类、override 虚函数 | 组合不同的 Component，System 处理数据 |
| **优势** | 表达力强、封装好、适合"复杂对象行为" | 缓存友好、易并行、适合"海量相似实体的高频处理" |
| **劣势** | 缓存不友好、难并行（见第三节缺点） | 表达复杂行为时啰嗦、调试难、"万物皆组件"易失控 |
| **典型场景** | 角色、技能、UI、游戏逻辑（行为复杂、数量不多） | 子弹、粒子、大规模单位（数量巨大、逻辑简单、高频） |

**关键结论**：**不是"谁取代谁"，而是"各管一摊"**。
- 现代引擎（含 UE）主流仍是 **OOP 打底**，因为游戏逻辑天然是"复杂对象行为"，OOP 表达力最强。
- ECS 用在"**需要极致性能的特定子系统**"（如大规模模拟、高性能计算）。UE 自己也提供了 **MassEntity**（ECS 框架）给这类场景。
- 很多所谓"数据驱动"其实是**用数据配置替代硬编码**（如 DataAsset、蓝图），这跟"要不要用 ECS"是两回事——**数据驱动可以和 OOP 共存**。

> 一句话：**"数据驱动"是思想（用数据配置行为），ECS 是它的一种激进实现。OOP 多态没过时，只是大家会在性能热点上改用数据导向/ECS。**

---

## 五、Lyra 怎么做（真实源码，都实际打开核对过）

**Lyra 主体是 OOP + 虚函数多态，同时在"配置/内容"层大量用数据驱动（DataAsset）。** 是典型的"OOP 骨架 + 数据驱动内容"混合体。

### 例子 ①：虚函数多态——`ULyraAbilitySystemComponent` override 引擎虚函数

`Source/LyraGame/AbilitySystem/LyraAbilitySystemComponent.h` 里，Lyra 的 ASC override 了引擎一大堆虚函数：

```37:40:Source/LyraGame/AbilitySystem/LyraAbilitySystemComponent.h
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ...
	UE_API virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
```

还有 `NotifyAbilityActivated` / `NotifyAbilityFailed` / `NotifyAbilityEnded` / `ApplyAbilityBlockAndCancelTags` 等（第 80-84 行）——**全是 override 引擎基类的虚函数**。

**这就是第二节优点③"框架调虚函数、你填实现"的活教材**：GAS 引擎在合适时机调这些虚函数，Lyra override 它们插入自己的逻辑（如记录、标签处理）。**我们刚学的 `GetAbilitySystemComponent() override`、`PostInitializeComponents() override` 也是同一套思路**。

### 例子 ②：数据驱动——`ULyraExperienceDefinition`（DataAsset）

`Source/LyraGame/GameModes/LyraExperienceDefinition.h`：

```15:16:Source/LyraGame/GameModes/LyraExperienceDefinition.h
UCLASS(BlueprintType, Const)
class ULyraExperienceDefinition : public UPrimaryDataAsset
```

它继承 `UPrimaryDataAsset`（数据资产），字段全是 `UPROPERTY(EditDefaultsOnly)`（可在编辑器里配置的数据）：

```37:51:Source/LyraGame/GameModes/LyraExperienceDefinition.h
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TArray<FString> GameFeaturesToEnable;          // 要启用哪些 GameFeature 插件

	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TObjectPtr<const ULyraPawnData> DefaultPawnData;  // 默认 Pawn 数据

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;   // 要执行哪些动作
```

**这是"数据驱动"的典型**：一个"游戏体验"（如某个游戏模式）不是写死在 C++ 里，而是**在编辑器里配这个 DataAsset**——配要开哪些插件、用哪个 Pawn、跑哪些 Action。**改玩法不改代码，改数据就行**。

> 注意：这里的数据驱动**没有用 ECS**，也没抛弃 OOP——`ULyraExperienceDefinition` 本身就是个 UObject（OOP 的类），只是它"装的是数据"。这正是第四节说的"**数据驱动可以和 OOP 共存**"。

### Lyra 的取舍总结

| 层 | Lyra 用什么 | 为什么 |
|----|-----------|--------|
| **游戏逻辑骨架**（角色/ASC/技能/角色） | **OOP + 虚函数多态** | 行为复杂、数量不多，OOP 表达力最强（见例子①） |
| **配置/内容**（体验/玩法/Pawn 数据） | **数据驱动（DataAsset）** | 让策划不改代码就能调配玩法（见例子②） |
| **高性能子系统** | 按需（UE 提供 MassEntity ECS，Lyra 主体未重度使用） | 需要极致性能时才上 |

---

## 六、和我们刚学的东西怎么联系

- 我们写的 `ATenkaichiCharacterWithAbilities`：`PostInitializeComponents() override`、`GetAbilitySystemComponent() override`——**就是虚函数多态**（例子①那套）。
- 我们前面建的 `UTenkaichiHealthSet`（属性集）、将来的 `PawnData`——**就是数据驱动**（例子②那套，用 DataAsset 装数据）。

**所以你现在学的每一步，恰好都踩在 Lyra（乃至整个 UE）的两大支柱上：OOP 多态搭骨架 + 数据驱动配内容。**

---

## 七、一句话结论

**虚函数 = 运行时多态，优点是"接口不变、行为可扩展、支撑框架设计"，缺点是"间接跳转开销 + 破坏缓存局部性"。ECS 数据驱动不是取代 OOP，而是各管一摊（ECS 管海量高频实体，OOP 管复杂行为）。Lyra 走"OOP 骨架（虚函数多态，如 ASC override）+ 数据驱动内容（DataAsset，如 ExperienceDefinition）"的混合路线——我们学的角色类 override 和属性集/PawnData，正好分别对应这两根支柱。**

---

## 八、下一步

回到主线第 5 步（授予属性集 / 初始化血量），继续按 Lyra 真实代码一比一推进。
