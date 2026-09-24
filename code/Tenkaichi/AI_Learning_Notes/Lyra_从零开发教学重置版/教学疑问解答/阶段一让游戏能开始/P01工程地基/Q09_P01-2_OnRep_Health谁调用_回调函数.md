# Q09：C++ 定义了函数，谁调它？（以 `OnRep_Health` 为例）

> 问题：`OnRep_Health` 我定义了，但工程里搜不到任何地方调用它。那它到底谁调？C++ 里"定义了没人调"的函数是怎么回事？

---

## 一句话

**`OnRep_Health` 不是你调的，也不是你代码里调的——是引擎在网络复制完成时"自动回调"的。** 这种"你只负责定义、引擎在特定时机自动调用"的函数，叫**回调函数（Callback）**。

---

## 核心概念：回调函数（Callback）

C++ 里有一类函数，**你写了定义，但全工程搜不到调用点**——因为它们不是被你的代码调的，而是被**引擎在某个特定时机自动调**。

`OnRep_Health` 就是典型：

```cpp
void UTenkaichiHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTenkaichiHealthSet, Health, OldValue);
}
```

你搜遍工程，找不到 `OnRep_Health(...)` 的调用——**因为调用它的是引擎，不是你**。

---

## 引擎怎么知道要调它？靠 `.h` 里的"绑定"

回看血量集 `.h` 第 24 行：

```cpp
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, ...)
FGameplayAttributeData Health;
```

关键在 **`ReplicatedUsing = OnRep_Health`** 这一句——它是一句**绑定**：

> "Health 这个属性，一旦复制完成，就调用 `OnRep_Health` 这个函数。"

引擎的网络系统在**客户端收到新的 Health 值时**，看到这句绑定，就**自动调** `OnRep_Health`，把"旧值"作为参数传进来（所以它有个参数 `OldValue`）。

---

## 调用时机（引擎什么时候调）

```
服务器：Health 从 100 变成 80
   ↓ （网络复制）
客户端：收到新的 Health = 80
   ↓ （引擎网络系统发现 .h 里标了 ReplicatedUsing=OnRep_Health）
客户端：引擎自动调用 OnRep_Health(旧值100)   ← 这里！你不用写调用代码
   ↓
你的 OnRep_Health 里：GAMEPLAYATTRIBUTE_REPNOTIFY(...) 通知 GAS"Health 变了"
```

**你从头到尾不用写一行"调用 OnRep_Health"的代码**——引擎帮你调。

---

## C++ 里"定义了谁调"的几种情况

| 情况 | 谁调 | 例子 |
|------|------|------|
| 普通函数 | 你代码里显式调 `Foo()` | `int x = Add(1,2);` |
| **回调函数** | **引擎在特定时机自动调** | `OnRep_Health`（复制完成）、`BeginPlay`（游戏开始） |
| 虚函数重写 | 通过基类指针/引擎调，运行时决定调哪个 | `GetLifetimeReplicatedProps`（引擎复制时调） |
| 构造函数 | 引擎创建对象时自动调 | `UTenkaichiHealthSet()` |

`OnRep_Health` 属于第 2 种——**回调函数**。

> 类比：你不用每天去快递柜取件，你只是**在快递柜登记了"到件发短信给我"**（`.h` 的 `ReplicatedUsing`）。快递一到（复制完成），**系统自动给你发短信**（调 `OnRep_Health`）——你不用自己盯着查快递。

---

## `GAMEPLAYATTRIBUTE_REPNOTIFY` 那句是干嘛

```cpp
GAMEPLAYATTRIBUTE_REPNOTIFY(UTenkaichiHealthSet, Health, OldValue);
```

引擎调 `OnRep_Health` 后，执行到这一句——它通知 GAS："Health 属性在客户端被复制更新了，旧值是 OldValue。" GAS 收到通知后，会触发依赖这个属性的逻辑（比如 UI 血条刷新、依赖 Health 的 GameplayEffect 重算）。

