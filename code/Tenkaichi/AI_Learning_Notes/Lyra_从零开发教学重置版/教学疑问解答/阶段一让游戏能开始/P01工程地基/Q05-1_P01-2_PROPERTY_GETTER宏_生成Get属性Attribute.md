# Q05-1 — `GAMEPLAYATTRIBUTE_PROPERTY_GETTER`：生成"拿属性句柄"的函数

> **所属**：第 02 课 · 第 2 步 → [Q05 总宏 ATTRIBUTE_ACCESSORS](./Q05_第02课第2步_ATTRIBUTE_ACCESSORS宏_哪里用怎么展开.md) 拆 4 个子宏之 ①
>
> **源码**：引擎 `D:\ue5\Epic Games\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\...\Public\AttributeSet.h` 第 430-435 行
>
> **一句话**：它生成一个 `Get属性Attribute()` 函数，返回这个属性的"**句柄/身份证号**"（`FGameplayAttribute`），供 ASC、GE 用来"指名要操作哪个属性"。

---

## 一、源码（真实定义，`AttributeSet.h:430-435`）

```cpp
#define GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	static FGameplayAttribute Get##PropertyName##Attribute() \
	{ \
		static FProperty* Prop = FindFieldChecked<FProperty>(ClassName::StaticClass(), GET_MEMBER_NAME_CHECKED(ClassName, PropertyName)); \
		return Prop; \
	}
```

以 `GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ULyraHealthSet, Health)` 为例，**展开成**：

```cpp
static FGameplayAttribute GetHealthAttribute()
{
	static FProperty* Prop = FindFieldChecked<FProperty>(ULyraHealthSet::StaticClass(), GET_MEMBER_NAME_CHECKED(ULyraHealthSet, Health));
	return Prop;
}
```

---

## 二、逐行拆解

| 代码 | 人话 |
|------|------|
| `static FGameplayAttribute GetHealthAttribute()` | 生成一个**静态**函数，返回 `FGameplayAttribute`（属性的"句柄"） |
| `FindFieldChecked<FProperty>(...)` | 用 UE **反射**，按名字找到 `Health` 这个属性对应的 `FProperty`（引擎内部的属性描述） |
| `GET_MEMBER_NAME_CHECKED(ULyraHealthSet, Health)` | 安全地取"成员名 Health"（编译期检查，写错名字会报错） |
| `return Prop` | 把这个属性包成 `FGameplayAttribute` 返回 |
| `static`（函数内） | 这个 `Prop` **只算一次**，之后缓存复用（反射查找有开销，不每次重找） |

---

## 三、里面 3 个关键字（重点）

1. **`FGameplayAttribute`** —— 属性的"**句柄/身份证号**"。它不是血量值，而是"指向 Health 这个属性本身"的引用。ASC/GE 改属性时，必须拿它来指名"我要改的是 Health"。
2. **`FProperty` + `FindFieldChecked`** —— UE **反射**机制。运行时按名字找到属性。`Checked` = 找不到就直接报错（防止拼错属性名）。
3. **`static`** —— 查找结果**缓存一次**，之后调用直接返回同一个，不重复反射（性能）。

> 剧场类比：`GetHealthAttribute()` = 档案卡（属性集）拿出自己的"**身份证号**"。要办事（改血量）时，得先报出身份证号，办事窗口（ASC）才知道"你要办的是 Health 这项"。

---

## 四、用在哪（真实调用点）

- `LyraHealthComponent.cpp:83`：初始化时告诉 ASC"我要设的是 Health"
  ```cpp
  AbilitySystemComponent->SetNumericAttributeBase(ULyraHealthSet::GetHealthAttribute(), ...);
  ```
- `LyraHealthSet.cpp:211`：上限降低时，让 ASC 改 Health
  ```cpp
  LyraASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
  ```

> 注意：`GetHealthAttribute()`（拿句柄）和 `GetHealth()`（拿值）是**两个不同函数**，别混——前者给系统指名，后者给人看数字。

---

## 五、一句话结论

**`GAMEPLAYATTRIBUTE_PROPERTY_GETTER` 生成 `Get属性Attribute()`——用反射拿到这个属性的"句柄"（`FGameplayAttribute`），供 ASC/GE 指名"要改哪个属性"。static 缓存查找结果，`FindFieldChecked` 保证名字没拼错。**
