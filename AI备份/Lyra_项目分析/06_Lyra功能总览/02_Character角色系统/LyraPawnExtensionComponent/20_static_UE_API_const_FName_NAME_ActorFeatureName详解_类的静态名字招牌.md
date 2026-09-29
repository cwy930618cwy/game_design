# 20 — `static UE_API const FName NAME_ActorFeatureName;` 看不懂？逐个词拆开讲

> **定位**：`LyraPawnExtensionComponent.h` 第 36 行写着：
>
> ```cpp
> static UE_API const FName NAME_ActorFeatureName;
> ```
>
> 一眼看去每个词都见过，拼起来就懵。这篇**按"写法习惯 + 声明与定义分离 + 它是干嘛的 + 谁在用它"**讲透。
>
> **衔接**：这是第 19 篇 `FActorInitStateChangedParams` 的"续集"——第 19 篇里 `Params.FeatureName` 和 `FeatureName == 某某` 反复出现，那个"某某"就是这个 `NAME_ActorFeatureName`。

---

## 〇、一句话先给结论

> **`NAME_ActorFeatureName` 是 `ULyraPawnExtensionComponent` 这个类的一个"静态名字招牌"**——它代表"我这个组件的特性（Feature）叫 **PawnExtension**"。
>
> 整个初始化状态系统（ModularGameplay）靠"名字"来识别谁是谁，而这个 `static const FName` 就是这个类对外亮出的**身份证名字**。它是**类的**（static），不是某个对象的；是**常量**（const），全工程共享且永不改变。

拆解：`static`（类级）+ `UE_API`（导出宏）+ `const`（不可变）+ `FName`（引擎字符串）+ `NAME_ActorFeatureName`（变量名，前导 `NAME_` 是命名习惯）。

---

## 一、逐个词拆开讲

### 词 1：`static` —— "这个变量属于类，不属于对象"

普通成员变量是"**每个对象一份**"：

```cpp
// 普通成员：创建 100 个组件，就有 100 份 PawnData
UPROPERTY(...) TObjectPtr<const ULyraPawnData> PawnData;
```

`static` 成员是"**整个类只有一份**"：

```cpp
// 静态成员：不管你创建多少个组件，这个名字全工程只有一份
static UE_API const FName NAME_ActorFeatureName;
```

**大白话**：`PawnData` 是"每辆车的车牌号"（每辆车各一个）；`NAME_ActorFeatureName` 是"车厂的名字"（所有这牌子的车共享同一个）。

**访问方式也不同**：
```cpp
// 普通成员：通过某个实例访问
SomeComponent->PawnData;

// 静态成员：不依赖实例，直接用"类名::名字"访问
ULyraPawnExtensionComponent::NAME_ActorFeatureName
```

> **为什么要 static？** 因为"这个特性的名字"是**全类统一、与具体哪个实例无关**的。你场上可以有 10 个 PawnExtension 组件（10 个角色各挂一个），但它们报的名字都必须是同一个 `"PawnExtension"`——否则状态系统就没法把它们当"同一种特性"来协调。所以它天然就该是 static。

### 词 2：`UE_API` —— 导出宏（跨 DLL 可见）

在 `LyraPawnExtensionComponent.h` 第 10 行有：

```cpp
#define UE_API LYRAGAME_API
```

UE 的模块会编译成 DLL，其他模块要调用 `LyraGame` 模块里的类/函数，**必须有导出标记**。`LYRAGAME_API` 就是给 `LyraGame` 模块的符号打"导出"标签，这样 `LyraGame` 之外（比如别的插件、游戏模块）也能访问 `NAME_ActorFeatureName`。

> 为什么这个变量特别需要 `UE_API`？因为**别的组件要跨类引用它**（见第 19 篇：`HeroComponent` 里写 `ULyraPawnExtensionComponent::NAME_ActorFeatureName`）。Hero 组件和 PawnExtension 组件不总在同一个 DLL，不导出就链接不到。
>
> 详细原理见 `LyraPawn/12_API宏导出详解`。

### 词 3：`const` —— "不可修改"

`const FName` = 这个 `FName` 一旦初始化就不能改。因为**名字是"身份标识"，改了身份就乱了**——如果某个组件运行时偷偷把 `"PawnExtension"` 改成 `"PawnX"`，所有等它的人都对不上号。所以它必须永久固定。

### 词 4：`FName` —— UE 的"高效只读字符串"

`FName` 是 UE 三大字符串类型之一（`FString` / `FText` / `FName`）：

| 类型 | 特点 | 用途 |
|---|---|---|
| `FString` | 可变、可拼接、性能较低 | 用户输入、动态文本 |
| `FText` | 支持本地化 | UI 显示文字 |
| `FName` | **不可变 + 内部哈希表，查找/比较极快** | 标识符：变量名、Tag 名、特性名 |

**为什么状态系统用 FName 当"特性名字"？** 因为状态系统要**反复比较**两个名字是否相等（`Params.FeatureName == NAME_ActorFeatureName`）。`FName` 的比较是"内部 ID 比数字"，O(1) 秒出结果；如果用 `FString` 就得逐字符比较，慢得多。**用 FName 就是为"高频身份比较"优化的。**

### 词 5：`NAME_ActorFeatureName` —— 变量名（注意命名习惯）

