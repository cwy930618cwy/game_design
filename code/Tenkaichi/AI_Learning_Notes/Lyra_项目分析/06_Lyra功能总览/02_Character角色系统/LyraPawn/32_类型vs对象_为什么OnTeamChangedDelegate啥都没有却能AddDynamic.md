# 类型 vs 对象：为什么 `OnTeamChangedDelegate` 啥都没有，却能 `AddDynamic`？

> **定位**：回答一个根本性困惑——"**`FOnLyraTeamIndexChangedDelegate` 我一直以为只是个类型，它啥数据都没有，为什么能拿来 `AddDynamic` 记联系人？**"。这其实是 C++ 里"**类型**"和"**对象**"的区别，是初学者最容易晕的通用盲区。
>
> **关联**：
> - [30_LyraTeamAgentInterface三个实现函数详解](./30_LyraTeamAgentInterface三个实现函数详解_Set_Get_GetDelegate.md) — `GetOnTeamIndexChangedDelegate` 返回它
> - [26_宏Macro详解_从define到UE委托宏](./26_宏Macro详解_从define到UE委托宏.md) — 这个类型是委托宏生成的
> - [24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic](./24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic.md) — AddDynamic 怎么用
>
> **一句话**：`FOnLyraTeamIndexChangedDelegate` **既是类型，也是一种"容器"**。当你写 `OnTeamChangedDelegate`（成员变量）时，你不是声明了一个空壳子，而是**造出了一个真实的、空的"通讯录本"对象**——它天生就有"记联系人、打电话"的能力，只是暂时还没记人而已。`AddDynamic` 就是往这个本子里记人。

---

## 一、你的困惑拆成两半

你其实混了两个概念：

```cpp
FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;   // 第 61 行
//        ↑ 类型                    ↑ 对象(成员变量)
```

- **`FOnLyraTeamIndexChangedDelegate`** —— 这是**类型**（type），是一个"模板/模具"。
- **`OnTeamChangedDelegate`** —— 这是**对象**（object/实例），是用那个模具**造出来的一个真实东西**。

> **你的误区**：你以为"`FOnLyraTeamIndexChangedDelegate` 只是个类型，所以它是空的、啥也干不了"。
> **真相**：类型本身确实不能干活，但**用它造出来的对象 `OnTeamChangedDelegate` 是能干活的**——因为它一出生就自带了"记联系人、广播"的能力。

---

## 二、关键类比：饼干模具 vs 一块饼干

还记得第 26 篇说的"宏是饼干模具"吗？这里同理：

| | 是什么 | 能不能吃/用 |
|---|---|---|
| **`FOnLyraTeamIndexChangedDelegate`（类型）** | 饼干**模具**（图纸） | ❌ 模具本身不能吃，它只是"造饼干的样板" |
| **`OnTeamChangedDelegate`（对象）** | 用模具压出来的**一块真饼干** | ✅ 能吃！天生就是块完整的饼干 |

- **类型 = 模具**：它定义了"这种饼干长什么样、能干啥"，但它自己不是饼干。
- **对象 = 用模具造出的实物**：你一声明成员变量，就等于"用模具压了一块饼干出来"，这块饼干**天生完整、天生可用**。

```cpp
FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;
//        模具                       用模具压出的一块饼干(对象)
//                                  ↑ 这块饼干天生就能"被加联系人、被广播"
```

---

## 三、为什么这块"饼干"天生就能 `AddDynamic`？

因为**委托这个类型，本质就是一个"通讯录本容器"**。

回忆第 26/27 篇：委托宏展开后，生成的类大致长这样：

```cpp
class FOnLyraTeamIndexChangedDelegate : public ... {
    // ...
    void AddDynamic(...);      // ← 方法：往名单里记一个联系人
    void Broadcast(...);       // ← 方法：给名单上所有人都打一遍电话
private:
    TArray<FScriptDelegate> MulticastDelegate;   // ← 联系人名单数组(天生的)
};
```

**重点看最后一行**：每个委托对象内部，都**天生带了一个 `TArray`（联系人名单数组）**。

所以当你写：

```cpp
FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;
```

等于造出了一块饼干，这块饼干**肚子里天生就有一张"联系人名单"**（只是现在是空的）。

### 于是整个逻辑通了：

```
① 声明成员变量 → 造出一块饼干，肚子里天生有空名单
       ↓
② GetOnTeamIndexChangedDelegate() 返回 &OnTeamChangedDelegate
       ↓ 别人拿到这块饼干的地址
③ AddDynamic(this, &xxx)  → 往肚子里的名单里记一个联系人
       ↓
④ Broadcast(...)  → 按名单给所有人打电话(通知)
```

**所以"啥都没有却能 AddDynamic"的秘密是**：它不是"啥都没有"，而是"**肚子里天生有一张空的联系人名单**"。`AddDynamic` 就是往这张名单里写字。空着也能写，写了才能通知。

---

## 四、再补一刀：类型 vs 对象，到处都这样

这不是委托特有的，C++ 里**所有"值类型"成员变量都是这个道理**：

```cpp
class MyPawn {
    int         Score;              // 类型 int，对象 Score（天生是个数字，默认值可能是垃圾或0）
    float       Speed;              // 类型 float，对象 Speed
    TArray<int> Items;              // 类型 TArray<int>，对象 Items（天生是个空数组，能用）
    FString     Name;               // 类型 FString，对象 Name（天生是个空字符串，能用）
    FOnLyra...  OnTeamChangedDelegate;  // 类型 FOnLyra...，对象 OnTeamChangedDelegate（天生是空委托，能用）
};
```

