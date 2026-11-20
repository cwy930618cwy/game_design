# virtual / override / Super 三者区别 —— 用 C++ 讲透（对照 LyraPawn 真实代码）

> **定位**：彻底分清 C++ 里最容易混的三个"继承相关关键字"——`virtual`、`override`、`Super`。它们长得像、都跟父子类有关，但**根本不是一类东西**。
>
> **关联**：
> - [17_生命周期函数PreInitialize与EndPlay详解](./17_生命周期函数PreInitialize与EndPlay详解.md) — 这两个函数就是靠这三者配合工作的
> - [11_什么时候必须重写什么时候不用详解](./11_什么时候必须重写什么时候不用详解.md)
> - [07_父接口如何提供默认实现_继承与多态](./07_父接口如何提供默认实现_继承与多态.md)
>
> **一句话**：`virtual` 管"**能不能被覆盖**"（父类说了算），`override` 管"**我确实覆盖了它**"（子类表个态），`Super` 管"**去调用父亲那版**"（子类主动转手）。

---

## 一、先看真实代码（你就问这三个的区别）

这是 `LyraPawn.h` 的声明：

```cpp
//~AActor interface
UE_API virtual void PreInitializeComponents() override;   // ← virtual 和 override 同时出现
UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
//~End of AActor interface
```

这是 `LyraPawn.cpp` 的实现：

```cpp
void ALyraPawn::PreInitializeComponents()
{
    Super::PreInitializeComponents();    // ← Super 登场
}
```

**三个关键字全在这一小段里**：`virtual`、`override`、`Super`。下面把它们一个个拆开。

---

## 二、一张表先看清本质区别

| 关键字 | 出现在哪 | 作用 | 谁说了算 | 类比 |
|--------|---------|------|---------|------|
| **`virtual`** | **父类**定义函数时 | 声明"这个函数**允许**被子类覆盖"，并开启多态 | 父类 | 爸爸在门上挂个牌子："此房间可改造" |
| **`override`** | **子类**重写函数时 | 告诉编译器"**我就是来覆盖父类那个函数的**"，请帮忙检查 | 子类 | 你在门口说："对，我就是来改造这间的" |
| **`Super`** | **子类**函数体内部 | 在代码里**主动调用父类的版本**（不是自动的！） | 子类写不写 | 你改造完，亲自去喊一声"爸，出来看看" |

> **核心**：`virtual` 和 `override` 是**声明时的修饰**（描述"这个函数的身份"），`Super` 是**函数体里的一个调用动作**（真正去执行父亲的代码）。

---

## 三、逐个拆解（纯 C++ 例子）

### ① `virtual` —— 父类开的"许可"

```cpp
class Animal {
public:
    virtual void speak() {              // ← virtual：这个函数"可以被覆盖"
        cout << "（某种动物的叫声）" << endl;
    }
};
```

- `virtual` 是**父类**加在自己函数上的。
- 意思：**"我这个函数只是个默认版本，子类想改就改吧。"**
- 它同时开启了**多态**：通过父类指针调用时，会跑实际对象的那个版本（后面会讲）。
- **没有 `virtual` 的函数**：也能被同名函数"遮住"，但那是**静态绑定**（看指针类型决定调谁），不是多态，容易踩坑。

### ② `override` —— 子类表的"决心"

```cpp
class Dog : public Animal {
public:
    void speak() override {             // ← override：明确告诉编译器"我在覆盖父类的 speak"
        cout << "汪汪汪！" << endl;
    }
};
```

- `override` 是**子类**加在自己函数上的（C++11 引入）。
- 意思：**"我就是来覆盖父类那个虚函数的，请你核对一下——如果我写的签名跟父类对不上，请直接报错！"**
- **它的唯一作用是"编译期检查"**：帮你防止"想覆盖却没覆盖成功"的 bug（比如拼错名字、参数不一致）。
- ⚠️ **`override` 本身不影响运行行为**，它只是给编译器看的"确认键"。

### ③ `Super` —— 子类主动"打电话给父亲"

