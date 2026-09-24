# Q05-4 — `GAMEPLAYATTRIBUTE_VALUE_INITTER`：生成"初始化属性"的函数（直接设，不走 ASC）

> **所属**：第 02 课 · 第 2 步 → [Q05 总宏 ATTRIBUTE_ACCESSORS](./Q05_第02课第2步_ATTRIBUTE_ACCESSORS宏_哪里用怎么展开.md) 拆 4 个子宏之 ④
>
> **源码**：引擎 `AttributeSet.h` 第 453-458 行
>
> **一句话**：它生成一个 `Init属性()` 函数，**直接设属性的基础值和当前值**——开局初始化用，**不走 ASC、不触发回调**，干净利落。

---

## 一、源码（真实定义，`AttributeSet.h:453-458`）

```cpp
#define GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName) \
	FORCEINLINE void Init##PropertyName(float NewVal) \
	{ \
		PropertyName.SetBaseValue(NewVal); \
		PropertyName.SetCurrentValue(NewVal); \
	}
```

以 `GAMEPLAYATTRIBUTE_VALUE_INITTER(Health)` 为例，**展开成**：

```cpp
FORCEINLINE void InitHealth(float NewVal)
{
	Health.SetBaseValue(NewVal);      // ① 设基础值
	Health.SetCurrentValue(NewVal);   // ② 设当前值
}
```

---

## 二、逐行拆解

| 代码 | 人话 |
|------|------|
| `Health.SetBaseValue(NewVal)` | 直接设 `Health` 的**基础值** |
| `Health.SetCurrentValue(NewVal)` | 直接设 `Health` 的**当前值** |

**注意**：这里**没有 `GetOwningAbilitySystemComponent()`、没有 ASC、没有回调**——就是直接对属性数据本身设值。

---

## 三、核心：`Init` vs `Set` 的关键区别（最容易混）★

| | `SetHealth()`（③） | `InitHealth()`（④，本宏） |
|--|-------------------|--------------------------|
| 走 ASC 吗 | ✅ 走（`SetNumericAttributeBase`） | ❌ 不走，直接设 |
| 触发回调吗 | ✅ 触发（`PostAttributeChange` 等） | ❌ 不触发 |
| 什么时候用 | 游戏运行中**正常改血**（扣血/回血） | **开局初始化**设初值 |
| 为什么 | 改血要通知全系统（掉血事件、死亡检查） | 开局设初值不该触发一堆回调，要干净 |

> **为什么初始化要绕开 ASC？** 因为开局设初值时，很多系统（ASC、回调）可能**还没初始化好**；而且"设初值"本身不需要触发"血量变了"这类事件（不然开局就误报一次掉血）。所以 `Init` 直接设，干净。

> 剧场类比：`SetHealth` = 演到一半改血量，必须走总管（ASC）按流程通知全场。`InitHealth` = 演出还没开始，工作人员**直接在档案卡上填好"初始血量 100"**——这时候没有观众、不用广播，填完就行。

---

## 四、里面 2 个关键字（重点）

1. **`SetBaseValue`** —— 设"**基础值**"（没加任何 GE 修改的原始值）。
2. **`SetCurrentValue`** —— 设"**当前值**"（= 基础值 + GE 修改量；初始化时两者都设成初值）。

> GAS 里属性有"基础值 / 当前值"两层：当前值 = 基础值叠加所有 GE。初始化时把两者都设成初值，保证干净起点。

---

## 五、用在哪

`InitHealth` 通常在**初始化属性集**时调用（比如 `InitStats`、或 ASC 初始化属性那套流程里），给血量一个起始值。

> 对比 [Q04c](./Q04c_第02课第2步_空构造函数为什么还要写.md)：构造函数里写 `Health(100.0f)` 是"造对象时设初值"的**另一种方式**（C++ 构造初始化）；`InitHealth` 是"运行中用函数设初值"的方式。两者目的相同（设初值），时机/手段不同。

---

## 六、一句话结论

**`GAMEPLAYATTRIBUTE_VALUE_INITTER` 生成 `Init属性()`——直接 `SetBaseValue`+`SetCurrentValue` 设初值，不走 ASC、不触发回调，用于开局初始化。与 ③`Set`（走 ASC、触发回调、运行中改值）形成对照：初始化用 Init，正常改值用 Set。**
