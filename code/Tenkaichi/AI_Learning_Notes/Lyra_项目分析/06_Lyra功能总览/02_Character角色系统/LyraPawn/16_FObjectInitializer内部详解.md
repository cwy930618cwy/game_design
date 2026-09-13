# 16 — FObjectInitializer 内部详解（真实源码）

> **定位**：深入 `FObjectInitializer` 内部，看它**到底有什么方法**，每个是干嘛的。
>
> **一句话**：`FObjectInitializer` 主要提供**创建子对象（组件）的方法**——`CreateDefaultSubobject`（必须创建）、`CreateOptionalDefaultSubobject`（可选创建）、`SetDefaultSubobjectClass`（换类）等。核心就是"**在构造函数里造组件**"。
>
> **文件**：`CoreUObject/Public/UObject/UObjectGlobals.h`

---

## 一、FObjectInitializer 里有什么（核心方法）

从真实源码看，`FObjectInitializer` 提供的方法，**几乎都是"创建/设置子对象（组件）"**：

| 方法 | 干嘛 | 常用度 |
|------|------|:---:|
| `CreateDefaultSubobject()` | **创建必须的组件** | ⭐⭐⭐ 最常用 |
| `CreateOptionalDefaultSubobject()` | 创建**可选**组件 | ⭐⭐ |
| `SetDefaultSubobjectClass()` | 设置子对象的**类** | ⭐ |
| `DoNotCreateDefaultSubobject()` | **不创建**某组件 | ⭐ |

**核心**：FObjectInitializer = "创建组件的工具箱"，主要就是 `CreateDefaultSubobject`。

---

## 二、CreateDefaultSubobject —— 创建必须的组件（最常用）

**真实源码**（UObjectGlobals.h 第 1331 行）：

```cpp
// 真实源码
template <class TReturnType>
TReturnType* CreateDefaultSubobject(UObject* Outer, FName SubobjectName, bool bTransient = false) const
{
    UClass* ReturnType = TReturnType::StaticClass();
    return static_cast<TReturnType*>(
        CreateDefaultSubobject(Outer, SubobjectName, ReturnType, ReturnType, /*bIsRequired =*/ true, bTransient)
    );
}
```

**参数**：
| 参数 | 含义 |
|------|------|
| `Outer` | 谁创建它（一般是 this） |
| `SubobjectName` | 组件的名字 |
| `bTransient` | 是否临时（默认 false） |

**用途**：创建**必须存在**的组件（如根组件、模型组件）。

```cpp
// 用法：创建根组件
RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
```

**`bIsRequired = true`**：这个组件"必须创建"，就算蓝图说"不要"，也强制创建。

---

## 三、CreateOptionalDefaultSubobject —— 创建可选组件

**真实源码**（第 1346 行）：

```cpp
// 真实源码
template <class TReturnType>
TReturnType* CreateOptionalDefaultSubobject(UObject* Outer, FName SubobjectName, bool bTransient = false) const
{
    UClass* ReturnType = TReturnType::StaticClass();
    return static_cast<TReturnType*>(
        CreateDefaultSubobject(Outer, SubobjectName, ReturnType, ReturnType, /*bIsRequired =*/ false, bTransient)
    );
}
```

**和 CreateDefaultSubobject 的区别**：
- `CreateDefaultSubobject` → **bIsRequired = true**（必须创建）
- `CreateOptionalDefaultSubobject` → **bIsRequired = false**（可选创建，可用 `DoNotCreateDefaultSubobject` 跳过）

**用途**：创建"可选的组件"（不需要时可以不创建）。

```cpp
// 用法：创建可选组件
// 如果需要，就创建；如果蓝图说不要，就不创建
MyOptionalComp = ObjectInitializer.CreateOptionalDefaultSubobject<UMyComp>(this, TEXT("Optional"));
```

---

## 四、DoNotCreateDefaultSubobject —— 不创建某组件

**真实源码**（第 1426 行）：

```cpp
// 真实源码
const FObjectInitializer& DoNotCreateDefaultSubobject(FName SubobjectName) const
{
    AssertIfSubobjectSetupIsNotAllowed(SubobjectName);
    // 标记：不要创建这个组件
}
```

**用途**：标记"这个组件不要创建"——用于**子类想去掉父类创建的某组件**。

```cpp
// 父类创建了 Mesh 组件
// 子类不想要，用 DoNotCreateDefaultSubobject 标记去掉
```

**配 CreateOptionalDefaultSubobject 用**：
- 父类用 `CreateOptionalDefaultSubobject` 创建可选组件
- 子类用 `DoNotCreateDefaultSubobject` 标记不要

---

## 五、SetDefaultSubobjectClass —— 设置子对象的类

**真实源码**（第 1434 行附近）：

```cpp
// 真实源码
// 设置子对象的类（用哪个类来创建）
void SetDefaultSubobjectClass(FName SubobjectName, const UClass* Class);
```

**用途**：把某个组件**换成子类**来创建。

```cpp
// 父类用基础类创建组件
// 子类想用更具体的子类替换
ObjectInitializer.SetDefaultSubobjectClass<UMySpecialComponent>(TEXT("Mesh"));
```

---

## 六、FObjectInitializer 内部总结

```
FObjectInitializer（创建组件的工具箱）：
  CreateDefaultSubobject()      ← 创建必须组件（最常用）
  CreateOptionalDefaultSubobject() ← 创建可选组件
  DoNotCreateDefaultSubobject() ← 标记不创建
  SetDefaultSubobjectClass()    ← 换子对象的类

核心作用：在构造函数里创建组件
```

**一句话（看源码后）**：`FObjectInitializer` 内部主要提供**创建组件的方法**——`CreateDefaultSubobject`（必须创建，最常用）、`CreateOptionalDefaultSubobject`（可选）、`DoNotCreateDefaultSubobject`（不创建）、`SetDefaultSubobjectClass`（换类）。**它就是"在构造函数里造组件的工具箱"，核心是 `CreateDefaultSubobject`。**

---

## 七、下一步

理解了 FObjectInitializer 内部，下一步可以看 **`ALyraPawn` 的 .cpp 里实际怎么用它创建组件**，或深入 UE 对象构造流程。
