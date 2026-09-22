# GetTeamChangedDelegateChecked / AddDynamic 底层源码剖析

> **定位**：把第 24 篇那行链式调用背后的**两个方法扒到源码层面**讲清楚——`GetTeamChangedDelegateChecked()`（Lyra 自己写的）和 `AddDynamic()`（UE 引擎标准实现）。看完你就知道"委托"在内存里到底长什么样。
>
> **关联**：
> - [24_委托绑定详解](./24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic.md) — 上层用法
> - [05_LyraTeamAgentInterface队伍接口详解](./05_LyraTeamAgentInterface队伍接口详解.md) — 委托的定义来源
>
> **一句话**：`GetTeamChangedDelegateChecked` = "拿到那个响铃器，并确保它没坏"；`AddDynamic` = "往响铃器的'联系人名单'里加一条记录（谁 + 哪个函数）"。本质就是**往一个数组里塞了个 {对象, 函数指针}**。

---

## 一、先看委托是怎么"声明"出来的

一切从 `LyraTeamAgentInterface.h` 第 15 行这个宏开始：

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnLyraTeamIndexChangedDelegate,   // 委托类型名
    UObject*, ObjectChangingTeam,       // 参数①
    int32, OldTeamID,                   // 参数②
    int32, NewTeamID);                  // 参数③
```

这个宏被 UE 的 UHT 工具展开后，大致生成这样一个类（**简化还原版**，核心字段保留）：

```cpp
// ↓↓↓ 宏展开后大概长这样（简化版，抓核心）↓↓↓
class FOnLyraTeamIndexChangedDelegate : public TMulticastScriptDelegate<FWeakObjectPtr>
{
    // 内部其实就一个"联系人名单"（数组），每个元素是一条绑定记录
    TArray<FScriptDelegate> MulticastDelegate;   // ← 核心：存所有监听者的列表

public:
    // 加一个监听者
    void AddDynamic(UObject* InUserObject, 
                    decltype(&UClass::OnControllerChangedTeam) InFunc, ...);

    // 移除某个对象的所有监听
    void RemoveAll(UObject* InUserObject);

    // 广播（事件发生时调用所有登记的函数）
    void Broadcast(UObject* ObjectChangingTeam, int32 OldTeamID, int32 NewTeamID);
};
```

> **记住这个画面**：委托在内存里就是**一个数组**，数组的每一项是一条"**谁（对象）+ 它的什么函数（函数指针）**"的记录。AddDynamic 就是往这个数组里加一条，Broadcast 就是遍历数组逐个调用。

---

## 二、`GetTeamChangedDelegateChecked()` —— Lyra 自己写的（真实源码）

这个不是引擎的，是 **Lyra 在 `LyraTeamAgentInterface.h` 第 42~47 行自己写的内联函数**。我拿到了真实源码：

```cpp
// 文件：LyraTeamAgentInterface.h  第 42-47 行（真实源码）
FOnLyraTeamIndexChangedDelegate& GetTeamChangedDelegateChecked()
{
    FOnLyraTeamIndexChangedDelegate* Result = GetOnTeamIndexChangedDelegate();  // ① 去拿委托指针
    check(Result);                                                              // ② 断言：不许为空！
    return *Result;                                                             // ③ 解引用，返回委托本身
}
```

### 逐行拆解

| 行 | 代码 | 在干嘛 |
|----|------|--------|
| ① | `GetOnTeamIndexChangedDelegate()` | 调另一个虚函数，拿到"委托的指针"（可能为空） |
| ② | `check(Result)` | **断言**：如果 Result 是空（nullptr），直接**崩溃报错**——"这里不该为空，空了就是 bug" |
| ③ | `return *Result` | 解引用指针，返回"委托对象本身"（所以后面用 `.` 而不是 `->`） |

### 为什么要包这一层？—— 对比两个版本

接口里其实有**两个**获取委托的方法，一高一低：

```cpp
// 版本 A：裸的（可能返回空）—— 第 38 行
virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() { return nullptr; }