- `NAME_` 前缀是 UE/Lyra 的**命名约定**：看到 `NAME_` 开头就知道"这是个静态的 FName 常量"，一般对应一个"标识名"。
- 同类例子：`LyraHeroComponent.h` 里还有 `NAME_BindInputsNow`（一个用于发送事件的"名字"）。

---

## 二、最关键的语法：`.h` 只"声明"，`.cpp` 才"定义"

这是 C++ `static const` 成员最容易懵的地方——**这行写在 .h 里并没有真正给它值！**

看 `.h`（第 36 行）只是声明：
```cpp
// LyraPawnExtensionComponent.h
static UE_API const FName NAME_ActorFeatureName;    // ← 只有类型和名字，没有初值
```

真正的值在 `.cpp` 里给出（`LyraPawnExtensionComponent.cpp` 第 20 行）：
```cpp
// LyraPawnExtensionComponent.cpp
const FName ULyraPawnExtensionComponent::NAME_ActorFeatureName("PawnExtension");  // ← 在这里初始化 = "PawnExtension"
```

**为什么 .h 里不给值？** 因为 `static` 成员属于类，**必须全工程只有一份定义**。如果写在 .h 里给初值，而 .h 被很多 .cpp include，就会产生"多份定义"——编译器直接报错（重复定义）。所以规矩是：**.h 声明、.cpp 定义一次**。C++ 语法上 `static` 成员变量必须类外定义一次。

> **类比**：`.h` 是"招聘广告"（声明：我们部门有个叫 `NAME_ActorFeatureName` 的招牌，值待定）；`.cpp` 才是"正式挂牌"（定义：这个招牌上写的是 `PawnExtension`）。广告可以发无数份，但牌子全世界只能挂一次。

---

## 三、它是干嘛的？（回到状态系统的意义）

现在把它放回第 19 篇的语境。整个 ModularGameplay 初始化状态系统，是**按"名字"来协调多个组件**的：

每个想参与状态系统的组件，都要实现 `IGameFrameworkInitStateInterface::GetFeatureName()`，告诉大管家"我管自己叫啥"。而 `ULyraPawnExtensionComponent` 的实现就是**返回这个 static 名字**（`.h` 第 39 行）：

```cpp
virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
//                                                         ↑ 就是返回 "PawnExtension"
```

于是大管家登记时，就把"这个 Actor 上的这个组件"记为名字 `"PawnExtension"`。

### 谁在用它？（源码里的实际引用）

| 使用处 | 代码 | 含义 |
|---|---|---|
| 总指挥自己上报名字 | `GetFeatureName() { return NAME_ActorFeatureName; }` | 登记为 `"PawnExtension"` |
| 总指挥判断"是不是别人变的" | `Params.FeatureName != NAME_ActorFeatureName` | 自己变的不理会 |
| Hero 组件监听总指挥 | `BindOnActorInitStateChanged(ULyraPawnExtensionComponent::NAME_ActorFeatureName, ...)` | 只关心 `"PawnExtension"` 的变化 |
| Hero 判断谁变到 DataInitialized | `Params.FeatureName == ULyraPawnExtensionComponent::NAME_ActorFeatureName` | 是总指挥变的才处理 |
| Hero 检查总指挥是否就绪 | `Manager->HasFeatureReachedInitState(Pawn, ULyraPawnExtensionComponent::NAME_ActorFeatureName, InitState_DataInitialized)` | 问大管家"PawnExtension 到位没" |

> **发现规律**：跨组件沟通时，用的全是 `类名::NAME_ActorFeatureName`（而不是写死字符串 `"PawnExtension"`）——这样如果将来改名，只需改 `.cpp` 那一处，所有引用自动跟着变，**不会出现手写字符串拼错还不报错**的坑。

---

## 四、一句话总结

> `static UE_API const FName NAME_ActorFeatureName;` 是 `ULyraPawnExtensionComponent` 类的一个**静态常量名字招牌**：
> - `static` = 整个类共享一份，用 `类名::` 访问，与实例无关（所有组件的特性名必须统一）；
> - `UE_API` = 跨 DLL 导出，别的模块（如 HeroComponent）才能引用；
> - `const` = 名字不可变（身份标识必须固定）；
> - `FName` = UE 高性能只读字符串（身份比较高频，用 FName 秒比）；
> - `NAME_` = 命名习惯。
> 它在 `.h` 里只声明、在 `.cpp` 第 20 行定义成 `"PawnExtension"`，作为**这个组件在 ModularGameplay 状态系统里的注册名**——`GetFeatureName()` 上报、`HeroComponent` 等订阅/比对全靠 `ULyraPawnExtensionComponent::NAME_ActorFeatureName` 引用，避免手写字符串出错。

---

## 五、下一步

- 复习 `LyraPawn/12_API宏导出详解`（UE_API 完整原理）和第 19 篇（FeatureName 怎么被读）。
- 对照看 `LyraHeroComponent.h` 第 60/63 行两个 static FName（`NAME_BindInputsNow` + `NAME_ActorFeatureName`），理解"一个类可以有多个不同用途的 static FName"。
- 深入 `FName` vs `FString` vs `FText`：什么场景用哪个（做 UI 文本 / 做标识符 / 做动态拼接）。
