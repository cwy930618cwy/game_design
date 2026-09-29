# Q08：`GetLifetimeReplicatedProps` 哪来的？

> 问题：血量集 `.cpp` 里重写了 `GetLifetimeReplicatedProps`，这个方法从哪来？属性集又不是 Actor，为什么能有它？

---

## 一句话

**`GetLifetimeReplicatedProps` 是 `UObject` 的方法**（不是 Actor！）。属性集虽然只继承到 `UObject`（`UTenkaichiHealthSet → UAttributeSet → UObject`），但因为 `UObject` 就有这个方法，所以属性集也能重写它，用来**登记"哪些属性要网络复制"**。

---

## 关键澄清：它不是 Actor 专属的

很多人以为"网络复制 = Actor 的事"，其实**复制的根源在 `UObject`**。查引擎源码，这个方法在三个层次都有，但**根定义在 `UObject`**：

| 层次 | 文件 | 行号 | 说明 |
|------|------|------|------|
| **`UObject`（根）** | `Engine\Source\Runtime\CoreUObject\Public\UObject\Object.h` | **1042** | ← **真正的源头**，属性集用的就是它 |
| `UActorComponent` | `Engine\Source\Runtime\Engine\Classes\Components\ActorComponent.h` | 1173 | 组件层 override（组件也是 UObject） |
| `AActor` | `Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h` | 270 | Actor 层 override |

**注意**：`AActor` 和 `UActorComponent` 那两行末尾都是 `override`——说明它们都是**重写 `UObject` 的**那个版本。`UObject::GetLifetimeReplicatedProps` 才是最初的那个。

---

## 属性集的继承链（为什么不继承 Actor 也能用）

```
UTenkaichiHealthSet
    └─ UAttributeSet        （GAS 的基类）
          └─ UObject        （引擎万物基类）← GetLifetimeReplicatedProps 在这里！
```

属性集**不是 Actor**，它只到 `UObject` 这一层。但 `UObject` 本身就有 `GetLifetimeReplicatedProps`，所以属性集照样能重写它。

> 类比：你以为"快递登记"是"公司（Actor）"才能做的事，其实"每个人（UObject）"都能登记快递。属性集是个"独立个人"（UObject），没挂靠公司（Actor），但照样能登记。

---

## 这个方法干嘛用

它是引擎网络复制系统的**回调钩子**：

- **何时被调**：对象开始网络复制时，引擎**自动调用**它（你不用手动调）
- **你要做的**：在里面用 `DOREPLIFETIME` 系列宏，告诉引擎"我这个对象有哪些属性要同步给客户端"

你的代码：

```cpp
void UTenkaichiHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);   // ① 先让父类登记它自己的

	// ② 登记：Health 这个属性要网络复制
	DOREPLIFETIME_CONDITION_NOTIFY(UTenkaichiHealthSet, Health, COND_None, REPNOTIFY_Always);
}
```

- `Super::GetLifetimeReplicatedProps(...)` → 先调父类（`UAttributeSet`）的，让它登记它那一层的复制属性
- `DOREPLIFETIME_CONDITION_NOTIFY(...)` → 再登记自己的：`Health` 要复制，条件 `COND_None`（无条件都发），通知方式 `REPNOTIFY_Always`（每次都触发 `OnRep_Health`）

---

## 呼应：和 `.h` 里两个东西配对

这个函数是**配合**血量集 `.h` 里的两处用的：

1. **`UPROPERTY(..., ReplicatedUsing = OnRep_Health)`**（`.h` 第 24 行）
   → 声明 `Health` 复制时触发 `OnRep_Health` 回调
2. **`OnRep_Health`**（`.h` 第 20 行）
   → 客户端收到新血量后的处理函数

**三件套**：
- `.h` 的 `ReplicatedUsing` → 标"这个属性复制时要回调"
- `.cpp` 的 `GetLifetimeReplicatedProps` → 登记"这个属性要复制"（本篇）
- `.cpp` 的 `OnRep_Health` → 客户端收到后的实际处理

缺一不可：只在 `.h` 标 `ReplicatedUsing` 但不在 `GetLifetimeReplicatedProps` 登记，引擎根本不知道要复制它。

---

## 剧场故事（回扣人物谱）

演员（属性集）身上有张"血型档案卡"（Health）。这卡要**同步给全场记分牌**（网络复制）——服务器改了血量，所有客户端都要知道。

`GetLifetimeReplicatedProps` 就是演员**上台时向场务登记**："我这张血型卡，是要全场同步的，请帮我广播。"

- `Super::GetLifetimeReplicatedProps` → 先让"前辈"（父类）登记他的卡
- `DOREPLIFETIME_CONDITION_NOTIFY` → 再登记"我这张 Health 卡，无条件全发、每次都通知"

> 类比：你搬进公寓，要去物业登记"我家要收快递"（网络复制）。`Super::...` = 先让房东登记他的房间；你的那行 = 再登记"我这个房间要收快递"。不登记，快递员（引擎）根本不知道往你家送。

---

## 一句话总结

- **哪来的**：`UObject` 的方法（`Object.h:1042`），不是 Actor 专属。属性集继承到 `UObject` 就能用。
- **干嘛用**：对象开始复制时引擎自动调它，你在里面用 `DOREPLIFETIME` 登记"哪些属性要同步给客户端"。
- **配对**：和 `.h` 的 `ReplicatedUsing=OnRep_Health`、`.cpp` 的 `OnRep_Health` 组成"复制三件套"。