**这行是 GAS 的要求**：属性复制后必须用它通知，否则 GAS 不知道属性变了。

---

## 剧场故事（回扣人物谱）

演员（属性集）的血型档案卡（Health）要同步给全场。

- **`.h` 的 `ReplicatedUsing=OnRep_Health`** = 演员在物业登记："我血型卡一更新，**自动通知我的经纪人**（`OnRep_Health`）。"
- **引擎** = 物业。服务器那边血型卡一改，物业（引擎）广播给全场；**客户端这边一收到新值，物业自动打电话给经纪人**（调 `OnRep_Health`）。
- **`OnRep_Health` 里那句 `GAMEPLAYATTRIBUTE_REPNOTIFY`** = 经纪人接到电话后，立刻跑去告诉 GAS 总管："血型卡更新了！"

演员自己**从头没亲手打过这个电话**——是物业（引擎）在"血型卡到货"那一刻自动打的。

---

## 呼应前面三件套（补全）

上一问 Q08 讲了"复制三件套"，现在把"谁调用"补上：

| 三件套 | 位置 | 谁调用/触发 |
|--------|------|------------|
| `ReplicatedUsing=OnRep_Health` | `.h` | 绑定（登记"复制后调谁"） |
| `GetLifetimeReplicatedProps` + `DOREPLIFETIME` | `.cpp` | **引擎复制时自动调**（登记"要复制"） |
| `OnRep_Health` | `.cpp` | **引擎复制完成后自动回调**（本篇主角） |

---

## 一句话总结

- **谁调 `OnRep_Health`**：引擎。客户端收到复制的新 Health 值时，引擎根据 `.h` 里 `ReplicatedUsing=OnRep_Health` 的绑定**自动调用**它。
- **这类函数叫什么**：回调函数（Callback）——你只定义，引擎在特定时机调。
- **你要做的**：只负责定义 + 在 `.h` 绑定，**不用写调用代码**。

---

## 追加：为什么传旧值不传新值？新旧值怎么区分

**一句话**：新值不用传——它已经自动写进成员变量了；旧值如果不传给你，就被覆盖没了，所以引擎把它当参数传进来。

### 引擎的执行顺序

客户端收到复制的新血量时，引擎是这样处理的：

```
① 引擎先把新值（80）写进成员变量 Health        ← 通过反射自动写
② 然后调用 OnRep_Health(旧值100)              ← 把"即将被覆盖的旧值"传给你
```

所以进入 `OnRep_Health` 的那一刻：

| | 从哪拿 | 值 |
|--|--------|-----|
| **新值** | 成员变量 `Health`（已被 ① 更新） | 80 ← 你调 `GetHealth()` 读到的就是这个 |
| **旧值** | 函数参数 `OldValue` | 100 ← 引擎单独传给你的 |

### 为什么这么设计

- **新值已经在成员变量里了**，你能直接读（`GetHealth()`），没必要再传一遍。
- **旧值如果不传给你，就永远丢了**——因为成员变量已经被新值覆盖。而很多逻辑需要对比"变之前 vs 变之后"（比如"血量从 100 掉到 80"和"从 10 掉到 8"意义完全不同），所以引擎**把即将被覆盖的旧值作为参数塞给你**。

> 类比：你的银行卡余额被改成 80 元（新值已存进卡里）。银行给你发条短信："您余额变动，**变动前是 100 元**"（旧值当参数传给你）。你查卡里余额（成员变量）= 80，短信里（参数）= 100。银行不会在短信里重复"现在是 80"——你自己查卡就知道；但"之前是多少"你不记下来就没了，所以它特意告诉你。

### 在 `OnRep_Health` 内部怎么拿两个值

```cpp
void UTenkaichiHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	// OldValue        → 旧值（参数给的）
	// GetHealth()     → 新值（成员变量已被引擎更新，直接读）
	float NewValue = GetHealth();
	// 现在新旧值都有了，可以对比做逻辑
}
```

**记住**：参数 `OldValue` = 旧值；`GetHealth()` = 新值。
