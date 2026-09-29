# Q07：`FGameplayAttributeData` 是啥？源码在哪？

> 问题：血量集 `.h` 里 `FGameplayAttributeData Health;` 和 `OnRep_Health(const FGameplayAttributeData& OldValue)` 用的 `FGameplayAttributeData` 到底是什么？源码在哪个文件？

---

## 一句话

**`FGameplayAttributeData` 是 GAS 用来"装一个属性数值"的结构体**——每个属性（血量、蓝量、体力）本体都是它。它内部存**两个数**：基础值（BaseValue）和当前值（CurrentValue）。

---

## 源码位置

**文件**：`D:\ue5\Epic Games\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Public\AttributeSet.h`
**行号**：**21-55**

（就是你上一问那 4 个宏所在文件的**最开头**，紧挨着它上面就是它。）

---

## 源码原文（第 21-55 行）

```cpp
struct FGameplayAttributeData
{
	GENERATED_BODY()
	FGameplayAttributeData()
		: BaseValue(0.f)
		, CurrentValue(0.f)
	{}

	FGameplayAttributeData(float DefaultValue)
		: BaseValue(DefaultValue)
		, CurrentValue(DefaultValue)
	{}

	virtual ~FGameplayAttributeData()
	{}

	/** Returns the current value, which includes temporary buffs */
	UE_API float GetCurrentValue() const;

	/** Modifies current value, normally only called by ability system or during initialization */
	UE_API virtual void SetCurrentValue(float NewValue);

	/** Returns the base value which only includes permanent changes */
	UE_API float GetBaseValue() const;

	/** Modifies the permanent base value, normally only called by ability system or during initialization */
	UE_API virtual void SetBaseValue(float NewValue);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	float BaseValue;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	float CurrentValue;
};
```

---

## 拆开看：它就装两个数

| 成员 | 类型 | 含义 |
|------|------|------|
| `BaseValue` | `float` | **基础值**：只算"永久变化"（如升级永久加血上限） |
| `CurrentValue` | `float` | **当前值**：包含"临时 buff/debuff"（如吃了加血药） |

**为什么分两个值？** 因为 GAS 的数值有两层：
- **基础值**：角色的"底子"，永久生效
- **当前值**：基础值 + 各种临时增益减益算出来的"此刻实际值"

> 类比：你的"底薪"（BaseValue）是 1 万，这个月加了奖金（临时 buff），"本月实发"（CurrentValue）是 1 万 5。奖金没了，实发回到 1 万，但底薪没变。

---

## 4 个方法（对应上一问的宏）

| 方法 | 作用 |
|------|------|
| `GetCurrentValue()` | 读当前值（含临时 buff） |
| `SetCurrentValue()` | 改当前值（一般由 ASC 调） |
| `GetBaseValue()` | 读基础值（永久） |
| `SetBaseValue()` | 改基础值（永久） |

**回扣上一问**：`GAMEPLAYATTRIBUTE_VALUE_GETTER(Health)` 展开的 `GetHealth()` 里那句 `return Health.GetCurrentValue();`——就是调这里的 `GetCurrentValue()`。`InitHealth()` 里的 `SetBaseValue/SetCurrentValue`——就是调这里的两个 Set。

---

## 两个构造函数

```cpp
FGameplayAttributeData()              // 默认：BaseValue=0, CurrentValue=0
FGameplayAttributeData(float v)       // 带初值：BaseValue=v, CurrentValue=v
```

所以 `FGameplayAttributeData Health;` 声明时，Health 默认 0/0；真正设初值靠构造函数里调 `InitHealth(100.0f)`（那个宏展开会调 `SetBaseValue/SetCurrentValue`）。

---

## 剧场故事（回扣人物谱）

血型档案卡（属性集）上，每个数值栏（血量/蓝量）都是一张**小数值卡**，这张小卡就是 `FGameplayAttributeData`。

卡正面写两个数：
- **基础值**：演员的"体质底子"（永久）
- **当前值**：此刻实际状态（吃了兴奋剂 +20，就比底子高）

> 类比：档案卡里夹着一张"数值小卡"，小卡上写"底薪 100 / 实发 120"。血量这个属性本体，就是这么一张小卡。

---

## 一句话总结

- **是什么**：GAS 装"一个属性数值"的结构体，每个属性本体都是它。
- **源码**：`AttributeSet.h:21-55`（GAS 引擎，就在 4 个宏的上方）。
- **核心**：内部两个数——`BaseValue`（永久基础值）+ `CurrentValue`（含临时 buff 的当前值），配 4 个读/写方法。
