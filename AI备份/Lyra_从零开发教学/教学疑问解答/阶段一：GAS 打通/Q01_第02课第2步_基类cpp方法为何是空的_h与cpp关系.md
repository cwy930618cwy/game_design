# Q01 — 第 02 课 · 第 2 步【基类】疑问：构造函数为何是空的？`.h` 和 `.cpp` 到底什么关系？

> **所属章节**：第 02 课 · 第 2 步【1/4】建基类 `TenkaichiAttributeSet.h` +【2/4】建基类 `TenkaichiAttributeSet.cpp`
>
> **对应 Lyra**：`AbilitySystem/Attributes/LyraAttributeSet.h/.cpp`
>
> **一句话**：构造函数空是正常的（基类没属性要初始化）；而 `.h` 声明 + `.cpp` 实现是 C++ 铁律——你之前的理解完全正确，是我给的教学 `.h` 漏了两个函数声明。

---

## 一、你的两个疑问（原话）

1. `UTenkaichiAttributeSet` 方法（构造函数）怎么是空的？
2. 我一直以为 `.h` 定义方法，`.cpp` 才能写 `UTenkaichiAttributeSet::GetWorld()` / `GetTenkaichiAbilitySystemComponent()`。

---

## 二、疑问 1：构造函数为什么是空的

**结论**：空是正常的，因为**基类没有任何属性需要初始化**。

看 Lyra 真实源码 `LyraAttributeSet.cpp` 第 12-14 行：
```cpp
ULyraAttributeSet::ULyraAttributeSet()
{
}
```
**Lyra 的基类构造函数也是空的！** 我们一比一还原，所以也是空的。

**为什么空？** 构造函数是用来"初始化成员变量"的。基类 `UTenkaichiAttributeSet` 里**一个属性都没有**（`Health` 在子类 `TenkaichiHealthSet` 里）——没有成员要初始化，构造函数自然是空的，只写个空壳表示"有这么个构造函数"。

> 类比：基类是"空档案盒底座"，它里面没放任何东西，所以"开箱清点"（构造函数）时没啥可点的，空转一下就完事。真正要清点的是装了血量的抽屉（子类 `TenkaichiHealthSet`），它的构造函数才写 `Health(100.0f)`。

**对比一下**：
| 类 | 构造函数内容 | 为什么 |
|----|-------------|--------|
| 基类 `UTenkaichiAttributeSet` | 空 `{}` | 没有属性要初始化 |
| 子类 `UTenkaichiHealthSet` | `Health(100.0f)` | 要初始化血量初值 |

---

## 三、疑问 2：`.h` 声明 + `.cpp` 实现的关系（你的理解完全正确！）

**结论**：你的理解**100% 正确**——C++ 铁律就是"`.h` 里先声明函数，`.cpp` 里再写实现"。

**问题出在哪？** 出在我的教学 md 给的 `.h` **漏了两个函数的声明**。你照着 md 写的 `.h` 只有构造函数，但 `.cpp` 里却实现了 `GetWorld` 和 `GetTenkaichiAbilitySystemComponent`——`.cpp` 有实现、`.h` 没声明，编译器找不到"这俩函数是谁的"，就会报错。

**正确的完整写法**（`.h` 必须补上这两个声明）：

```cpp
#pragma once

#include "AttributeSet.h"
#include "TenkaichiAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class UTenkaichiAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UTenkaichiAttributeSet();

	// ↓↓↓ 这两行声明，必须和 .cpp 里的实现一一对应！（之前漏了，现在补上）
	UWorld* GetWorld() const override;
	UTenkaichiAbilitySystemComponent* GetTenkaichiAbilitySystemComponent() const;
};
```

**对照关系（`.h` 声明 ↔ `.cpp` 实现）**：

| `.h` 里声明（预告） | `.cpp` 里实现（兑现） |
|--------------------|---------------------|
| `UWorld* GetWorld() const override;` | `UWorld* UTenkaichiAttributeSet::GetWorld() const { ... }` |
| `UTenkaichiAbilitySystemComponent* GetTenkaichiAbilitySystemComponent() const;` | `UTenkaichiAbilitySystemComponent* UTenkaichiAttributeSet::GetTenkaichiAbilitySystemComponent() const { ... }` |

> **规则**：`.h` 里写"有这么个函数"（声明/预告），`.cpp` 里写"这个函数具体怎么干"（实现/兑现）。两者函数名、参数、返回值必须完全一致。`.cpp` 里 `UTenkaichiAttributeSet::` 这个前缀，就是在说"我实现的是 `UTenkaichiAttributeSet` 这个类的成员函数"。

**为什么基类的这两个函数要写在 `.h` 里声明？**
因为它们是**基类的成员函数**，子类要用（继承自基类）。基类成员函数和普通函数一样，都得"`.h` 声明 + `.cpp` 实现"。

> 补充：`GetWorld` 后面带 `override`，是因为它**重写**了父类 `UObject` 的同名虚函数（`UObject` 里已声明了 `virtual GetWorld()`，我们提供自己的版本）。重写必须加 `override` 关键字，让编译器帮你检查"确实重写了父类的函数"。

---

## 四、你现在要做的（修正）

打开 `TenkaichiAttributeSet.h`，在构造函数下面补上这两行声明：
```cpp
	UWorld* GetWorld() const override;
	UTenkaichiAbilitySystemComponent* GetTenkaichiAbilitySystemComponent() const;
```

补完 `.h` 和 `.cpp` 就能对上了，编译应能通过（前提是 `TenkaichiAbilitySystemComponent` 已存在，否则先按 `02_2_2` md 的说明临时注释掉 ASC 那个函数）。

---

## 五、一句话结论

**① 基类构造函数空是正常的（没属性要初始化，Lyra 基类也是空的）；② 你的".h 声明 + .cpp 实现"理解完全正确，是我给的教学 `.h` 漏了两个函数声明，补上即可。**
