# 19 — `FActorInitStateChangedParams` 详解：状态系统的"通知单"

> **定位**：看 `LyraPawnExtensionComponent.h` 第 18 行的前向声明 `struct FActorInitStateChangedParams;`——它后面跟的 `OnActorInitStateChanged(const FActorInitStateChangedParams& Params)`（第 42 行）会用到。这个结构体是什么、里面的信息哪来的、Lyra 代码里怎么读它？这篇讲透。
>
> **衔接**：前面第 01/04/07 篇讲了 ModularGameplay 的初始化状态链和 `UGameFrameworkComponentManager`（大管家）。这篇是补上"状态变化广播时，到底传的是什么"——那"一嗓子通知"的内容格式。

---

## 一、一句话看懂它

> **`FActorInitStateChangedParams`（复数 `Params`）= ModularGameplay 状态系统里，描述"某个 Actor 上某个 Feature（特性）的状态发生了什么变化"的一份参数包（通知单）。**

当任何组件把初始化状态往前推一步（比如从 `DataAvailable` → `DataInitialized`），**大管家 `UGameFrameworkComponentManager` 就会组一份 `FActorInitStateChangedParams`，广播给所有订阅者**。谁收到它，就知道"哪个 Actor 的哪个特性、到哪个状态了"。

**它就是"值班室广播"里的那句话**："角色 A 的『PawnExtension』特性已经进入 `DataInitialized` 了！"

---

## 二、它从哪来？——不是 Lyra 自创，是引擎 ModularGameplay 的公开类型

- 定义位置：**引擎** `Plugins/Runtime/ModularGameplay/.../GameFrameworkInitStateInterface.h`（UE5 官方插件）。
- 它服务于 `IGameFrameworkInitStateInterface`（状态接口）那套机制（第 06 篇讲过：实现这个接口 = 承诺参与状态管理）。

> 所以 `LyraPawnExtensionComponent.h` 里对它**前向声明**即可——它只以"引用参数"出现（第 42 行 `const FActorInitStateChangedParams& Params`），属于"指针/引用 → 前向声明"那一类（见第 17 篇）。

---

## 三、它里面装了什么？（核心字段）

结构体很薄，就是一组"信息快照"。核心字段（结合 Lyra 源码实际读取的字段列出）：

| 字段 | 含义 | Lyra 源码里怎么用 |
|---|---|---|
| `AActor* OwningActor` | **谁的状态变了**（哪个 Actor） | —— |
| `FName FeatureName` | **哪个 Feature 变了**（特性的名字，如 `NAME_ActorFeatureName` = "PawnExtension"） | `Params.FeatureName != NAME_ActorFeatureName` |
| `UObject* ImplementingObject` | 实现该 Feature 的对象（通常是那个组件自身） | —— |
| `FGameplayTag FeatureState` | **变成了哪个状态**（`InitState_Spawned` / `DataAvailable` / `DataInitialized` / `GameplayReady`） | `Params.FeatureState == InitState_DataAvailable` |

> **类比**：它就是一张**快递面单**——"发件人（ImplementingObject）从哪发出（OwningActor 的哪个 Feature）、货物现在到哪个中转站了（FeatureState）"。收件人拿到面单就能判断"我要不要准备收货"。

---

## 四、它是怎么流转的？（谁填、谁收）

### 全景图：一嗓子广播怎么传遍所有组件

```
                          【发出端：某个想推进的组件，如 LyraHeroComponent】
                               │
                               ▼  TryToChangeInitState(InitState_DataInitialized)
    ┌──────────────────────────────────────────────────────────────┐
    │             UGameFrameworkComponentManager（大管家）            │
    │  1. 更新该 Feature 的状态（DataAvailable → DataInitialized）    │
    │  2. 组好一张"通知单" FActorInitStateChangedParams：            │
    │       OwningActor    = 角色 A                                  │
    │       FeatureName    = "PawnExtension"                         │
    │       ImplementingObject = 总指挥组件本身                       │
    │       FeatureState   = InitState_DataInitialized               │
    │  3. 把它塞进队列，依次广播给所有订阅者                          │
    └──────────────────────────────────────────────────────────────┘
                               │
              ┌────────────────┼─────────────────────┐
              ▼                ▼                     ▼
     【订阅者1】总指挥      【订阅者2】Hero      【订阅者3】能力系统…
     读 Params 判断        读 Params 判断          读 Params 判断
     不是自己→继续等        是"PawnExtension"       等 DataInitialized
                          + DataInitialized          到了→初始化
      │                    → CheckDefault
      │                    → 自己也推进
      └──────────────► 于是 A 推一步通知 B，B 推一步通知 C …
                      像多米诺一样直到 GameplayReady
```

### 字段流向图：通知单里 4 个字段是谁填的

