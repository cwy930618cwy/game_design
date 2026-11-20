# 02-2-4 — 第 02 课 · 第 2 步【4/4】：建**血量集** `TenkaichiHealthSet.cpp`

> **对应 Lyra**：`Source/LyraGame/AbilitySystem/Attributes/LyraHealthSet.cpp`
>
> **一句话**：写血量集 `.cpp`——构造函数设初值 + 网络复制注册 + `OnRep_Health` 里那句必须的宏。

---

## 一、这一小步要做出什么

新建 `Source/Tenkaichi/AbilitySystem/Attributes/TenkaichiHealthSet.cpp`，实现 3 个函数，让上一文件的 `Health` 真正"活"起来（有初值、能复制、变化有通知）。

---

## 二、先搞懂：为什么 `.cpp` 要写这 3 个函数

`.h` 里只是"声明"，`.cpp` 才是"兑现"。上一文件埋了 3 个坑，这一步填：

**为什么①：构造函数里要设 `Health(100.0f)`？**
属性得有初始值，否则角色一出生血量是未定义的垃圾值。设 100，角色生成就满血 100。

**为什么②：为什么要写 `GetLifetimeReplicatedProps`？**
上一文件用 `ReplicatedUsing = OnRep_Health` 说"Health 要复制"，但 UE 需要你再**显式登记**一次。`DOREPLIFETIME_CONDITION_NOTIFY` 就是那个登记动作。不写它，复制不生效。

> 类比：`.h` 的 `ReplicatedUsing` 是"贴了个'需联网同步'标签"；`.cpp` 的 `GetLifetimeReplicatedProps` 是"去系统后台正式登记这个同步请求"。光贴标签不登记，系统不理会。

**为什么③：`OnRep_Health` 里第一句为什么必须是 `GAMEPLAYATTRIBUTE_REPNOTIFY`？**
客户端收到复制来的新 Health 后，GAS 需要被"通知一声"属性变了，才会触发后续逻辑（UI 刷新、死亡判定）。漏了它，客户端血条不会动。

---

## 三、你要写的代码（对照 Lyra `LyraHealthSet.cpp` 第 21-36、38-57 行）

```cpp
#include "AbilitySystem/Attributes/TenkaichiHealthSet.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiHealthSet)

UTenkaichiHealthSet::UTenkaichiHealthSet()
	: Health(100.0f)   // ① 构造函数设初值
{
}

void UTenkaichiHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ② 登记网络复制
	DOREPLIFETIME_CONDITION_NOTIFY(UTenkaichiHealthSet, Health, COND_None, REPNOTIFY_Always);
}

void UTenkaichiHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	// ③ 这一句必须！通知 GAS 属性已复制
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTenkaichiHealthSet, Health, OldValue);
}
```

**逐段回扣"为什么"**：
- 构造函数 `Health(100.0f)` → 回扣为什么①。
- `GetLifetimeReplicatedProps` + `DOREPLIFETIME_CONDITION_NOTIFY` → 回扣为什么②。
- `OnRep_Health` 里 `GAMEPLAYATTRIBUTE_REPNOTIFY` → 回扣为什么③。

---

## 四、验收

编译通过 = 属性集建完（基类 + 血量集 4 个文件）。此时还读不到值（ASC 还没挂），**第 3 步挂完 ASC 后** `GetHealth()` 才会返回 100。

---

## 五、一句话结论

**血量集 `.cpp` = 构造函数设初值 + `GetLifetimeReplicatedProps` 登记复制 + `OnRep_Xxx` 里必写 `GAMEPLAYATTRIBUTE_REPNOTIFY`。**
