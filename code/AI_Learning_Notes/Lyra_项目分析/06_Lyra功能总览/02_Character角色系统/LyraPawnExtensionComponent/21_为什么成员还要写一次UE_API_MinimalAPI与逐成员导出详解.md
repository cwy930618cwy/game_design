# 21 — 写了 `#define UE_API LYRAGAME_API`，为什么成员还要再写一次 `UE_API`？

> **定位**：看 `LyraPawnExtensionComponent.h` 时你产生了这个疑问：
>
> ```cpp
> // 第 10 行
> #define UE_API LYRAGAME_API
> // ...中间隔了很多...
> // 第 36 行
> static UE_API const FName NAME_ActorFeatureName;   // 这里怎么又要写 UE_API？！
> ```
>
> "我明明已经定义了 `UE_API`，不就等于整个文件都导出/暴露了吗？为什么某个成员前还得再标一次？"
>
> 这篇用**事实 + 原理**把这个困惑彻底拆掉。

---

## 〇、一句话结论（先纠正误解）

> **`#define UE_API LYRAGAME_API` 只是"给 `UE_API` 这个缩写起个别名"，它并不会"自动让整个文件暴露"。** 真正决定"谁被导出"的，是**每个符号（类/函数/静态变量）前面有没有写 `UE_API`**。所以第 36 行想让这个静态变量被别的模块用，就必须再写一次 `UE_API`。

打个比方：
- `#define` = **在键盘上给某个词设了个快捷键**（输入 `UE_API` 自动补全成 `LYRAGAME_API`）。
- 真正"导出" = **你把这句话（快捷键打出来的宏）写在谁前面，谁才被导出**。
- 快捷键不会自动帮你把文件里所有东西都标上导出——**你得自己把 `UE_API` 写到想导出的东西前面**。

---

## 一、先厘清：`#define` 到底做了什么、没做什么

看第 10 行：

```cpp
#define UE_API LYRAGAME_API
```

**它做的事**：以后在这个文件里，凡是写 `UE_API` 的地方，编译器预处理时都替换成 `LYRAGAME_API`。

**它没做的事**：
- ❌ 没有给任何类/函数/变量打上"导出"标记。
- ❌ 没有"遍历文件、把全部符号自动变成可跨模块访问"。

`#define` 只是**文本替换规则**，本身没有任何"导出"能力。**`LYRAGAME_API` 才是导出宏，而它必须出现在某个符号前面才对该符号生效。**

---

## 二、那这个类为什么"没有整体暴露"？—— `UCLASS(MinimalAPI)` 是元凶

现在看类声明（第 26~27 行）：

```cpp
UCLASS(MinimalAPI)                                          // ← 注意这里！
class ULyraPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
```

**注意：类名 `ULyraPawnExtensionComponent` 前面并没有写 `class LYRAGAME_API`！** 只有一个 `UCLASS(MinimalAPI)`。

`MinimalAPI` 的意思是（官方文档原意）：

> **只导出这个类的"类型信息"（Type Info）供其他模块使用，但不导出它的成员函数和静态成员。**

展开说，`MinimalAPI` 下其他模块能做什么、不能做什么：

| 别的模块能 | 别的模块不能 |
|---|---|
| include 这个 `.h`、引用这个**类型** | 直接调用它的**成员函数**（会链接错误） |
| 用 `Cast<>` 安全转换、`StaticClass()` 反射 | 直接访问它的**静态成员**（如 `NAME_ActorFeatureName`） |
| `GetDefaultObject` 拿 CDO | 构造它的实例（除非构造函数单独标了 API） |

> **大白话**：`MinimalAPI` = "**别人能认识我（类型信息），但别想随便调用我的本事（成员）**"。想让人调用某个具体本事，你得**点名放行**——在它前面单独写 `UE_API`。

**为什么 Lyra 这么设计？** 因为暴露越少，编译依赖越轻、二进制越小、改内部实现时影响面越小。**大多数成员只在 LyraGame 模块内部用，不需要给外部；只有少数需要被其他模块（HeroComponent、GameMode 等）直接调用的，才单独放行。**

---

## 三、所以第 36 行"再写一次"的真正原因

对照看：

```cpp
// 第 36 行：这个静态成员需要被别的模块直接引用 → 单独放行
static UE_API const FName NAME_ActorFeatureName;

// 反例：第 99 行的普通成员，只在类内部用 → 不需要 UE_API
UPROPERTY(...)
TObjectPtr<const ULyraPawnData> PawnData;
```

**第 36 行为什么必须加 `UE_API`？** 因为（第 19、20 篇讲过）**其他模块的组件要用 `ULyraPawnExtensionComponent::NAME_ActorFeatureName` 这个静态名字**来做订阅/比对。比如 `LyraHeroComponent.cpp` 里：

```cpp
// HeroComponent 属于另一个组件类，它跨类引用了这个静态名字
ULyraPawnExtensionComponent::NAME_ActorFeatureName
```