```cpp
class Dog : public Animal {
public:
    void speak() override {
        cout << "（先深吸一口气）" << endl;
        Super::speak();                 // ← Super：主动调用父类的 speak
        cout << "汪汪汪！" << endl;
    }
};
```

- `Super` 是 UE 对 C++ 原生写法 `父类名::函数名()` 的**封装别名**（`Super` = 直接父类）。
- 意思：**"在我这个函数里，我想顺便执行一下父亲那版代码。"**
- **关键：它不是自动的！** 你不写 `Super::xxx()`，父亲就**完全不知道**这个时机发生过。

> **对比标准 C++**：标准 C++ 里没有 `Super` 这个关键字，得写 `Animal::speak()`（把 `Animal` 换成你父类的名字）。UE 为了省事，给了个 `Super` 当万能替身。

---

## 四、回到 LyraPawn：为什么三个要一起出现？

```cpp
// .h
virtual void PreInitializeComponents() override;   // virtual(继承来的身份) + override(我的表态)
// .cpp
Super::PreInitializeComponents();                   // 主动调父亲
```

逻辑串起来是这样的：

```
① virtual（来自 AActor/AModularPawn）
     父亲早就把 PreInitializeComponents() 声明成虚函数
     → "这个时机允许子类来响应"

② override（ALyraPawn 加的）
     LyraPawn 说："好，我也要在这个时机做点事"
     → 于是重写了它（哪怕函数体几乎是空的）

③ Super::（ALyraPawn 在函数体里主动调）
     LyraPawn 说："但我爹在 AModularPawn 里藏着关键逻辑（报到/销号），
                  千万别漏了他——所以我亲自把他请出来"
     → Super::PreInitializeComponents()
```

**三者缺一不可的角色**：
- 没有 `virtual`（父类层面）→ 这函数根本没"可被覆盖"的机制，谈什么重写。
- 没有 `override`（子类层面）→ 照样能重写，但少了编译检查，容易"以为覆盖了其实没覆盖"。
- 没有 `Super::`（函数体里）→ **父亲 `AModularPawn` 的组件装配逻辑就不会执行，组件挂不上去！**

---

## 五、最关键的一个坑：重写 ≠ 自动通知父亲

很多人以为"我重写了这个函数，父亲那边应该会自动执行吧？"——**不会！**

```cpp
class Dog : public Animal {
    void speak() override {
        // 如果这里不写 Super::speak()，
        // Animal::speak() 就永远不会被调用！
        cout << "汪汪汪！" << endl;
    }
};
```

- **重写（override）只是"我也参与这个时机"**，不代表会把活交给父亲。
- **想让父亲也执行，必须显式写 `Super::函数名()`**。
- LyraPawn 那两个函数之所以"只调了 Super"，就是因为**它的目的纯粹是"把父亲请出来干活"**，自己没啥要补充的。

### 类比

> 公司流程：总部（父亲）规定"每天开门前要打扫卫生"（`virtual` 函数）。
> 分公司（子类）接手后也可以重写这个流程。
> - `override` = 分公司说"行，这个流程我也走"。
> - `Super::` = 分公司**亲自打电话给总部**："你们那套打扫流程还是照常执行啊！"
> - 如果分公司不打电话（不写 Super），总部就**真的不来了**，卫生没人扫。

---

## 六、`virtual` 带来的"多态"到底是怎么回事？

这是 `virtual` 最核心的价值，单独讲一下。

```cpp
Animal* a1 = new Dog();
Animal* a2 = new Cat();

a1->speak();   // 输出：汪汪汪！  ← 注意：指针类型是 Animal，但跑的是 Dog 的 speak
a2->speak();   // 输出：喵喵喵！  ← 跑的是 Cat 的 speak
```

- **没有 `virtual`**：`a1->speak()` 会调 `Animal::speak()`（看指针类型 Animal），那就闹笑话了——狗不会叫。
- **有 `virtual`**：运行时看**实际对象类型**（Dog），于是调 `Dog::speak()`——这就是**多态**。

> **记忆**：`virtual` 让"调用哪个版本"由**运行时对象的真实类型**决定，而不是由**指针/引用的声明类型**决定。

---

## 七、三者关系图