// 版本 B：Checked 包装版 —— 第 42 行
FOnLyraTeamIndexChangedDelegate& GetTeamChangedDelegateChecked() { ... check(Result); ... }
```

| | `GetOnTeamIndexChangedDelegate`（裸） | `GetTeamChangedDelegateChecked`（包装） |
|---|---|---|
| 返回 | 指针 `T*`，**可能为空** | 引用 `T&`，**保证非空** |
| 空了会怎样 | 返回 nullptr，你自己判空 | `check` 直接崩，替你暴露 bug |
| 谁用 | 需要谨慎处理的地方 | "我确定它该有值，放心用"的地方 |

> **设计意图**：Lyra 团队认为"能走到这里，委托一定该存在"，所以用 `check` 把"万一为空"的情况变成**立刻报错**，而不是让空指针悄悄溜过去导致后面莫名其妙崩。这是"**快速失败（fail fast）**"的防御式编程。

### `check()` 是什么？

- UE 的断言宏，Debug/Development 编译下：条件为假 → **触发断言崩溃 + 打印信息**。
- Shipping（发布）编译下：通常被优化掉（不检查），避免影响性能。
- 类比：`check(Result)` = "我这里**断定** Result 不为空，要是空了就是你代码有问题，赶紧崩给我看！"

---

## 三、`AddDynamic()` —— UE 引擎标准实现（**真实源码**，UE5.6）

> ✅ 本节已对照 **UE 5.6 引擎真实源码**更新（路径：`Engine/Source/Runtime/Core/Public/Delegates/`）。不再是"原理还原"。

### 3.1 第一步：`AddDynamic` 其实是个宏

你以为 `AddDynamic(this, &ThisClass::OnControllerChangedTeam)` 是个普通函数调用？**不，它是个宏**。真实定义在 `Delegate.h` 第 439 行：

```cpp
// 文件：Delegate.h  第 439 行（真实源码）
#define AddDynamic( UserObject, FuncName ) \
    __Internal_AddDynamic( UserObject, FuncName, STATIC_FUNCTION_FNAME( TEXT( #FuncName ) ) )
```

**它干了件很聪明的事**：把你写的两个参数，自动补上**第三个隐藏参数——函数名字符串**。

```cpp
// 你写的：
AddDynamic(this, &ThisClass::OnControllerChangedTeam);
//                ↓ 宏把它展开成下面这样（注意多出来的第三个参数）
__Internal_AddDynamic(this, &ThisClass::OnControllerChangedTeam,
                      STATIC_FUNCTION_FNAME(TEXT("OnControllerChangedTeam")));
//                        ↑ 宏用 #FuncName 把函数名"字符串化"了
```

> **关键洞察**：`#FuncName` 是预处理器的"字符串化运算符"——把 `OnControllerChangedTeam` 这个单词变成字符串 `"OnControllerChangedTeam"`。所以 Dynamic 委托最终靠**函数名**去查函数，而不是靠地址。

### 3.2 第二步：`__Internal_AddDynamic` 真正干活

宏展开后调用的真身，在 `DelegateSignatureImpl.inl` 第 1226~1236 行（真实源码）：

```cpp
// 文件：DelegateSignatureImpl.inl  第 1226-1236 行（真实源码）
template< class UserClass >
void __Internal_AddDynamic( UserClass* InUserObject,
                            typename FDelegate::template TMethodPtrResolver<UserClass>::FMethodPtr InMethodPtr,
                            FName InFunctionName )
{
    check( InUserObject != nullptr && InMethodPtr != nullptr );   // ① 参数不能空

    // NOTE: We're not actually storing the incoming method pointer or calling it.
    //       We simply require it for type-safety reasons.
    //       （注意：我们并不保存这个函数指针，也不调用它，只是为类型安全要求它）

    FDelegate NewDelegate;                                        // ② 造一条新记录
    NewDelegate.__Internal_BindDynamic( InUserObject, InMethodPtr, InFunctionName );  // ③ 绑定

    this->Add( NewDelegate );                                     // ④ 塞进数组
}
```

逐句拆解：

| 步骤 | 代码 | 在干嘛 |
|------|------|--------|
| ① | `check(...)` | 对象和函数指针都不能为空 |
| ② | `FDelegate NewDelegate` | 新建一条"联系人记录" |
| ③ | `__Internal_BindDynamic(...)` | 把对象+函数名绑到这条记录上（见 3.3） |
| ④ | `this->Add(NewDelegate)` | 把这条记录加进委托的数组（联系人名单） |

### 3.3 第三步：`__Internal_BindDynamic` —— 真正存东西的地方（**最核心的真相**）

在 `DelegateSignatureImpl.inl` 第 1134~1149 行（真实源码）：

```cpp
// 文件：DelegateSignatureImpl.inl  第 1134-1149 行（真实源码）
template< class UserClass >
void __Internal_BindDynamic( UserClass* InUserObject,
                             typename TMethodPtrResolver<UserClass>::FMethodPtr InMethodPtr,
                             FName InFunctionName )
{
    check( InUserObject != nullptr && InMethodPtr != nullptr );

    // NOTE: We're not actually storing the incoming method pointer or calling it.
    //       We simply require it for type-safety reasons.

    // ① 存"对象"：转成 UObject 指针存起来
    this->Object = Cast<UObject>(InUserObject);

    // ② 存"函数名"：把那个 FName 存起来（关键！不是存地址！）
    this->FunctionName = InFunctionName;

    // ③ 确保绑定成功（如果函数没标 UFUNCTION 或对象快被回收，会报警）
    ensureMsgf(this->IsBound(),
               TEXT("Unable to bind delegate to '%s' ..."), *InFunctionName.ToString());
}
```

### 🔑🔑🔑 最关键的真相：存的是"对象 + 函数名"，不是"函数偏移"！

很多教程（包括旧版讲解）会说"Dynamic 委托存的是函数偏移量"——**这在 UE5.6 里是不准确的**。看真实源码，它存的是：

```
每条委托记录实际存的东西：
  ┌───────────────────────────────┐
  │ Object      → 那个 UObject 指针 │  ← this->Object = Cast<UObject>(InUserObject)
  │ FunctionName → "OnController.. │  ← this->FunctionName = InFunctionName（函数名！）
  └───────────────────────────────┘
            ↑
   注意：并没有存函数指针/偏移！
   源码注释明确写："not actually storing the incoming method pointer"
```

**为什么存函数名就够了？**

因为能用在 Dynamic 委托上的函数，**必须是 `UFUNCTION`**（前面标了 `UFUNCTION()` 宏的函数）。而所有 UFUNCTION 都有**反射信息**（UE 通过 UHT 生成的元数据，记录了"哪个类有哪些 UFUNCTION、叫什么名"）。所以：

```
运行时要调用时：
  拿着 Object（哪个对象）+ FunctionName（哪个函数名）
        ↓
  去 UE 的反射系统里查："这个对象的这个类，名叫 XXX 的 UFUNCTION 在哪？"
        ↓
  找到函数的真正地址，调用它
```

**这才是 "Dynamic" 的真正含义**：运行时靠**反射 + 函数名**动态找到函数，而不是编译期就定死的地址。

### 3.4 这样设计的好处（解释了三个"为什么"）

| 好处 | 原因 |
|------|------|
| **支持蓝图覆盖** | 蓝图也能有同名 UFUNCTION，按名字查就能找到蓝图版本 |
| **支持序列化/存档** | 函数名是字符串，能存盘；函数地址不能存盘 |
| **支持延迟绑定** | 只要名字对，哪怕现在函数还不存在，等反射就绪也能绑上 |
| **类型安全** | 虽然只存名字，但宏要求你传函数指针做"编译期类型检查"（传错了编译就报错） |

> ⚠️ **代价**：按名字查找比直接调地址慢一点（要查反射表），所以高频热点（每帧几千次）别用 Dynamic 委托，用普通 C++ 委托/直接调用。这也是为什么 Lyra 只在"队伍变化"这种**低频事件**上用 AddDynamic。

---

## 四、把整条链串起来（内存视角）

回到你那行代码：

```cpp
ControllerAsTeamProvider->GetTeamChangedDelegateChecked()
    .AddDynamic(this, &ThisClass::OnControllerChangedTeam);
```

在内存里发生的事：

```
① ControllerAsTeamProvider->GetTeamChangedDelegateChecked()
       │
       ├─ GetOnTeamIndexChangedDelegate()  → 拿到委托指针
       ├─ check(非空)                       → 确保没坏
       └─ return *Result                    → 交出委托对象本身（引用）
                │
                ▼
② .AddDynamic(this, &ThisClass::OnControllerChangedTeam)
       │
       ├─ 宏把函数名字符串化："OnControllerChangedTeam"
       ├─ __Internal_AddDynamic → 造一条记录
       ├─ __Internal_BindDynamic → 存 Object=this + FunctionName="OnControllerChangedTeam"
       └─ this->Add() 往委托数组里加这条记录：
              ┌───────────────────────────────────┐
              │ { Object: this(LyraPawn),           │
              │   FunctionName: "OnControllerChangedTeam" }│
              └───────────────────────────────────┘
```

以后老板队伍一变，就会 `Broadcast(...)`，遍历数组，对每条记录拿 `Object + FunctionName` 去**反射系统查函数真身**再调用。

---

## 五、一张图看懂"委托数组"的本质

```
委托对象（响铃器）
┌─────────────────────────────────────────────┐
│  MulticastDelegate（联系人名单 = 一个数组）    │
│                                             │
│   [0] ┌──────────────────────────────┐      │
│       │ Object: LyraPawn(this)        │      │  ← AddDynamic 加进来的
│       │ FunctionName: OnController..  │      │  （存的是对象+函数名）
│       └──────────────────────────────┘      │
│                                             │
│   [1] ┌──────────────────────────────┐      │
│       │ Object: 另一个监听者           │      │  ← 别人也能 AddDynamic
│       │ FunctionName: 它的回调函数名   │      │
│       └──────────────────────────────┘      │
│                                             │
│   Broadcast() → 遍历 [0][1]... 逐个调用       │
└─────────────────────────────────────────────┘
```

---

## 六、常见误区

| 误区 | 正确理解 |
|------|---------|
| "GetTeamChangedDelegateChecked 是引擎的" | ❌ 它是 **Lyra 自己写的内联包装**，引擎只提供了底层的 `GetOnTeamIndexChangedDelegate` 和委托基类 |
| "Checked 只是名字，没区别" | ❌ 它带 `check()` 断言，空了就崩，是"快速失败"设计 |
| "AddDynamic 存的是函数绝对地址/偏移" | ❌（UE5.6 真实源码）存的是 **对象指针 + 函数名（FName）**，靠反射按名查找 |
| "委托就是一个函数指针" | ❌ 委托是"对象+函数名"绑定的封装，运行时靠反射查真身 |
| "check 在任何编译下都会检查" | ❌ Shipping 下通常被优化掉，只有 Debug/Dev 下生效 |

---

## 七、总结

```
GetTeamChangedDelegateChecked()（Lyra 自己写的，真实源码）：
  1. 调 GetOnTeamIndexChangedDelegate() 拿委托指针
  2. check(非空) —— 空了就崩，快速失败
  3. return *Result —— 返回委托对象本身（所以用 . 访问）

AddDynamic()（UE5.6 真实源码）：
  1. 它是个宏！自动把函数名字符串化，补成第三个隐藏参数
  2. __Internal_AddDynamic 造一条记录并塞进数组
  3. __Internal_BindDynamic 真正存：Object(对象) + FunctionName(函数名)
  本质：存"对象指针 + 函数名"，运行时靠反射按名查真身 → 支持蓝图/序列化

委托的本质：
  内存里就是一个数组，每项 = {Object: 谁的对象, FunctionName: 哪个函数名}
  AddDynamic = 加一条记录
  Broadcast  = 遍历数组，逐个用"对象+函数名"查反射调用
  RemoveAll  = 删掉某对象的所有记录
```

**一句话**：`GetTeamChangedDelegateChecked` 负责"**安全拿到那个响铃器**"（空了就崩），`AddDynamic` 负责"**往响铃器的名单里登记一条'我 + 我的回调函数'**"——底层就是一个数组，增删查播都是围绕这个数组打转。

---

## 八、下一步

- [24_委托绑定详解](./24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic.md) — 上层用法回顾
- [05_LyraTeamAgentInterface队伍接口详解](./05_LyraTeamAgentInterface队伍接口详解.md) — 委托从哪定义
- [23_Cast为什么返回指针](./23_Cast为什么返回指针_指针基础详解.md) — 指针与偏移的关系
