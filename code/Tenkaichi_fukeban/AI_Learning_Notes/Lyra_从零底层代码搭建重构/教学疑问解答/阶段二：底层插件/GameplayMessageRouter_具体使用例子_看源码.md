# GameplayMessageRouter 具体怎么用？—— 看源码的真实例子

> 回答：光说"松耦合、消息总线"太泛了。本文用**源码里的真实类**，把"谁在发、谁在听、怎么对上号"整条链路串起来看。
> 源码依据：`Tenkaichi\Plugins\GameplayMessageRouter\` + `LyraStarterGame\Source\LyraGame\`（Lyra 真实收发例子）。

---

## 一、先看清这套系统的三个零件

`GameplayMessageRouter` 插件里，真正干活的是这三个东西：

| 零件 | 是什么 | 源码位置 |
|------|--------|---------|
| **消息结构体** | 一个 `USTRUCT`，发的人和听的人约定好同一种类型 | 如 `FLyraVerbMessage` |
| **总线（子系统）** | `UGameplayMessageSubsystem`，负责"广播"和"登记监听" | `GameplayMessageSubsystem.h/.cpp` |
| **频道（Tag）** | 一个 `FGameplayTag`，发的人和听的人靠它对上号 | 如 `TAG_Lyra_Damage_Message` |

---

## 二、真实例子 1：先看"消息长什么样"（`FLyraVerbMessage`）

发消息前，得先定义一个"消息信封"。Lyra 用的是一个通用信封 `FLyraVerbMessage`（`Messages\LyraVerbMessage.h`）：

```cpp
// 表示一个通用消息：谁(Instigator) 干了什么(Verb) 对谁(Target)（在某个上下文 Context，带个数值 Magnitude）
USTRUCT(BlueprintType)
struct FLyraVerbMessage
{
    GENERATED_BODY()

    FGameplayTag Verb;                        // 干了什么（这就是"频道/暗号"）
    TObjectPtr<UObject> Instigator = nullptr; // 谁干的
    TObjectPtr<UObject> Target = nullptr;     // 对谁
    FGameplayTagContainer InstigatorTags;
    FGameplayTagContainer TargetTags;
    FGameplayTagContainer ContextTags;
    double Magnitude = 1.0;                   // 数值（比如伤害量）
};
```

**关键**：`Verb` 这个字段本身就是个 `FGameplayTag`，它同时扮演两个角色——既描述"发生了什么"，又当"频道"用。所以发的时候用 `Message.Verb` 当频道广播，听的时候也按某个 Tag 监听。

---

## 三、真实例子 2：谁在"发"？（`ULyraHealthComponent` 广播死亡消息）

看 `Character\LyraHealthComponent.cpp` 第 169~182 行，当角色死亡时：

```cpp
// 发一个标准化的 Verb 消息，让其他系统都能观察到
{
    FLyraVerbMessage Message;
    Message.Verb = TAG_Lyra_Elimination_Message;              // 频道：淘汰
    Message.Instigator = DamageInstigator;                    // 谁杀的
    Message.Target = ULyraVerbMessageHelpers::GetPlayerStateFromObject(...); // 谁死了
    Message.TargetTags = ...;

    // 关键一句：拿到总线，把消息广播出去
    UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
    MessageSystem.BroadcastMessage(Message.Verb, Message);    // 按 Verb 频道广播
}
```

**发消息就三步**：
1. `UGameplayMessageSubsystem::Get(World)` 拿到总线
2. 填好 `FLyraVerbMessage`（重点是 `Verb` 频道）
3. `BroadcastMessage(频道, 消息)` 广播

> Lyra 里到处是这种"发"：能力释放失败（`TAG_ABILITY_SIMPLE_FAILURE_MESSAGE`）、重置（`GameplayEvent_Reset`）、快捷栏变化（`TAG_Lyra_QuickBar_Message_SlotsChanged`）、背包变化（`TAG_Lyra_Inventory_Message_StackChanged`）……全都是同一个套路。

---

## 四、真实例子 3：谁在"听"？（`ULyraDamageLogDebuggerComponent` 监听伤害）

看 `Weapons\LyraDamageLogDebuggerComponent.cpp`，这是一个"伤害日志调试组件"，它想监听伤害消息：

```cpp
void ULyraDamageLogDebuggerComponent::BeginPlay()
{
    Super::BeginPlay();

    // 监听：拿到总线，按 TAG_Lyra_Damage_Message 频道登记，回调 OnDamageMessage
    UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
    ListenerHandle = MessageSubsystem.RegisterListener(
        TAG_Lyra_Damage_Message,        // 我要听这个频道
        this,                           // 回调绑在谁身上（弱引用，自动判活）
        &ThisClass::OnDamageMessage);   // 收到消息调这个成员函数
}

void ULyraDamageLogDebuggerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 注销：不用再听了，把句柄交回去
    UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
    MessageSubsystem.UnregisterListener(ListenerHandle);
    Super::EndPlay(EndPlayReason);
}

