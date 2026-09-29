# 33 — `ensureAlwaysMsgf((Pawn != nullptr), ...)` 是什么？OnRegister 里的"运行时断言"

> **定位**：`LyraPawnExtensionComponent.cpp` 第 46 行（在 `OnRegister` 里）：
>
> ```cpp
> const APawn* Pawn = GetPawn<APawn>();
> ensureAlwaysMsgf((Pawn != nullptr), TEXT("LyraPawnExtensionComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));
> ```
>
> 这行是 **UE 的"运行时断言（ensure）"**——意思很直白：**"如果 Pawn 是空的（组件没挂在角色上），就在这里停下报警，因为这本不该发生。"**
>
> 这篇讲：ensure 系列是什么、和 check/verify 什么区别、为什么 Lyra 在这里用它、它"崩不崩"取决于什么。

---

## 〇、30 秒先给答案

| 问题 | 答案 |
|---|---|
| `ensureAlwaysMsgf` 是什么 | UE 的**运行时断言宏**（条件不满足就报错/中断） |
| 它在检查什么 | 这个组件**必须挂在 Pawn 上**（`GetPawn` 不能返回空） |
| 不满足会怎样 | Debug/开发版：弹窗 + 中断进调试器；Shipping：打日志但**继续跑** |
| 为什么写它 | 防"把总指挥挂到非 Pawn 上"的配置错误，早暴露早修 |
| 名字里 Always 啥意思 | 每次都查（区别于只查一次的 ensure） |

---

## 一、先拆解这一行（语法层面）

```cpp
ensureAlwaysMsgf( (Pawn != nullptr),                                    // ① 条件
                  TEXT("LyraPawnExtensionComponent on [%s] can only be added to Pawn actors."),  // ② 格式串
                  *GetNameSafe(GetOwner()) );                           // ③ 参数（填进 %s）
```

| 部分 | 含义 |
|---|---|
| ① 条件 `Pawn != nullptr` | "如果 Pawn 是空指针，说明出问题了" |
| ② 格式串 | 出错时打印的话：**"这个组件只能加到 Pawn 角色上"** |
| ③ `*GetNameSafe(GetOwner())` | 把挂载它的 Owner（那个 Actor）的名字填进 `%s`，方便查是谁挂错了 |

> 合起来：**"如果这组件没挂在角色上（Pawn 是空），就在这里报警，并把挂它的那个对象名字打出来。"** 这是个"给开发者看"的提示，不是给玩家的。

---

## 二、背景：这段代码在哪个函数里？（OnRegister）

先看它所在函数（`LyraPawnExtensionComponent.cpp` L41~54）全貌：

```cpp
void ULyraPawnExtensionComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();                                   // 拿我挂载的 Pawn
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("... can only be added to Pawn actors."), *GetNameSafe(GetOwner()));   // ← 检查

	TArray<UActorComponent*> PawnExtensionComponents;
	Pawn->GetComponents(ULyraPawnExtensionComponent::StaticClass(), PawnExtensionComponents);   // 找出所有总指挥组件
	ensureAlwaysMsgf((PawnExtensionComponents.Num() == 1), TEXT("Only one LyraPawnExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));   // 只允许一个
	...
}
```

**`OnRegister` 是什么时候跑的？** 组件被注册到 Actor 上的时刻（引擎生命周期，比 BeginPlay 早）。所以这是**组件第一次能确定"我挂在谁身上"的地方**——很适合做"我挂对了没"的体检。

> 前面第 29 篇讲过：构造函数太早（还没挂到 Actor，不知道 Owner）；`OnRegister` 时组件已经知道挂在哪个 Actor 上了。所以**这种"我挂错对象没"的检查放 OnRegister 最合适**。

---

## 三、ensure 家族：check / ensure / verify 三大断言区别

UE 有三类常用断言，**它们最大的区别是"失败后崩不崩"**：

