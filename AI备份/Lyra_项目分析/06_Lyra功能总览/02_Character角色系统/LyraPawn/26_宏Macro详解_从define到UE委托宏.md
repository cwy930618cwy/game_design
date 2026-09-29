# 宏（Macro）到底是什么？—— 从 `#define` 到 UE 委托宏彻底搞懂

> **定位**：回答一个根本问题"**宏是什么？**"。从最朴素的 `#define` 讲起，一步步讲到 UE 里那些吓人的大宏（如 `DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams`）。看完你再也不会被 `#xxx` 开头的东西唬住。
>
> **关联**：
> - [25_GetTeamChangedDelegateChecked与AddDynamic底层源码剖析](./25_GetTeamChangedDelegateChecked与AddDynamic底层源码剖析.md) — 那个委托宏从哪来
> - [12_API宏导出详解](./12_API宏导出详解.md) — `LYRAGAME_API` 也是宏
> - [20_LyraPawn.h所有类型的出身](./20_LyraPawn.h所有类型的出身_传递依赖全解析.md) — `UCLASS`/`UPROPERTY` 都是宏
>
> **一句话**：宏 = **"编译前的文本替换工具"**。你在代码里写 `#define A B`，编译器就会在编译前把所有 `A` 字面替换成 `B`。它不是程序运行时才执行的代码，而是"编译之前就被换掉了的文字"。

---

## 一、宏最根本的样子：`#define`

一切宏都从这开始：

```cpp
#define PI 3.14159

int main() {
    double r = PI * 2;   // 编译前，这行被悄悄变成 →  double r = 3.14159 * 2;
}
```

**关键理解**：`#define PI 3.14159` 这句话**本身不是 C++ 代码**，它是一条给**预处理器**的命令：

> "从现在起，凡是我看到 `PI` 这个单词，就把它**原样替换**成 `3.14159`。"

### 宏 vs 普通变量（最容易混的地方）

| | 宏 `#define PI 3.14159` | 变量 `const double PI = 3.14159;` |
|---|---|---|
| 本质 | **文本替换**（编译前） | **真正的内存存储**（运行时） |
| 有没有类型 | ❌ 没有类型，就是文字 | ✅ 有类型（double） |
| 占不占内存 | ❌ 不占（它不是数据） | ✅ 占内存 |
| 能不能调试 | ❌ 没法单步（编译前就没了） | ✅ 能 |

> **记忆**：宏不是"值"，是"替换规则"。它在你看到代码之前，就已经把文字换好了。

---

## 二、为什么 UE 爱用宏？三个原因

UE 里到处都是宏（`UCLASS`、`UPROPERTY`、`GENERATED_BODY`、`LYRAGAME_API`……），因为宏能干几件普通函数/变量干不了的事：

### 原因 1：生成样板代码（省打字）

与其每个类都手写一堆重复代码，不如用一个宏一次性展开：

```cpp
// 没有宏：每个类都要写这么多
class MyClass {
    static UClass* GetPrivateStaticClass();
    static bool IsInInternal();
    // ... 一大堆重复的反射代码
};

// 有宏：一行搞定，UHT 帮你生成上面那些
UCLASS()
class MyClass { GENERATED_BODY() };
```

### 原因 2：编译期开关（条件编译）

让同一段代码在不同情况下"存在或消失"：

```cpp
#if WITH_EDITOR          // 如果是在编辑器里编译
    DoEditorOnlyStuff(); // 这段只在编辑器里存在
#endif
```

### 原因 3：跨平台 / 性能优化

把"不同平台不同的写法"统一成一个宏名：

```cpp
// LYRAGAME_API 就是典型：本模块内 = dllexport，外部 = dllimport
#define UE_API LYRAGAME_API
```

---

## 三、宏可以"带参数"——像函数一样

宏不只能替换固定文字，还能接收参数：

```cpp
// 定义：把参数 x 平方
#define SQUARE(x) ((x) * (x))

int main() {
    int a = SQUARE(5);    // 编译前变成 →  int a = ((5) * (5));
    int b = SQUARE(2+3);  // 编译前变成 →  int b = ((2+3) * (2+3));
}
```

> ⚠️ 注意坑：`SQUARE(2+3)` 里的 `2+3` 被替换了**两次**，所以会算两遍加法。宏是"纯文字替换"，不懂"只算一次"。这就是宏比函数危险的地方。

---

## 四、重点来了：UE 的委托宏是怎么"变出类"的？

回到你看不懂的那段（第 25 篇第 15~23 行）：

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnLyraTeamIndexChangedDelegate,   // 参数①：给新类起个名字
    UObject*, ObjectChangingTeam,       // 参数②：第一个参数
    int32, OldTeamID,                   // 参数③：第二个参数
    int32, NewTeamID);                  // 参数④：第三个参数
```

**这一行根本不是普通代码——它是一个宏调用。** 你写的这四个参数，会被宏"喂进"一个模板，展开成一大坨真正的 C++ 类代码。

### 它本质上等价于什么？

宏展开后，**大致**生成了这样一个类（简化还原，核心保留）：

```cpp
// ↓↓↓ 宏 DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams 展开后大概长这样 ↓↓↓
class FOnLyraTeamIndexChangedDelegate : public TMulticastScriptDelegate<FWeakObjectPtr>
{
    // —— 这些就是"三个参数"被塞进来后填进去的 ——
public:
    using FParam1 = UObject*;     // ← 你的第②个参数
    using FParam2 = int32;        // ← 你的第③个参数
    using FParam3 = int32;        // ← 你的第④个参数

