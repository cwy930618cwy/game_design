# `LyraAttributeSet.h` 速览

> 所有 Lyra 属性集的**基类**。**最有价值的是那个宏和那个委托**。

| 成员 | 干嘛的 |
|---|---|
| `ATTRIBUTE_ACCESSORS(ClassName, PropertyName)` | ⭐ 宏：一行生成 4 个函数（见下） |
| `FLyraAttributeEvent` | ⭐ 6 参数多播委托：Instigator / Causer / EffectSpec / Magnitude / OldValue / NewValue |
| `: UAttributeSet` | 标准继承 |
| `GetWorld()` | 属性集默认拿不到 World，这里通过 Outer 补上 |
| `GetLyraAbilitySystemComponent()` | 拿到强类型的 ASC |

## `ATTRIBUTE_ACCESSORS` 宏生成了什么

```cpp
ATTRIBUTE_ACCESSORS(ULyraHealthSet, Health)
// 展开后等价于：
static FGameplayAttribute GetHealthAttribute();   // 取属性（给 GE 用）
float GetHealth() const;                          // 读值
void  SetHealth(float NewVal);                    // 写值
void  InitHealth(float NewVal);                   // 初始化
```

> 💡 **这是 GAS 里最常用也最容易写错的样板代码**，Lyra 把它收成一个宏，所有属性集都在用。

**优先级**：`ATTRIBUTE_ACCESSORS` 宏 → `FLyraAttributeEvent`
