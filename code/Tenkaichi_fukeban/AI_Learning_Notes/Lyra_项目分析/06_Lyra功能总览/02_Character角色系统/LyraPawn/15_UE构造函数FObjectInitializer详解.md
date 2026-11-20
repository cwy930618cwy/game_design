# 15 — UE 构造函数：`FObjectInitializer` 详解

> **定位**：解释 `ALyraPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())` —— UE 的**构造函数写法**。
>
> **一句话**：这是 UE 对象的构造函数，带一个 `FObjectInitializer` 参数。**它用来在构造时创建子对象（组件）、初始化属性**，是 UE 对象构造的标准写法。

---

## 一、先看这行代码

```cpp
UE_API ALyraPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
//              ↑ 构造函数的参数                          ↑ 默认参数
```

拆开：

| 部分 | 含义 |
|------|------|
| `ALyraPawn(...)` | 构造函数 |
| `const FObjectInitializer& ObjectInitializer` | 参数：对象初始化器（引用） |
| `= FObjectInitializer::Get()` | 默认参数：不传就用默认初始化器 |

---

## 二、为什么 UE 构造函数要有 `FObjectInitializer` 参数？

**普通 C++ 构造函数**：只创建对象本身。
**UE 构造函数**：还要**创建子对象（组件）、初始化属性**，这些需要 `FObjectInitializer`。

### 普通 C++ vs UE

```cpp
// 普通 C++ 构造函数
class Dog {
    Dog() { /* 只初始化自己 */ }
};

// UE 构造函数（带 FObjectInitializer）
class ALyraPawn : public AModularPawn {
    ALyraPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
        : Super(ObjectInitializer)   // 传给父类
    {
        // 这里创建子对象（组件）
    }
};
```

**`FObjectInitializer` 用来干嘛**：它帮 UE 对象**创建子对象**（如组件），并把初始化信息传给父类构造函数。

---

## 三、`FObjectInitializer` 最常用的地方：`CreateDefaultSubobject`

**`CreateDefaultSubobject`（创建默认子对象）** 是 UE 构造函数里最常做的——创建组件。它需要通过 `FObjectInitializer` 调用。

```cpp
ALyraPawn::ALyraPawn(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)   // 先传给父类
{
    // 通过 ObjectInitializer 创建子对象（组件）
    // 例：创建一个静态网格组件
    // MeshComp = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Mesh"));
}
```

**`CreateDefaultSubobject` 的作用**：创建组件并注册，让组件能挂在 Actor 上。**它是 UE 构造函数里创建组件的标准方式**（你之前学组件系统时见过）。

---

## 四、配具体场景：创建组件

**场景：ALyraPawn 构造时创建组件**

```cpp
// LyraPawn.cpp 里（简化）
ALyraPawn::ALyraPawn(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)   // ① 传给父类（重要！）
{
    // ② 用 ObjectInitializer 创建子对象（组件）
    // 例如：
    // RootComp = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
    // RootComponent = RootComp;
}
```

**为什么要 `Super(ObjectInitializer)`**：把初始化器传给父类，父类才能先初始化自己的东西，再回到子类创建自己的组件。**必须传，否则父类没初始化。**

---

## 五、那 `= FObjectInitializer::Get()` 默认参数是啥？

```cpp
ALyraPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
```

**默认参数**：如果调用时**不传**这个参数，就用 `FObjectInitializer::Get()`（一个默认的初始化器）。

```
调用方式 1：显式传
  ALyraPawn(SomeInitializer);

调用方式 2：不传（用默认）
  ALyraPawn();   // 自动用 FObjectInitializer::Get()
```

**作用**：让构造函数**可以省略参数**——多数时候不用手动传，用默认的就行。**这个默认参数让调用更简单。**

---

## 六、为什么是 `const FObjectInitializer&`（引用）？

```cpp
const FObjectInitializer& ObjectInitializer
//    ↑ const         ↑ &（引用）
```

- **`const`**：构造函数只"用"初始化器，不改它
- **`&`（引用）**：不拷贝，直接用（省内存）

**一句话**：用引用避免拷贝，加 const 表示"只读不改"。

---

## 七、用故事理解：工厂组装机器人

> 用生活比喻，把 FObjectInitializer 彻底讲透。

### 7.1 机器人不是从零造的，是有"底子"的

