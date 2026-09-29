# Q05 — 第 02 课 · 第 2 步【基类】疑问：`ATTRIBUTE_ACCESSORS` 宏是干嘛的？哪里用到、怎么展开？

> **所属章节**：第 02 课 · 第 2 步【1/4】建基类 `TenkaichiAttributeSet.h`
>
> **对应 Lyra**：`LyraAttributeSet.h:29-33`（宏定义） + `LyraHealthSet.h:40-43`（真实用法） + `LyraHealthSet.cpp`（生成的函数被调用处）
>
> **一句话**：`ATTRIBUTE_ACCESSORS(类, 属性)` 是一个"**一键生成 4 个访问函数**"的便利宏——你写一行，它帮你生成 `Get属性Attribute()` / `Get属性()` / `Set属性()` / `Init属性()` 四个函数，省得每个属性手写四遍。

---

## 一、你的疑问（原话）

```cpp
// TenkaichiAttributeSet.h 第 11-16 行
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
```
- 这个宏干嘛的？
- 哪里用到它？怎么用的？

---

## 二、宏本身：一个"一键生成 4 个函数"的模具

`ATTRIBUTE_ACCESSORS` 是个**宏**（`#define`），本质是**一个模具**：你给它"类名 + 属性名"两个参数，它一次性帮你生成 **4 个访问函数**。

Lyra 官方注释（`LyraAttributeSet.h` 第 18-27 行）写得很清楚：

```cpp
/**
 * The following example of the macro:
 *		ATTRIBUTE_ACCESSORS(ULyraHealthSet, Health)
 * will create the following functions:
 *		static FGameplayAttribute GetHealthAttribute();   ← ① 拿"属性句柄"
 *		float GetHealth() const;                          ← ② 读血量值
 *		void SetHealth(float NewVal);                     ← ③ 改血量（走 GE 通道）
 *		void InitHealth(float NewVal);                    ← ④ 初始化血量（直接设基础值）
 */
```

**所以这个宏的作用 = 省打字**。没有它，你每个属性都要手写这 4 个函数；有了它，一行搞定。

---

## 三、4 个生成的函数分别干嘛（重点区分 ②③④）

以 `ATTRIBUTE_ACCESSORS(ULyraHealthSet, Health)` 为例，生成：

| 函数 | 返回/参数 | 干嘛 | 关键区别 |
|------|----------|------|---------|
| ① `GetHealthAttribute()` | 返回 `FGameplayAttribute` | 拿这个属性的"**句柄/身份证号**" | 给 GE、ASC 用来"指名要改哪个属性" |
| ② `GetHealth()` | 返回 `float` | **读**当前血量值 | 只读，纯取值 |
| ③ `SetHealth(新值)` | 参数 `float` | **改**血量 | **走 GE 通道**，会触发回调（`PostAttributeChange` 等） |
| ④ `InitHealth(新值)` | 参数 `float` | **初始化**血量基础值 | **绕过 GE**，直接设，不触发回调 |

> **最容易混的是 ③ vs ④**：
> - `SetHealth` = "正常改血"（比如扣血），必须走 GE，会触发一堆回调（扣血→广播掉血事件）。
> - `InitHealth` = "开局设初值"，直接设基础值，不走 GE、不触发回调（开局初始化用，干净）。

---

## 四、哪里用到它？—— 在"子类"里给每个属性用一次

**宏本身定义在基类**（`LyraAttributeSet.h`），但**真正使用它的是子类**（血量集 `LyraHealthSet`）。

**哪里用**：`LyraHealthSet.h` 第 40-43 行

```cpp
// LyraHealthSet.h 第 40-43 行（血量集给它的 4 个属性各用一次宏）
ATTRIBUTE_ACCESSORS(ULyraHealthSet, Health);      // 生成 GetHealthAttribute/GetHealth/SetHealth/InitHealth
ATTRIBUTE_ACCESSORS(ULyraHealthSet, MaxHealth);   // 生成 ...MaxHealth 那套
ATTRIBUTE_ACCESSORS(ULyraHealthSet, Healing);     // 生成 ...Healing 那套
ATTRIBUTE_ACCESSORS(ULyraHealthSet, Damage);      // 生成 ...Damage 那套
```