    // 自动生成的方法
    void AddDynamic(UObject* InUserObject, /*函数指针*/);
    void RemoveAll(UObject* InUserObject);
    void Broadcast(UObject* ObjectChangingTeam, int32 OldTeamID, int32 NewTeamID);

private:
    TArray<FScriptDelegate> MulticastDelegate;   // 联系人名单数组
};
```

**所以那个宏的作用就是**：你给它"类名 + 三个参数"，它替你写出整个委托类的骨架。`FOnLyraTeamIndexChangedDelegate` 这个类**不是你手写的，是宏生成的**。

### 为什么 UE 要用宏生成类？

因为委托有大量**重复的样板代码**（数组、Add/Remove/Broadcast 的实现），每种"参数组合"的委托结构几乎一样，只是参数类型不同。用宏：
- 程序员只需写一行（类名 + 参数），不用抄几百行。
- UHT 工具能扫描这些宏，额外生成反射数据（让蓝图能用）。

> **类比**：宏就像"饼干模具"。面团（参数）往里一倒，压一下（展开），出来就是一块形状完整的饼干（一个完整的类）。你不用每次都从揉面做起。

---

## 五、宏的"变身"层次（从简单到复杂）

UE 里你见到的宏，按复杂度排：

| 层次 | 例子 | 干了什么 |
|------|------|---------|
| ① 简单文字替换 | `#define UE_API LYRAGAME_API` | 换个名字 |
| ② 带参数的计算 | `#define SQUARE(x) ((x)*(x))` | 套公式 |
| ③ 生成一小段代码 | `#define check(expr) ...` | 展开成几条语句 |
| ④ 生成整个类 | `DECLARE_..._DELEGATE_ThreeParams(...)` | 展开成一个类！ |
| ⑤ 配合 UHT 的反射宏 | `UCLASS()` / `UPROPERTY()` | 生成类 + 反射元数据 |

**你看，宏的能力是递进的**——越往上的宏越像"代码生成器"。委托宏属于第 ④ 层，是最唬人但也最有用的那种。

---

## 六、一张图看懂"宏展开"的过程

```
你写的源码：                          预处理器处理后：
┌──────────────────────────┐         ┌──────────────────────────────┐
│                          │         │                              │
│  #define 类名 参数...      │  编译前  │  class 类名 : public 基类 {     │
│       ↓                  │ ──────► │      自动生成的方法和数据...    │
│  （宏被"展开"）            │  文本替换 │  };                          │
│                          │         │                              │
│  看起来像"魔法"            │         │  其实就是一堆普通代码           │
└──────────────────────────┘         └──────────────────────────────┘
        你看到的"神秘一行"                    编译器真正拿到的东西
```

---

## 七、常见误区

| 误区 | 正确理解 |
|------|---------|
| "宏是一种数据类型" | ❌ 宏是**文本替换规则**，不是数据也不是代码 |
| "宏在运行时执行" | ❌ 宏在**编译之前**就被替换完了，运行时根本没它 |
| "宏和函数一样" | ❌ 宏是文字替换、无类型、参数可能多次求值；函数是真调用 |
| "委托宏声明的就是一个普通函数" | ❌ 它展开成**一整个类**，包含数组、多个方法 |
| "宏很危险所以别用" | ⚠️ 确实易踩坑，但 UE 重度依赖它；看懂它能极大提升读源码能力 |

---

## 八、总结

```
宏（Macro）= 编译前的文本替换工具

最朴素形式：#define A B  →  把所有 A 换成 B（编译前）

UE 为什么爱用：
  1. 生成样板代码（一行顶一百行）
  2. 条件编译（不同情况代码存在/消失）
  3. 跨平台/性能（统一名字，不同实现）

委托宏的本质：
  DECLARE_..._DELEGATE_ThreeParams(类名, 参数...)
  = 一个"饼干模具"，把你的参数塞进去，展开成一整个委托类
  FOnLyraTeamIndexChangedDelegate 这个类不是手写的，是宏生成的

记住：你看到的"神秘一行宏"，编译前都会被展开成一大堆普通 C++ 代码。
      宏不是运行时魔法，是"编译前的文字游戏"。
```

**一句话**：宏就是"**编译前的查找替换**"——`#define` 是最简单的，UE 的委托宏是"高级版"，能把一行参数展开成一整个类。看懂这一点，那些吓人的大宏就不再神秘了。

---

## 九、下一步

- [25_GetTeamChangedDelegateChecked与AddDynamic底层源码剖析](./25_GetTeamChangedDelegateChecked与AddDynamic底层源码剖析.md) — 那个委托宏展开后是什么
- [12_API宏导出详解](./12_API宏导出详解.md) — `LYRAGAME_API` 这种简单宏
- [20_LyraPawn.h所有类型的出身](./20_LyraPawn.h所有类型的出身_传递依赖全解析.md) — `UCLASS`/`UPROPERTY` 等反射宏
