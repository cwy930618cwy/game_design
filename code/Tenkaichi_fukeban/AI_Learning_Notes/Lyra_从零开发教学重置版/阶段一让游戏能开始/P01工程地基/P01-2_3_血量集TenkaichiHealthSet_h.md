# 02-2-3 — 第 02 课 · 第 2 步【3/4】：建**血量集** `TenkaichiHealthSet.h`

> **对应 Lyra**：`Source/LyraGame/AbilitySystem/Attributes/LyraHealthSet.h`
>
> **一句话**：建血量集头文件——继承基类，真正定义 `Health` 属性 + 复制回调 + 用宏生成访问函数。

---

## 一、这一小步要做出什么

新建 `Source/Tenkaichi/AbilitySystem/Attributes/TenkaichiHealthSet.h`，里面放 `Health` 属性（这才是真正装血量的地方）。

---

## 二、先搞懂：为什么血量集要这么写

上一文件是"空底座"，这一文件是"装血量的抽屉"，继承底座。围绕它有 3 个"为什么"：

**为什么①：血量集要继承 `UTenkaichiAttributeSet`（我们的基类），不是直接继承 `UAttributeSet`？**
这样才能吃到基类里的宏和工具函数（`GetWorld` 等）。这也是 Lyra 的层级：`LyraHealthSet : public ULyraAttributeSet`。

**为什么②：属性类型必须是 `FGameplayAttributeData`，不能用 `float`？**
GAS 需要"带登记、能联网同步、能被技能改写"的数值类型。普通 `float` 是便签纸，GAS 认不出。`FGameplayAttributeData` 才是 GAS 认可的标准档案卡。

**为什么③：为什么不手写 `GetHealth()/SetHealth()`，而用 `ATTRIBUTE_ACCESSORS` 宏？**
一个属性要配 4 个函数（拿句柄、读值、改值、初始化）。手写又臭又长。用基类里的宏一行搞定——这就是为什么上一文件要把宏放基类。

**为什么④：为什么要写 `OnRep_Health` 复制回调？**
血量网络复制（服务器改、客户端同步血条）。`ReplicatedUsing = OnRep_Health` 表示：每次 Health 复制变化时自动调 `OnRep_Health`，好让客户端 UI 刷新。

---

## 三、你要写的代码（对照 Lyra `LyraHealthSet.h` 第 40、56-57、74-75 行）

```cpp
#pragma once

#include "AbilitySystem/Attributes/TenkaichiAttributeSet.h"
#include "TenkaichiHealthSet.generated.h"

UCLASS()
class UTenkaichiHealthSet : public UTenkaichiAttributeSet
{
	GENERATED_BODY()

public:
	UTenkaichiHealthSet();

	// 用基类里的宏生成 GetHealth/GetHealthAttribute/SetHealth/InitHealth
	ATTRIBUTE_ACCESSORS(UTenkaichiHealthSet, Health);

protected:
	// 网络复制回调声明
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

private:
	// 属性本体
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Lyra|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Health;
};
```

**逐段回扣"为什么"**：
- `: public UTenkaichiAttributeSet` → 回扣为什么①，继承我们的基类。
- `ATTRIBUTE_ACCESSORS(...)` → 回扣为什么③，用基类的宏。
- `OnRep_Health` 声明 → 回扣为什么④。
- `FGameplayAttributeData Health` → 回扣为什么②，属性必须是这个类型。

> 注：Lyra 的 `Health` 还带 `Meta = (HideFromModifiers)`（普通 GE 改不了，只有 Execution 能改）。教学骨架先省掉，走通后再加。

---

## 四、验收

编译通过 = 这一小步过关。

---

## 五、一句话结论

**血量集 `.h` = 继承我们的基类 + `ATTRIBUTE_ACCESSORS` 宏 + `FGameplayAttributeData Health` + `ReplicatedUsing` 绑 OnRep 回调；这才是真正装血量的文件。**
