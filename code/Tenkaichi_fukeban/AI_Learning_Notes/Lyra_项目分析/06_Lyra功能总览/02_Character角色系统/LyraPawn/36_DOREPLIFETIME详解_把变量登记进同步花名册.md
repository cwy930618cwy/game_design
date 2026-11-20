# `DOREPLIFETIME(ThisClass, MyTeamID)` 详解：把变量正式登记进"同步花名册"

> **定位**：单独讲透 `LyraPawn.cpp` 第 24 行这一句——它是整个网络复制里"**真正动手登记**"的那一步。上一行 `Super::...` 是帮父亲填名单，这一行才是**你自己填的那一条**："我要同步 `MyTeamID`"。
>
> **关联**：
> - [35_GetLifetimeReplicatedProps与DOREPLIFETIME详解_网络复制注册](./35_GetLifetimeReplicatedProps与DOREPLIFETIME详解_网络复制注册.md) — 整体函数、数组从哪来、"等别人调我"
> - [26_宏Macro详解_从define到UE委托宏](./26_宏Macro详解_从define到UE委托宏.md) — `DOREPLIFETIME` 是个宏
> - [34_OnControllerChangedTeam详解_队伍变化回调是怎么被触发的](./34_OnControllerChangedTeam详解_队伍变化回调是怎么被触发的.md) — 客户端收到同步后走 OnRep_MyTeamID
>
> **一句话**：`DOREPLIFETIME(ThisClass, MyTeamID)` = **"把 `MyTeamID` 这个变量，正式登记进引擎的网络同步花名册。"** 登记之后，服务器上它一变，引擎就自动复制给所有客户端。没有它，`MyTeamID` 只是服务器本地的普通变量。

---

## 一、先看它在整段代码里的位置

```cpp
void ALyraPawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);   // ① 帮父亲填他的那份
    DOREPLIFETIME(ThisClass, MyTeamID);                    // ② ← 你自己填的这一条
}
```

回顾上一轮你悟通的"等别人调我"：

```
整个 GetLifetimeReplicatedProps() = 引擎来问："你要同步啥？"（递个空数组给你）
   ├─ Super::...                            → 让父亲也往同一个数组里填他的属性
   └─ DOREPLIFETIME(ThisClass, MyTeamID)    → 你具体填的那一条记录
```

> **这一行就是"你回答问题的具体内容"**——父亲通过 Super 回答了他的，你用 DOREPLIFETIME 回答了你的。

---

## 二、它到底是干嘛的？

一句话：

> **把 `MyTeamID` 这个变量，正式登记进引擎的"网络同步花名册"。**

- **没写这一行**：`MyTeamID` 只是个服务器本地的普通变量，客户端永远不知道它的值，更不会跟着变。
- **写了这一行**：引擎记住了"`MyTeamID` 要同步"，以后服务器上它一变，引擎就自动打包发给所有客户端。

> **类比**（接着上一篇的"快递"比喻）：
> - `GetLifetimeReplicatedProps` = 你填的整张寄件清单。
> - `Super::...` = 清单上"父类那些包裹"。
> - **`DOREPLIFETIME(ThisClass, MyTeamID)` = 你在清单上具体写下的一行："这个包裹（MyTeamID）要寄。"**
> - 不写这一行，这个包裹快递公司就当它不存在。

---

## 三、两个参数在干嘛？

```cpp
DOREPLIFETIME(ThisClass, MyTeamID);
//            └──┬──┘  └───┬───┘
//              参数1      参数2
```

| 参数 | 是什么 | 作用 |
|------|--------|------|
| `ThisClass` | 当前类（`ALyraPawn`） | 告诉宏"我在哪个类里"，用来**校验继承合法性 + 精确定位这个属性** |
| `MyTeamID` | 要同步的变量名 | 告诉宏"**我要同步的是这个变量**" |

> **为什么第一个参数是 `ThisClass` 而不是直接写类名？**
> `ThisClass` 是 UE 的一个宏/约定，会自动展开成"当前所在的类"。这样你**复制粘贴这段代码到任何类里都不用改**，它自己知道是谁。很贴心。

---

## 四、它是宏！真实源码长这样

它是宏（第 26 篇讲过宏），定义在 `UnrealNetwork.h` 第 261 行：

```cpp
#define DOREPLIFETIME(c,v)  DOREPLIFETIME_WITH_PARAMS(c, v, FDoRepLifetimeParams())
```

再往里展开（第 253~259 行的核心）：

```cpp
{
    // 编译期校验：这个属性确实属于合法的继承链
    static_assert(ValidateReplicatedClassInheritance<c, ThisClass>(...), "...");

    // 靠 UE 反射系统，找到变量 MyTeamID 对应的元数据 FProperty
    FProperty* ReplicatedProperty = GetReplicatedProperty(
        StaticClass(), c::StaticClass(), GET_MEMBER_NAME_CHECKED(c, v));

    // 把这条属性塞进那个"花名册"数组（就是引擎递来的 OutLifetimeProps）
    RegisterReplicatedLifetimeProperty(
        ReplicatedProperty, OutLifetimeProps, FixupParams<decltype(c::v)>(params));
}
```

**人话翻译三步**：