// 回调：收到消息时被调用，签名是 (FGameplayTag Channel, const FLyraVerbMessage& Payload)
void ULyraDamageLogDebuggerComponent::OnDamageMessage(FGameplayTag Channel, const FLyraVerbMessage& Payload)
{
    if (Payload.Target == GetOwner())          // 只关心打到我身上的
    {
        LogEntry.NumImpacts++;
        LogEntry.SumDamage += -Payload.Magnitude;  // 累加伤害
    }
}
```

**听消息就三步**：
1. `UGameplayMessageSubsystem::Get(this)` 拿到总线
2. `RegisterListener(频道, 对象, 成员函数)` 登记，返回一个 `ListenerHandle`（句柄）
3. 用完 `UnregisterListener(句柄)` 注销

**注意**：`OnDamageMessage` 的签名是 `(FGameplayTag Channel, const FLyraVerbMessage& Payload)`——第二个参数类型 `FLyraVerbMessage` **必须和广播方发的是同一种结构体**，否则收不到（类型对不上会打错误日志）。

---

## 五、发的人和听的人，真的互不认识

这是这套系统最妙的地方：

- **发的人** `ULyraHealthComponent`（角色血量组件）——它不知道 `ULyraDamageLogDebuggerComponent` 这个调试组件存在。
- **听的人** `ULyraDamageLogDebuggerComponent`——它也不知道血量组件具体是谁。

它们唯一的交集是：**同一个频道 Tag（`TAG_Lyra_Damage_Message`）+ 同一个消息结构体（`FLyraVerbMessage`）**。

> 类比：对讲机广播。A 拿着对讲机喊"所有人注意，3 号区域有敌人"；B 正好在听 3 号频道，就收到了。A 不需要知道 B 是谁、B 在不在、有多少个 B。

---

## 六、广播底层到底怎么匹配的？（看 `BroadcastMessageInternal`）

`GameplayMessageSubsystem.cpp` 第 65~122 行，是广播的真正逻辑。核心是**按 Tag 的层级向上遍历**：

```cpp
void UGameplayMessageSubsystem::BroadcastMessageInternal(FGameplayTag Channel, ...)
{
    bool bOnInitialTag = true;
    // 从当前 Tag 开始，一路往父 Tag 走（A.B.C → A.B → A）
    for (FGameplayTag Tag = Channel; Tag.IsValid(); Tag = Tag.RequestDirectParent())
    {
        if (const FChannelListenerList* pList = ListenerMap.Find(Tag))
        {
            for (const FGameplayMessageListenerData& Listener : ListenerArray)
            {
                // 精确匹配：只有第一个 Tag（bOnInitialTag）才发
                // 部分匹配：所有父 Tag 都发
                if (bOnInitialTag || (Listener.MatchType == EGameplayMessageMatch::PartialMatch))
                {
                    // 类型校验：收的类型必须是发类型的父类，否则打错误日志
                    if (StructType->IsChildOf(Listener.ListenerStructType.Get()))
                    {
                        Listener.ReceivedCallback(Channel, StructType, MessageBytes);
                    }
                }
            }
        }
        bOnInitialTag = false;
    }
}
```

这解释了头文件里两个匹配规则（`GameplayMessageTypes2.h`）：

| 匹配规则 | 含义 | 例子 |
|---------|------|------|
| `ExactMatch`（默认） | 只收"频道完全一样"的消息 | 听 `A.B`，只收到广播 `A.B` |
| `PartialMatch` | 收"频道是它的子级"的消息 | 听 `A.B`，能收到 `A.B.C`、`A.B.D` |

---

## 七、蓝图怎么用？（`UAsyncAction_ListenForGameplayMessage`）

C++ 用 `RegisterListener`，蓝图用异步节点 `Listen For Gameplay Messages`：

```cpp
UFUNCTION(BlueprintCallable, ...)
static UAsyncAction_ListenForGameplayMessage* ListenForGameplayMessages(
    UObject* WorldContextObject,
    FGameplayTag Channel,          // 听哪个频道
    UScriptStruct* PayloadType,    // 消息结构体类型（必须和发的一致）
    EGameplayMessageMatch MatchType);
```

蓝图里拖出这个节点，指定频道和 Payload 类型，`OnMessageReceived` 引脚就会在收到消息时触发，再用 `GetPayload` 取回消息内容。

底层其实还是调 `RegisterListenerInternal`（看 `AsyncAction_ListenForGameplayMessage.cpp` 第 40 行），只是包了一层蓝图友好的异步封装。

---

## 八、整条链路串起来（一句话版）

```
ULyraHealthComponent（发）           ULyraDamageLogDebuggerComponent（听）
        │                                      │
        │ BroadcastMessage(TAG_Damage, Msg)    │ RegisterListener(TAG_Damage, &OnDamageMessage)
        ▼                                      ▼
        └──────────► UGameplayMessageSubsystem ◄──────────┘
                     （按 Tag 对上号，转发消息）
```

**所以 GameplayMessageRouter 的"具体用法"就是**：

1. 定义一个 `USTRUCT` 消息结构体（比如 `FLyraVerbMessage`）。
2. 发的一方：`Get(World)` 拿总线 → `BroadcastMessage(频道Tag, 消息)`。
3. 听的一方：`Get(this)` 拿总线 → `RegisterListener(频道Tag, this, &成员函数)` → 拿到句柄 → 用完 `UnregisterListener(句柄)`。
4. 发和听唯一的约定 = **同一个频道 Tag + 同一个消息结构体类型**，除此之外互不认识。

**好处**：血量组件（发）和调试组件（听）完全解耦。哪天想加个"成就系统"也监听伤害消息，直接 `RegisterListener` 就行，血量组件一行代码都不用改。