# Q05-2 — `GAMEPLAYATTRIBUTE_VALUE_GETTER`：生成"读属性值"的函数

> **所属**：第 02 课 · 第 2 步 → [Q05 总宏 ATTRIBUTE_ACCESSORS](./Q05_第02课第2步_ATTRIBUTE_ACCESSORS宏_哪里用怎么展开.md) 拆 4 个子宏之 ②
>
> **源码**：引擎 `AttributeSet.h` 第 437-441 行
>
> **一句话**：它生成一个 `Get属性()` 函数，返回这个属性的**当前数值**（`float`）——就是"读血量是多少"。

---

## 一、源码（真实定义，`AttributeSet.h:437-441`）

```cpp
#define GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	FORCEINLINE float Get##PropertyName() const \
	{ \
		return PropertyName.GetCurrentValue(); \
	}
```

以 `GAMEPLAYATTRIBUTE_VALUE_GETTER(Health)` 为例，**展开成**：

```cpp
FORCEINLINE float GetHealth() const
{
	return Health.GetCurrentValue();
}
```

---

## 二、逐行拆解

| 代码 | 人话 |
|------|------|
| `FORCEINLINE float GetHealth() const` | 生成一个**内联**函数，返回 `float`，**不修改成员**（`const`） |
| `return Health.GetCurrentValue();` | 返回 `Health` 这个属性数据的**当前值** |

就这么简单——一行，把属性里存的当前值读出来。

---

## 三、里面 3 个关键字（重点）

1. **`FORCEINLINE`** —— 强制**内联**（编译器把函数体直接展开到调用处，省掉函数调用开销）。因为读血量**极其频繁**（每帧、每次扣血都读），必须快。
2. **`const`** —— 这个函数**不修改任何成员**（只是读），标 `const` 让编译器能放心优化、也允许 const 对象调用。
3. **`GetCurrentValue()`** —— 读"**当前值**"。GAS 里属性有"基础值 + GE 修改量"，`GetCurrentValue()` 返回的是**叠加了所有 GE 后的最终值**（比如基础 100 + 某 buff +10 = 110）。

> 剧场类比：`GetHealth()` = 瞄一眼档案卡上"当前血量"那一栏的数字。看一眼就走，不改动任何东西（`const`），而且这个动作要做得极快（`FORCEINLINE`）。

---

## 四、用在哪（真实调用点）

`LyraHealthSet.cpp` 里到处都是，举几个：
```cpp
// 第 46 行：读当前血，算变化量
const float CurrentHealth = GetHealth();

// 第 148 行：读当前血，减去伤害，算新血量
SetHealth(FMath::Clamp(GetHealth() - GetDamage(), MinimumHealth, GetMaxHealth()));

// 第 171 行：判断血量是否变了
if (GetHealth() != HealthBeforeAttributeChange)
```

---

## 五、和 ① `GetHealthAttribute()` 的区别（再强调）

| 函数 | 返回 | 干嘛 |
|------|------|------|
| `GetHealthAttribute()`（①） | `FGameplayAttribute`（句柄） | 给系统指名"改哪个属性" |
| `GetHealth()`（②，本宏） | `float`（数值） | 给人看/参与计算"血量是多少" |

---

## 六、一句话结论

**`GAMEPLAYATTRIBUTE_VALUE_GETTER` 生成 `Get属性()`——一行 `return 属性.GetCurrentValue()` 读出当前数值（叠加所有 GE 后的最终值）。FORCEINLINE 求快（读血极频繁），const 表示只读不改。**