如果 `NAME_ActorFeatureName` 前面不写 `UE_API`，在 `MinimalAPI` 模式下它**根本没被导出** → HeroComponent 一链接就报"找不到符号"。所以**为了让外部模块能拿到这个静态变量，必须单独给它加一次 `UE_API` 放行**。

---

## 四、对比：如果想让"整个类"都暴露，怎么写？

这才是理解你最初直觉的关键——**其实确实有一种写法能让"整个文件/整个类都暴露"，但不是靠 `#define`，而是靠把宏写到类名前**：

```cpp
// 写法 A：类级整体导出（不写 MinimalAPI，类名前面直接加宏）
UCLASS()
class LYRAGAME_API UMyClass : public UObject
{
    // 这个类里所有成员都会默认导出，不需要逐个加 UE_API
};

// 写法 B：MinimalAPI + 逐成员导出（LyraPawnExtensionComponent 用的）
UCLASS(MinimalAPI)
class ULyraPawnExtensionComponent : ...
{
    UE_API void SomeFunction();        // 这一个成员单独放行
    static UE_API const FName NAME_ActorFeatureName;   // 这一个静态成员单独放行
    // 其他没写 UE_API 的成员：外部模块用不了
};
```

**两种写法的取舍：**

| 写法 | 别的模块能调用谁 | 代价 |
|---|---|---|
| A：`class LYRAGAME_API X`（类级导出） | **整个类的所有成员** | 暴露面大，改动影响广、编译依赖重 |
| B：`UCLASS(MinimalAPI)` + 逐个 `UE_API` | **只有标了的成员** | 暴露面小，精细控制，但要记得逐个标 |

> **所以你的直觉"应该能一次性全暴露"其实没错——UE 确实支持"整类导出"（写法 A）。** 但 LyraPawnExtensionComponent **故意选了写法 B**（MinimalAPI），把暴露控制到最小。于是你看到的"每个需要公开的成员都单独写一次 UE_API"，正是写法 B 的代价和特征。

---

## 五、补充：为什么构造函数也单独写了 `UE_API`？

看第 33 行：

```cpp
UE_API ULyraPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);
```

MinimalAPI 下，**连构造函数都不自动导出**。而其他模块（比如 GameMode）可能需要 `new` / 创建这个组件，所以构造函数也要单独加 `UE_API` 放行。

> **规律总结**：`MinimalAPI` 模式下，**凡是"外部模块需要直接调用/访问的成员"都必须单独加 `UE_API`**——构造函数、被跨类调用的函数、被跨类引用的静态变量，一个都不能漏。漏了就是链接错误。

---

## 六、总结一句话

> **`#define UE_API LYRAGAME_API` 只是"把 `UE_API` 简写替换成 `LYRAGAME_API`"的文本宏，它本身不会让整个文件自动暴露。** 真正决定"谁被导出"的是：**`UE_API`/`LYRAGAME_API` 有没有出现在那个符号前面**。这个类用的是 `UCLASS(MinimalAPI)`（只导出类型信息、不导出成员），所以想让某个成员被别的模块用，**就必须在它前面再写一次 `UE_API` 单独放行**——第 36 行 `static UE_API const FName NAME_ActorFeatureName` 就是因为 HeroComponent 等外部模块要引用它做状态订阅比对，才需要再标一次。若想"整类都暴露"，应改用写法 A：`class LYRAGAME_API`（直接把宏写在类名前），但 Lyra 刻意不用它，以减少暴露面。

---

## 七、配个场景收尾

> **场景**：你在写一个公共工具类 `UMyUtility`，希望 `LyraGame` 模块外也能调用它的 `PrintLog()` 和静态变量 `Version`。
>
> ```cpp
> // 若你写 UCLASS(MinimalAPI)：
> UCLASS(MinimalAPI)
> class UMyUtility : public UObject
> {
> public:
>     UE_API static void PrintLog(const FString& Msg);   // 外部要用 → 加 UE_API
>     UE_API static const FName Version;                 // 外部要用 → 加 UE_API
>     float InternalTemp = 0.f;                          // 只有内部用 → 不用加
> };
> ```
>
> 如果你不写 MinimalAPI，改成 `UCLASS()` + `class LYRAGAME_API UMyUtility`，那么 `PrintLog`、`Version`、甚至 `InternalTemp` **全都会被导出**——多导出的成员虽然"无害"，但会让模块间耦合变深。**Lyra 选择精细控制，只导出真正要被外部碰的东西。** 这就是你看到的"每个公开成员前都要写一次 UE_API"的原因。

---

## 八、下一步

- 复习 `LyraPawn/12_API宏导出详解`（LYRAGAME_API 的导出/导入原理）。
- 在 Lyra 里对比：搜哪些类是 `class LYRAGAME_API`（整类导出），哪些是 `UCLASS(MinimalAPI)` + 逐个 `UE_API`（精细导出），体会两种风格。
- 尝试把某个成员前的 `UE_API` 去掉再编译，观察"链接错误 LNK2019/2001"——直观理解不导出的后果。