```
  发出端组件                             大管家                                订阅端组件
 ┌─────────────┐    TryToChange   ┌──────────────────────┐   广播回调     ┌──────────────────┐
 │ 我(总指挥)    │ ───────────────► │ 我来填这张单子：        │ ─────────────► │ OnActorInitState   │
 │ 想推进状态    │   DesiredState  │  • OwningActor = 我   │  Params&      │ Changed(Params&)   │
 └─────────────┘                  │    的 Owner Actor     │               │    读：             │
                                  │  • FeatureName = 我   │               │  FeatureName ==    │
                                  │    注册的名字          │               │    我关心的那个？    │
                                  │  • FeatureState = 新  │               │  FeatureState ==    │
                                  │    状态 Tag           │               │    我等的那个状态？  │
                                  │  • ImplementingObject │               │  是 → CheckDefault  │
                                  │    = 我这个组件        │               │  Initialization()   │
                                  └──────────────────────┘               └──────────────────┘
    【组件只需要：喊一嗓子】           【大管家负责：登记+填单+广播】        【订阅者只需要：判断+推进】
```

### 一句话记住流转

> **谁要往前走，就喊一嗓子（TryToChangeInitState）；大管家负责登记状态、填好通知单（Params）、广播出去；谁关心这个变化，谁就自己在回调里读单子、判断、推进。** 通知单上的 4 个字段（谁、哪个特性、到哪个状态、哪个组件）就是让大家"对得上号"的凭据。

> 关键点（大管家源码里的机制，第 04 篇讲过）：状态变化是**放进队列**处理的，避免 A 通知 B、B 又通知 A 的递归死循环。

---

## 五、Lyra 代码里实际怎么读它（两处对照）

### 用法 1：`ULyraPawnExtensionComponent`（总指挥，`LyraPawnExtensionComponent.cpp` 第 280~290 行）

```cpp
void ULyraPawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// 如果是别的 feature 变了（不是自己）
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		// 而且它进入了 DataAvailable → 说明我依赖的某个零件就绪了
		if (Params.FeatureState == LyraGameplayTags::InitState_DataAvailable)
		{
			CheckDefaultInitialization();   // 再检查一次自己能不能往下走
		}
	}
}
```

它在 `BeginPlay` 里这样订阅（第 61 行）：
```cpp
	// Listen for changes to all features（NAME_None = 监听所有特性的变化）
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);
```

### 用法 2：`ULyraHeroComponent`（`LyraHeroComponent.cpp` 第 186~196 行）

```cpp
void ULyraHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == ULyraPawnExtensionComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == LyraGameplayTags::InitState_DataInitialized)
		{
			// 总指挥宣布"所有依赖的组件都初始化好了" → 我也试着推进到下一状态
			CheckDefaultInitialization();
		}
	}
}
```

它在 `BeginPlay` 里**只盯着总指挥**（第 211 行）：
```cpp
	// Listen for when the pawn extension component changes init state
	BindOnActorInitStateChanged(ULyraPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);
```

**对照看两处差异，就读懂了这套机制**：
- 总指挥用 `NAME_None` 订阅 → **监听所有组件**（它是大管家，谁到 `DataAvailable` 它都想推进）。
- Hero 组件订阅时指定 `PawnExtension` 这个 FeatureName → **只监听总指挥**（它只听总指挥宣布"齐了"）。
- 两者收到 `Params` 后做的事一样：**判断是不是自己关心的那个状态，是就 `CheckDefaultInitialization()`**。

---

## 六、为什么需要它？（而不是直接调用）

如果组件 A 要推进时就"点名"去调 B，两者就**写死耦合**了：

| 方案 | 问题 |
|---|---|
| A 直接调 B 的初始化 | A 得知道 B 存在、B 是谁、B 在不在乎——加新组件要改 A |
| A 只推自己的状态，由大管家广播 `Params` | **A 不用知道谁关心它**；B/C/D 想等，自己订阅即可 |

**这就是"齐步走"能成立的根基**：每个组件不需要认识所有同伴，只需要：
1. 把自己的状态推进 → 报告给大管家；
2. 订阅自己关心的"某个 Feature 到达某个状态"的通知；
3. 收到 `Params` 后判断、推进、再广播……链式完成整条初始化。

> **教学场景**：餐厅出餐。厨师（组件）做好菜就喊"XX 桌好了"（状态推进 + 广播）；传菜员（另一个组件）**只订阅**"我负责的那几桌"的通知，听到就端走。厨师不用认识每个传菜员，传菜员加多少个都不影响厨师。`FActorInitStateChangedParams` 就是那句"XX 桌好了"里包含的**桌号（Actor）+ 菜品（Feature）+ 完成度（State）**。

---

## 七、总结一句话

> **`FActorInitStateChangedParams` 是 ModularGameplay 初始化状态系统的"状态变化通知单"**：装着"哪个 Actor 的哪个 Feature 变成了什么状态"（`OwningActor` / `FeatureName` / `FeatureState` 等）。组件推进一步状态后，大管家组好这张单子广播出去；订阅者（如 LyraPawnExtensionComponent、LyraHeroComponent）在 `OnActorInitStateChanged` 里读它、判断"我等的状态到没到"，到了就自己也推进——**正是靠这张"通知单"，各个组件才能互不耦合地"齐步走"完成初始化。**

---

## 八、下一步

- 追大管家源码：`TryToChangeInitState` 怎么校验、入队、最终广播 `Params`（可看第 04 篇的 `StateChangeQueue`）。
- 对比 `LyraPawnExtensionComponent`（监听所有人）与 `LyraHeroComponent`（只听总指挥）的订阅差异，理解 `FeatureName` 的作用。
- 理解 `BindOnActorInitStateChanged` 的返回值 `FDelegateHandle` 和解除订阅 `Unbind` 的生命周期管理。