| 类型（模具） | 对象（造出来的实物，天生可用） |
|---|---|
| `int` | `Score`（就是个数字） |
| `TArray<int>` | `Items`（天生空数组，能 Add/遍历） |
| `FString` | `Name`（天生空字符串，能拼接） |
| `FOnLyraTeamIndexChangedDelegate` | `OnTeamChangedDelegate`（天生空委托，能 AddDynamic/Broadcast） |

> **记忆**：**类型是"名词的定义"，对象是"实际存在的那个东西"**。你说"人类"（类型）不能吃饭，但"张三"（对象）能吃饭。`FOnLyra...` 是"人类"，`OnTeamChangedDelegate` 是"张三"。

---

## 五、那"指针"为什么就必须赋值？（对比才彻底懂）

上一轮你问过类似的，这里彻底对比清楚：

```cpp
FOnLyraTeamIndexChangedDelegate* pDelegate;   // 这是【指针】
pDelegate->AddDynamic(...);                   // ❌ 崩溃！pDelegate 是野指针，没指向任何饼干
```

| | 成员对象（直接用） | 指针（要先赋值） |
|---|---|---|
| 写法 | `OnTeamChangedDelegate.AddDynamic(...)` | `pDelegate->AddDynamic(...)` |
| 出生状态 | 天生是一块完整饼干 | 只是一张空白纸条（没地址） |
| 要不要赋值 | ❌ 不用，天生可用 | ✅ 必须先 `pDelegate = &某个对象` |
| 不赋值后果 | 正常 | 崩溃（访问不存在的地址） |

> **一句话**：
> - **成员对象** = 兜里本来就有的饼干，拿来就能吃（`AddDynamic`）。
> - **指针** = 一张纸条，得先写上饼干在哪（赋值地址），才能去吃。

这也解释了为什么 `GetOnTeamIndexChangedDelegate()` 要返回**地址 `&OnTeamChangedDelegate`**——它把"兜里那块饼干的位置"告诉别人，别人才能通过指针找到并操作它。

---

## 六、一张图看懂"类型→对象→能用"

```
   【类型 / 模具】                【对象 / 造出的实物】           【能干啥】
 ┌────────────────────┐       ┌──────────────────────┐    ┌──────────────────┐
 │ FOnLyraTeamIndex   │ 造出  │ OnTeamChangedDelegate│    │ .AddDynamic()    │
 │ ChangedDelegate    │──────►│ (成员变量)            │───►│ .Broadcast()     │
 │ (只是定义/模具)     │       │ 肚子里天生有空名单    │    │ (天生就具备的能力) │
 └────────────────────┘       └──────────────────────┘    └──────────────────┘
      不能直接干活                  天生可用                    往名单记人/通知

   类比：
      饼干模具        ──压一块──►     一块真饼干          ──能──►   被咬(被AddDynamic)
```

---

## 七、常见误区

| 误区 | 正确理解 |
|------|---------|
| "`FOnLyra...` 只是个类型，所以不能用" | ❌ 类型不能用，但用它造的对象 `OnTeamChangedDelegate` 天生能用 |
| "成员变量出生是空的，得先赋值才能 AddDynamic" | ❌ 它出生就是"空的但完整的"委托对象，空名单≠不能用 |
| "对象和类型是一回事" | ❌ 类型是模具，对象是用模具造的实物 |
| "指针和成员对象一样，都不用赋值" | ❌ 指针必须赋值指向某对象；成员对象天生就在、不用赋值 |

---

## 八、总结

```
Q：FOnLyraTeamIndexChangedDelegate 只是个类型，啥都没有，为啥能 AddDynamic？

A：你把"类型"和"对象"混在一起了。
   • FOnLyraTeamIndexChangedDelegate = 类型/模具（不能直接干活）
   • OnTeamChangedDelegate          = 对象/实物（用模具造出来的，天生能干活）

   声明成员变量 = 用模具造出一块饼干，这块饼干：
     → 肚子里天生有一张"联系人名单"(TArray，初始为空)
     → 天生具备 AddDynamic(记人) / Broadcast(通知) 的能力
     → 空名单 ≠ 不能用，恰恰是等着你来记人

   对比指针：指针是"纸条"要先赋值地址；成员对象是"兜里的饼干"天生可用。
```

**一句话**：`FOnLyraTeamIndexChangedDelegate` 确实是类型（模具），但你写的 `OnTeamChangedDelegate` 是**用它造出来的对象（一块饼干）**——这块饼干肚子里**天生就有一张空的联系人名单**，所以天生就能 `AddDynamic`（往名单记人）和 `Broadcast`（通知）。"啥都没有"只是说名单暂时是空的，不是说它没能力。**类型是定义，对象才是能干活的那个。**

---

## 九、下一步

- [30_LyraTeamAgentInterface三个实现函数详解](./30_LyraTeamAgentInterface三个实现函数详解_Set_Get_GetDelegate.md) — GetOnTeamIndexChangedDelegate 返回它
- [26_宏Macro详解_从define到UE委托宏](./26_宏Macro详解_从define到UE委托宏.md) — 这个类型是委托宏生成的
- [24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic](./24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic.md) — AddDynamic 怎么用
