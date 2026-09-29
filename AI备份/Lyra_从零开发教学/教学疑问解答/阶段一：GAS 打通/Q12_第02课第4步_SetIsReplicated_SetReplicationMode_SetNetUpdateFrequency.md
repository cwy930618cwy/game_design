# Q12 — 第 02 课第 4 步：`SetIsReplicated` / `SetReplicationMode` / `SetNetUpdateFrequency` 都是干嘛的，源码在哪

> **对应源码**（都实际打开核对过）：
> - `SetIsReplicated` → `Engine/Source/Runtime/Engine/Classes/Components/ActorComponent.h` 第 585-587 行
> - `SetReplicationMode` + `EGameplayEffectReplicationMode` → `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AbilitySystemComponent.h` 第 81-89 行、第 259 行
> - `SetNetUpdateFrequency` → `Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h` 第 4572-4575 行
>
> **一句话**：这三行是给"自带 ASC 的角色"配**网络复制**——`SetIsReplicated` 打开 ASC 的复制开关，`SetReplicationMode(Mixed)` 设复制模式（对拥有者给全量、给别人给精简），`SetNetUpdateFrequency(100)` 把角色（Actor）的复制检查频率提到每秒 100 次。前两个作用在 **ASC（组件）** 上，第三个作用在 **角色（Actor）** 上。

---

## 一、问题是什么

`TenkaichiCharacterWithAbilities.cpp` 构造函数里有三行：

```68:76:Source/Tenkaichi/Character/TenkaichiCharacterWithAbilities.cpp
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UTenkaichiAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	// ...
	SetNetUpdateFrequency(100.0f);
```

你问：**这三个 `Set...` 都是干嘛的？源码在哪？**

---

## 二、逐个讲（都是干嘛 + 源码位置）

### ① `SetIsReplicated(true)` —— 打开 ASC 的"复制开关"

- **作用对象**：`AbilitySystemComponent`（它是一个 **组件** `UActorComponent`）。
- **干嘛**：开启/关闭这个组件的网络复制。传 `true` = 这个组件的状态会同步到客户端。
- **源码**：`ActorComponent.h` 第 585-587 行

```585:587:Engine/Source/Runtime/Engine/Classes/Components/ActorComponent.h
	/** Enable or disable replication. This is the equivalent of RemoteRole for actors (only a bool is required for components) */
	UFUNCTION(BlueprintCallable, Category="Components")
	ENGINE_API void SetIsReplicated(bool ShouldReplicate);
```

> 注意：它是 **`UActorComponent`** 的函数（不是 Actor 的）。因为 ASC 本身是个 ActorComponent，所以能调它。注释说它"等价于 Actor 的 RemoteRole"——组件只需一个 bool 就能开关复制。

### ② `SetReplicationMode(EGameplayEffectReplicationMode::Mixed)` —— 设 ASC 的复制"精细度"

- **作用对象**：`AbilitySystemComponent`（ASC 自己的函数）。
- **干嘛**：控制 **GameplayEffect（GE）** 复制给客户端时的信息量。`Mixed` = **对拥有者（Owner/自治代理）给全量信息，对其他玩家（模拟代理）只给精简信息**。
- **源码**：`AbilitySystemComponent.h` 第 259 行（函数）+ 第 81-89 行（枚举）

```81:89:Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AbilitySystemComponent.h
enum class EGameplayEffectReplicationMode : uint8
{
	/** Only replicate minimal gameplay effect info. Note: this does not work for Owned AbilitySystemComponents (Use Mixed instead). */
	Minimal,
	/** Only replicate minimal gameplay effect info to simulated proxies but full info to owners and autonomous proxies */
	Mixed,
	/** Replicate full gameplay info to all */
	Full,
};
```

> 为什么 Lyra 选 `Mixed`：GE 里可能有"只有自己该看到的私有数据"（如你的技能冷却、隐藏属性）。`Mixed` 让**本人看到全部、别人看到精简版**，既省带宽又保隐私。枚举注释也提示：Owned 的 ASC 不能用 `Minimal`，要用 `Mixed`。

### ③ `SetNetUpdateFrequency(100.0f)` —— 提角色的复制"检查频率"

- **作用对象**：`this`（**角色 Actor 本身**，不是 ASC！）。
- **干嘛**：设置这个 Actor **每秒被考虑复制多少次**。默认角色是较低频率（如 2Hz），ASC 需要高频更新（血量/状态实时变化），所以提到 100。
- **源码**：`Actor.h` 第 4572-4575 行

```4572:4575:Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h
	/**
	 * Set the frequency at which this object will be considered for replication.
	 */
	UFUNCTION(BlueprintSetter)
	ENGINE_API void SetNetUpdateFrequency(float Frequency);
```

> 注意：这个函数作用在 **Actor**（`AActor`）上，不是组件。Lyra 在构造函数里调它，是因为"自带 ASC 的角色"整体需要高频同步。对应字段 `NetUpdateFrequency`（`Actor.h` 第 860-863 行）在 5.5 起改为私有，官方要求用 `Set/GetNetUpdateFrequency` 访问。

---

## 三、三者关系图（谁作用在谁身上）

```
ATenkaichiCharacterWithAbilities（Actor）
│
├─ [Actor 层]  SetNetUpdateFrequency(100)     ← 提角色整体的复制检查频率
│
└─ AbilitySystemComponent（UActorComponent）
      ├─ SetIsReplicated(true)                ← 打开 ASC 的复制开关
      └─ SetReplicationMode(Mixed)            ← ASC 里 GE 的复制精细度
```

| 函数 | 作用对象 | 属于哪个类 | 管什么 |
|------|---------|-----------|--------|
| `SetIsReplicated` | ASC（组件） | `UActorComponent` | 复制开/关 |
| `SetReplicationMode` | ASC（组件） | `UAbilitySystemComponent` | GE 复制精细度 |
| `SetNetUpdateFrequency` | 角色（Actor） | `AActor` | 复制检查频率 |

---

## 四、我们这版 vs Lyra

**完全一致**——这三行就是照抄 Lyra `LyraCharacterWithAbilities.cpp` 第 16-17、24 行，一字未改（连注释都一样）。Lyra 为什么这么配，我们就这么配。

---

## 五、一句话结论

**`SetIsReplicated(true)`（组件层，开 ASC 复制开关）+ `SetReplicationMode(Mixed)`（ASC 层，GE 对本人全量/对别人精简）+ `SetNetUpdateFrequency(100)`（Actor 层，角色每秒复制检查 100 次）——三行一起把"自带 ASC 的角色"的网络复制配好。前两个作用在 ASC 组件上，第三个作用在角色 Actor 上。**

---

## 六、下一步

回到主线第 5 步（授予属性集 / 初始化血量），继续按 Lyra 真实代码一比一推进。