| 宏 | 失败后 | 适用场景 | 名字含义 |
|---|---|---|---|
| `check(x)` | **必定崩溃**（直接终止） | "永远不可能失败，失败就是程序 bug" | 硬断言 |
| `ensure(x)` | 报错/中断，**但能继续跑**（非 Shipping） | "不应该发生，但发生了也别让游戏直接挂" | 软断言 |
| `ensureAlways(x)` | 同 ensure，但**每次都查** | 同上 + "每次调用都要验证" | 软断言·无缓存 |
| `verify(x)` | 同 check 但**表达式总执行** | "我想执行这表达式，失败了才崩" | 带执行 |
| `ensureMsgf`/`ensureAlwaysMsgf` | ensure 的**带格式消息**版 | 报错时附上自定义文字（帮助排查） | 软断言·可打印 |

### ensure vs ensureAlways 的区别（容易忽略）

| | `ensure(cond)` | `ensureAlways(cond)` |
|---|---|---|
| 检查频率 | **同一调用点只报一次**（有缓存，防刷屏） | **每次调用都检查**（无缓存） |
| 适用 | 循环里可能多次触发，报一次够了 | 希望每次现场都抓（如这里的注册检查） |

**为什么 OnRegister 用 `ensureAlways`？** `OnRegister` 只在组件注册时跑一次，不涉及"循环刷屏"问题，且希望每次（尤其热重载/重新注册时）都认真检查——用 Always 确保不因缓存漏报。

### check vs ensure 怎么选（核心思想）

> - **check**：条件"数学上不可能失败" → 失败=程序有 bug，直接崩，别带病运行。
> - **ensure**：条件"理论上不该失败，但环境可能异常" → 失败=配置/环境问题，**报错提示但让游戏继续**（比如这里：挂错了组件，游戏不该直接崩，报出来让人改配置）。

**这里的 Pawn 空指针属于哪类？** 属于"配置错误"——有人把组件加到非 Pawn 的 Actor 上。用 `ensure`（而不是 check）是因为：**挂错组件是"配置问题"不是"代码 bug"，报警提醒即可，不该让整个游戏崩溃**；但也不该静默（静默了后面用 Pawn 会莫名崩，更难查）。

---

## 四、为什么"崩不崩"还分版本？（关键）

ensure 家族的行为**依赖编译配置**（这是它最反直觉的地方）：

| 编译配置 | ensure 失败时 |
|---|---|
| Debug / Development（开发） | 报错弹窗 + 可选中断进调试器，**可继续跑** |
| Test | 记录日志 |
| **Shipping（发行版）** | **ensure 被编译掉/仅最小处理**，几乎零开销 |

> 所以 ensure 是"**开发期帮你抓问题，发行版不影响性能**"的工具。这也回答"为什么到处都是 ensure 却发行版不卡"——发行版里它们基本不存在。

---

## 五、为什么不在编译期检查，非要在运行时 ensure？

你可能会想：'组件只能挂 Pawn 上'，不能编译期就拦住吗？

**不能**——因为组件能挂谁，是**运行时配置决定的**（你在蓝图/关卡里把它拖到哪个 Actor 上）。C++ 编译器不知道。只能在**运行时注册的那一刻**（`OnRegister`）检查"我现在挂的到底是不是 Pawn"。

> **类比**：门禁卡理论上该发给员工，但你没法在"造卡"时知道谁会拿错——只能在**刷卡进门那一刻**（OnRegister）验证"这卡是不是员工的"。ensure 就是"刷卡时的检查"。

---

## 六、总结一句话

> **`ensureAlwaysMsgf` 是 UE 的运行时软断言**：检查"这个组件必须挂在 Pawn 上"（`Pawn != nullptr`），失败就打印带挂载对象名字的错误（方便定位是谁挂错了）。它放在 `OnRegister`（组件注册、确定 Owner 的时刻）做"挂对没"的体检。区别于 check（失败必崩）：挂错组件属于"配置错误"，ensure 让它**报错但不崩游戏**；`Always` 表示每次注册都查（不缓存）；`Msgf` 表示带自定义说明文字。开发版抓问题、发行版近乎零开销。

---

## 七、下一步

- 在 Lyra 里搜其他 `ensure`/`check`/`verify` 用法，对比它们各自用在什么"性质"的错误上，体会选型。
- 去引擎源码搜 `ensureAlwaysMsgf` 宏定义，看它在不同编译配置下展开成什么。
- 试把这一行的 ensure 换成 check，体会"如果总指挥挂错了就直接崩"会是什么体验（加深 check vs ensure 理解）。