```
你不是凭空造一台机器人
你是拿一台"基础机器人"（父类 AModularPawn）改造
  → 先要有基础机器人
  → 再往上加零件（组件）
```

**`Super(ObjectInitializer)`** = "先把基础机器人交给父类工厂，让它把基础部分组装好"。

```
你：我要一台机器人
  → 传给父类：先造基础机器人
  → 父类造好基础（身体、骨架）
  → 轮到你加零件
```

### 7.2 加零件（组件）需要一个"螺丝刀套装"

```
基础机器人造好了，你要给它加零件（组件：相机、血量）
  → 加零件需要工具（创建子对象）
  → 这个"工具"就是 FObjectInitializer
```

**`FObjectInitializer` = 一把"万能螺丝刀套装"**，用来"往机器人上装零件（组件）"。

```
ObjectInitializer.CreateDefaultSubobject<UHealthComponent>(...)
= "用螺丝刀套装，往机器人上装一个血量组件"
```

### 7.3 为什么构造对象要带这个"螺丝刀"？

**因为 UE 的对象不是"凭空捏的"，是"要注册、要管理"的**：

```
普通 new：捏一个东西，自己管
UE 创建对象：
  需要螺丝刀（FObjectInitializer）来：
  ① 创建组件（零件）
  ② 让 UE 系统知道这个对象存在（注册）
  ③ 传给父类初始化
```

**FObjectInitializer = "UE 创建对象的官方工具箱"**，每个对象构造时都带着它，用来装组件、注册对象。

### 7.4 默认参数 = "不带螺丝刀也行，工厂有备用"

```
ALyraPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
//                                                              ↑ 默认参数
```

**`= FObjectInitializer::Get()`** = "如果你不自己带螺丝刀，工厂有公用的备用螺丝刀，直接用那个"。

```
调用时：
  带螺丝刀 → 用你的
  不带 → 用工厂的公用螺丝刀（默认）
```

### 7.5 完整故事：组装一台机器人（连起来）

```
你想造一台 ALyraPawn 机器人：

1. 你带着"螺丝刀套装"（FObjectInitializer）来到工厂
   （不带你也有公用的，= 默认参数）

2. 先把基础机器人交给父类工厂
   （Super(ObjectInitializer) = "把螺丝刀给父类，先造基础身体"）

3. 父类造好基础机器人（骨架、基础功能）

4. 轮到你，用螺丝刀装零件（组件）：
   ObjectInitializer.CreateDefaultSubobject<组件> = "装相机/血量零件"

5. 一台完整的 ALyraPawn 机器人造好了
```

### 7.6 一句话（用故事）

> **UE 创建对象 = 工厂组装机器人**，`FObjectInitializer` = **"螺丝刀套装"**（用来装组件/注册对象）。
>
> - `Super(ObjectInitializer)` = 把螺丝刀给父类，先造基础
> - `CreateDefaultSubobject` = 用螺丝刀装零件（组件）
> - `= FObjectInitializer::Get()` = 不带也有公用螺丝刀（默认）
>
> **所以 UE 构造函数要带 FObjectInitializer，是为了"装组件（零件）"。**

---

## 八、总结速查

```
UE 构造函数：
  ALyraPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
    : Super(ObjectInitializer)      // 传给父类（必须）
  {
    // 创建子对象（组件）
    // ObjectInitializer.CreateDefaultSubobject<...>(...)
  }

各部分：
  FObjectInitializer& = 对象初始化器（创建子对象用）
  const = 只读不改
  & = 引用（不拷贝）
  = FObjectInitializer::Get() = 默认参数（可不传）
  Super(ObjectInitializer) = 传给父类（必须）

核心作用：创建组件（CreateDefaultSubobject）
```

**一句话**：UE 构造函数带 `FObjectInitializer` 参数，**用来创建子对象（组件）**。必须 `Super(ObjectInitializer)` 传给父类，用 `ObjectInitializer.CreateDefaultSubobject` 创建组件。`= FObjectInitializer::Get()` 是默认参数，让调用可省略。**这是 UE 对象构造的标准写法。**

---

## 八、下一步

理解了构造函数，下一步可以看 **`ALyraPawn` 的 .cpp 里实际怎么用 ObjectInitializer 创建组件**（`02_LyraPawn.cpp详解.md`）。