**怎么用**：血量集有 4 个属性（血量/上限/治疗/伤害），每个属性都写一行 `ATTRIBUTE_ACCESSORS(类, 属性)`，一次性把 4×4=16 个函数全生成出来。

> 我们项目同理：`TenkaichiAttributeSet.h` 里先定义这个宏（放基类，方便所有子类用），等后面写血量集 `TenkaichiHealthSet` 时，就在血量集里写 `ATTRIBUTE_ACCESSORS(UTenkaichiHealthSet, Health)`。

---

## 五、生成的函数"在哪里被真正调用"（真实调用点）

宏生成的那些函数，最终在 `LyraHealthSet.cpp` 和 `LyraHealthComponent.cpp` 里被大量调用。举几个真实的：

### `GetHealth()`（② 读血量）—— 到处都是

```cpp
// LyraHealthSet.cpp 第 46 行（OnRep_Health 里读当前血）
const float CurrentHealth = GetHealth();

// LyraHealthSet.cpp 第 148 行（扣血：当前血 - 伤害）
SetHealth(FMath::Clamp(GetHealth() - GetDamage(), MinimumHealth, GetMaxHealth()));
```

### `SetHealth()`（③ 改血，走 GE 通道）—— 扣血/回血时

```cpp
// LyraHealthSet.cpp 第 148 行（把伤害转成扣血）
SetHealth(FMath::Clamp(GetHealth() - GetDamage(), MinimumHealth, GetMaxHealth()));

// LyraHealthSet.cpp 第 154 行（把治疗转成加血）
SetHealth(FMath::Clamp(GetHealth() + GetHealing(), MinimumHealth, GetMaxHealth()));
```

### `GetHealthAttribute()`（① 拿属性句柄）—— 给 ASC/GE 指名用

```cpp
// LyraHealthComponent.cpp 第 83 行（初始化时，告诉 ASC"我要设的是 Health 这个属性"）
AbilitySystemComponent->SetNumericAttributeBase(ULyraHealthSet::GetHealthAttribute(), HealthSet->GetMaxHealth());

// LyraHealthSet.cpp 第 211 行（上限降低时，让 ASC 改 Health 属性）
LyraASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
```

> **注意**：`GetHealthAttribute()`（① 句柄）和 `GetHealth()`（② 值）别混——前者返回"属性身份证"（给系统指名用），后者返回"血量数字"（给人看/算用）。

---

## 六、一张总表（宏 → 展开 → 谁用 → 在哪调）

| 层次 | 内容 | 位置 |
|------|------|------|
| **宏定义** | `#define ATTRIBUTE_ACCESSORS(...)` | `LyraAttributeSet.h:29`（基类） |
| **宏使用** | `ATTRIBUTE_ACCESSORS(ULyraHealthSet, Health)` ×4 | `LyraHealthSet.h:40-43`（子类） |
| **展开成** | `GetHealthAttribute` / `GetHealth` / `SetHealth` / `InitHealth` | 编译期自动生成 |
| **被调用** | `GetHealth()` 读血、`SetHealth()` 扣血/回血、`GetHealthAttribute()` 给 ASC 指名 | `LyraHealthSet.cpp` / `LyraHealthComponent.cpp` |

---

## 七、为什么放基类里定义（而不是每个子类各写一份）

因为**所有属性集（血量/蓝量/体力……）都要用这个宏**。放在基类 `LyraAttributeSet`（我们叫 `TenkaichiAttributeSet`）里定义一次，**所有子类直接拿来用**，不用每个子类重复写宏定义。

> 呼应 [Q04c](./Q04c_第02课第2步_空构造函数为什么还要写.md) 的思路：基类就是"给子类打样/备工具"的地方——构造函数占个位，宏也是"备好的工具"，等子类上场直接用。

---

## 八、一句话结论

**`ATTRIBUTE_ACCESSORS(类, 属性)` 是"一键生成 4 个访问函数"的便利宏：`Get属性Attribute()`（拿句柄给系统指名）、`Get属性()`（读值）、`Set属性()`（走 GE 改值、触发回调）、`Init属性()`（绕过 GE 设初值）。宏定义在基类（`LyraAttributeSet.h:29`），在子类给每个属性用一次（`LyraHealthSet.h:40`），生成的函数在 `LyraHealthSet.cpp` 里被大量调用（读血/扣血/给 ASC 指名）。**