```
                    【声明阶段】                          【运行阶段】
                 （描述函数身份）                       （真正执行代码）

   父类 ── virtual func(){}          定义"可被覆盖的默认版"
                │
                │ 允许覆盖
                ▼
   子类 ── func() override {}        表态"我来覆盖"+ 编译检查
                │
                │ 函数体里如果想让父亲也执行
                ▼
         Super::func();  ───────────►  主动调用父亲那版（手动，非自动！）


   ★ virtual  = 父类给的"许可"（能不能被覆盖）
   ★ override = 子类表的"决心"（我确实覆盖了，请检查）
   ★ Super    = 子类发的"呼叫"（去执行父亲的代码）
```

---

## 八、常见误区

| 误区 | 正确理解 |
|------|---------|
| "重写会自动调用父类" | ❌ 必须显式写 `Super::` / `父类名::`，否则父亲啥都不知道 |
| "`override` 会让程序更慢/更快" | ❌ 它只是编译期检查，不影响运行（Release 下甚至会被优化掉） |
| "不加 `virtual` 也能多态" | ❌ 不加 `virtual` 是静态绑定（看指针类型），不是真多态 |
| "`virtual` 是子类加的" | ❌ `virtual` 是**父类**加的；子类只需 `override` |
| "UE 的 `Super` 是 C++ 关键字" | ❌ C++ 标准里没有；UE 用它代替 `父类名::`，且 `Super` 专指**直接**父类 |
| "函数体空着不调 Super 也没事" | ❌ LyraPawn 这种就靠 Super 触发父亲逻辑，漏了就出 bug（如组件挂不上） |

---

## 九、动手练一练

补全下面的代码，体会三者配合：

```cpp
#include <iostream>
using namespace std;

class Base {
public:
    // TODO: 把这个函数声明为"可被覆盖"
    virtual void init() {
        cout << "Base 初始化" << endl;
    }
};

class Derived : public Base {
public:
    // TODO: 覆盖 init，并在里面调用父亲的 init
    void init() __________ {      // ← 填关键字
        cout << "Derived 先做事";
        ________::init();          // ← 填关键字，调用父亲
        cout << "，Derived 完成" << endl;
    }
};

int main() {
    Derived d;
    d.init();
    // 预期输出：Derived 先做事Base 初始化，Derived 完成
    return 0;
}
```

<details>
<summary>点击查看答案</summary>

```cpp
class Base {
public:
    virtual void init() {
        cout << "Base 初始化" << endl;
    }
};

class Derived : public Base {
public:
    void init() override {
        cout << "Derived 先做事";
        Super::init();       // 或写 Base::init()
        cout << "，Derived 完成" << endl;
    }
};
```
</details>

---

## 十、总结

```
三个关键字，三种职责，千万别混：

┌─ virtual ─ 父类加 ── "这个函数允许被覆盖"（开绿灯 + 开启多态）
│
├─ override ─ 子类加 ── "我确实覆盖了它"（编译期检查，防写错）
│
└─ Super ─── 子类函数体里写 ── "去执行父亲那版"（手动调用，非自动）

记住三个"不"：
  1. 重写 ≠ 自动通知父亲（必须写 Super::）
  2. override 不影响运行（只是给编译器看的）
  3. virtual 是父类加的，不是子类加的

对照 LyraPawn：
  virtual ... override  → 声明"我要响应这个时机"
  Super::PreInitializeComponents()  → 亲自把父亲 AModularPawn 请出来挂组件
```

**一句话**：`virtual` 是**父亲给的许可**，`override` 是**儿子的表态**，`Super` 是**儿子主动拨通父亲的电话**——前两个是"身份描述"，第三个才是"真正的行动"。

---

## 十一、下一步

- [17_生命周期函数PreInitialize与EndPlay详解](./17_生命周期函数PreInitialize与EndPlay详解.md) — 这两个函数为何必须调 Super
- [11_什么时候必须重写什么时候不用详解](./11_什么时候必须重写什么时候不用详解.md)
- [07_父接口如何提供默认实现_继承与多态](./07_父接口如何提供默认实现_继承与多态.md)