1. `GET_MEMBER_NAME_CHECKED(c, v)` —— 把变量名 `MyTeamID` 取出来（编译期转成字符串）。
2. `GetReplicatedProperty(...)` —— 靠 UE 反射，找到这个变量对应的 `FProperty`（描述它的元数据）。
3. `RegisterReplicatedLifetimeProperty(...)` —— **把它塞进 `OutLifetimeProps` 那个花名册数组**。

> **关键**：看到最后一步了吗？`Register...Property(ReplicatedProperty, OutLifetimeProps, ...)` —— 它正是往**引擎递给你的那个 `OutLifetimeProps` 数组里塞东西**。这就把前两篇串起来了：**引擎递数组 → 你调 DOREPLIFETIME → 它往数组里塞 MyTeamID**。

---

## 五、完整串联：从"登记"到"客户端收到"

```
【阶段一：建名单（游戏启动，一次性）】

   引擎递来空数组 OutLifetimeProps
        │
        ▼
   GetLifetimeReplicatedProps()
        ├─ Super::...                 父亲填他的属性
        └─ DOREPLIFETIME(ThisClass, MyTeamID)
              │  展开后：把 MyTeamID 塞进 OutLifetimeProps
              ▼
   引擎收走填满的数组 → 缓存成"同步花名册"
        │
        ════════════ 此后照名单干活，不再重复调用 ════════════
        ▼

【阶段二：真同步（运行期，反复发生）】

   【服务器】                          【所有客户端】
      │ ① 改 MyTeamID = 新队伍
      │ ② 引擎查名单：MyTeamID 在！
      │ ③ 打包发送 ───────────────►  ④ 收到，写入本地 MyTeamID
      │                              ⑤ 触发 OnRep_MyTeamID
      │                                 → ConditionalBroadcastTeamChanged
      │                                 → UI 更新队伍显示
```

---

## 六、和 `.h` 里 UPROPERTY 的关系（别漏了另一半）

光写 `DOREPLIFETIME` 还不够，`.h` 里得先声明（`LyraPawn.h` 第 57 行）：

```cpp
UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)   // 声明：我要复制 + 复制到后回调谁
FGenericTeamId MyTeamID;
```

| 在哪 | 干什么 | 缺了会怎样 |
|------|--------|-----------|
| `.h` 里 `UPROPERTY(ReplicatedUsing=...)` | **写申请单**：声明这变量要复制，并指定客户端收到后回调 `OnRep_MyTeamID` | 客户端收到了也不知道该通知谁 |
| `.cpp` 里 `DOREPLIFETIME` | **交进审批系统**：真正把变量登记进同步花名册 | 引擎根本不认识这变量，不同步 |

> **记忆**：`UPROPERTY` 是"写申请单"，`DOREPLIFETIME` 是"把申请交进审批系统"。**两者都做，复制才真正生效。**

---

## 七、常见误区

| 误区 | 正确理解 |
|------|---------|
| "写了 UPROPERTY 就能同步，不用写 DOREPLIFETIME" | ❌ 必须两者都做，DOREPLIFETIME 才是真正登记 |
| "DOREPLIFETIME 会立刻同步一次" | ❌ 它只是"登记"，真正同步发生在变量值改变时 |
| "它能同步任意变量" | ❌ 变量必须先在 `.h` 用 UPROPERTY 声明，且类型受支持 |
| "客户端改了也会被同步" | ❌ 复制是服务器→客户端单向，客户端改了会被服务器覆盖 |
| "这是网络发来的" | ❌ 它是本机引擎调用、往引擎递来的数组里塞数据，跟网络无关 |

---

## 八、总结

```
Q：DOREPLIFETIME(ThisClass, MyTeamID) 是干嘛的？
A：把 MyTeamID 正式登记进引擎的网络同步花名册。

  • 它是宏（UnrealNetwork.h 第261行），展开后：
      靠反射找到 MyTeamID 的 FProperty → 塞进引擎递来的 OutLifetimeProps 数组。
  • 两个参数：ThisClass=当前类（定位+校验），MyTeamID=要同步的变量。
  • 配合 .h 的 UPROPERTY(ReplicatedUsing=OnRep_MyTeamID)：
      UPROPERTY=写申请单，DOREPLIFETIME=交审批，两者都做才生效。
  • 效果：服务器改 MyTeamID → 引擎照名单复制给所有客户端 → 触发 OnRep_MyTeamID。
```

**一句话**：这一行是"**你回答引擎'我要同步啥'的具体内容**"——它把 `MyTeamID` 登记进引擎的同步花名册。它是宏，本质是"靠反射找到变量的元数据，塞进引擎递来的数组"。必须和 `.h` 里的 `UPROPERTY(ReplicatedUsing=...)` 配合使用，缺一不可。

---

## 九、下一步

- [35_GetLifetimeReplicatedProps与DOREPLIFETIME详解_网络复制注册](./35_GetLifetimeReplicatedProps与DOREPLIFETIME详解_网络复制注册.md) — 整体函数与"数组从哪来"
- [26_宏Macro详解_从define到UE委托宏](./26_宏Macro详解_从define到UE委托宏.md) — 它是宏
- [34_OnControllerChangedTeam详解_队伍变化回调是怎么被触发的](./34_OnControllerChangedTeam详解_队伍变化回调是怎么被触发的.md) — 客户端收到同步后的处理
