# 02-2-1 — 第 02 课 · 第 2 步【1/4】：建**基类** `TenkaichiAttributeSet.h`

> **对应 Lyra**：`Source/LyraGame/AbilitySystem/Attributes/LyraAttributeSet.h`
>
> **一句话**：建属性集的**基类**头文件——放便利宏 + 一个空的基类壳子。**基类不含任何属性**（属性留给下一文件 `TenkaichiHealthSet`）。

---

## 一、这一小步要做出什么

新建 `Source/Tenkaichi/AbilitySystem/Attributes/TenkaichiAttributeSet.h`，里面只有一个宏 + 一个空基类 + 构造函数。编译能过即过关。

---

## 二、先搞懂：为什么先建"基类"，而且基类里不放属性

**为什么①：为什么属性集要分"基类 + 血量集"两个文件？**
GAS 项目里数值很多（血量、蓝量、体力、攻击力……）。Lyra 的做法是：**抽一个所有数值集的公共基类** `LyraAttributeSet`，把"通用的东西"（便利宏、工具函数）放这里；再为每类数值建子类（`LyraHealthSet` 管血量、`LyraCombatSet` 管战斗）。

> 类比：基类是"档案盒的通用底座"，规定"这是个 GAS 档案盒"；血量集是"专门装血量的抽屉"，插进底座里。这样以后加蓝量、体力时，各建一个抽屉就行，不用重写底座。

**为什么②：为什么基类里不放 `Health`？**
因为基类是"公共底座"，它不知道也不该知道自己装的是血量还是蓝量。`Health` 是"血量集"专属的，必须放到 `TenkaichiHealthSet`（下一文件）。**这一步只搭底座，别把血量塞进来。**

**为什么③：为什么要把 `ATTRIBUTE_ACCESSORS` 宏放在基类里？**
因为每个子类（血量集、蓝量集……）都要用这个宏生成访问函数。放在基类里，所有子类自动共享。这是 Lyra 的真实做法（宏就在 `LyraAttributeSet.h`）。

---

## 三、你要写的代码（对照 Lyra `LyraAttributeSet.h`）

```cpp
#pragma once

#include "AttributeSet.h"
#include "TenkaichiAttributeSet.generated.h"

// 前向声明（对应 Lyra LyraAttributeSet.h 第 11-15 行）：
// .h 里只用到这两个类型的指针做返回值，不需要完整定义，前向声明即可，避免循环 include
class UWorld;
class UTenkaichiAbilitySystemComponent;

// 便利宏：每个子类都要用它生成访问函数，所以放在基类里
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

	// 两个工具函数声明（下一文件 .cpp 里实现，这里必须先声明）
	UWorld* GetWorld() const override;
	UTenkaichiAbilitySystemComponent* GetTenkaichiAbilitySystemComponent() const;
};
```

> ⚠️ **两处必须齐全（对照 Lyra 核对）**：
> 1. **前向声明** `class UWorld;` / `class UTenkaichiAbilitySystemComponent;` —— 因为 `.h` 里用它们做指针返回值，缺了编译器不认识（Lyra 的 `LyraAttributeSet.h` 第 11-15 行就有这些前向声明）。
> 2. **`.h` 声明 ↔ `.cpp` 实现一一对应** —— `.h` 声明了 `GetWorld` / `GetTenkaichiAbilitySystemComponent`，`.cpp` 才能写实现。（详见 [教学疑问解答/Q01](../教学疑问解答/Q01_第02课第2步_基类cpp方法为何是空的_h与cpp关系.md)）

**逐段回扣"为什么"**：
- `#include "AttributeSet.h"` → 因为要继承 GAS 基类 `UAttributeSet`。
- `class UWorld;` / `class UTenkaichiAbilitySystemComponent;` 前向声明 → 对应 Lyra 第 11-15 行；`.h` 里只用到指针，前向声明即可，不必完整 include（避免循环依赖）。
- `ATTRIBUTE_ACCESSORS` 宏 → 回扣为什么③，放基类里供所有子类共享。
- `class UTenkaichiAttributeSet : public UAttributeSet` → 这就是"属性集基类"。
- `GetWorld` / `GetTenkaichiAbilitySystemComponent` 声明 → 基类工具函数，供子类复用；`GetWorld` 带 `override` 因为重写父类 `UObject` 的虚函数。
- **注意：这里没有 `Health`** → 回扣为什么②，属性留给下一文件的血量集。

---

## 四、验收

编译通过 = 这一小步过关。

---

## 五、一句话结论

**基类 `.h` = 便利宏（放基类共享）+ 一个继承 `UAttributeSet` 的空壳；基类不放任何属性，属性归各子类。**
