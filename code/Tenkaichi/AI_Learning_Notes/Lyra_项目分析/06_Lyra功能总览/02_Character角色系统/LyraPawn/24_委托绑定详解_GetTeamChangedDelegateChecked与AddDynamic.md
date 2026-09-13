# 委托绑定逐方法拆解 —— `GetTeamChangedDelegateChecked()->AddDynamic(...)` 到底在干嘛

> **定位**：把 LyraPawn 里最绕的一行链式调用**拆成一个一个方法**，逐个讲清"它是谁、返回什么、为什么这么写"。这是读懂 UE "委托（Delegate）"机制的关键一战。
>
> **关联**：
> - [21_PossessedBy与UnPossessed详解](./21_PossessedBy与UnPossessed详解_队伍绑定与解绑.md) — 这行代码的上下文
> - [05_LyraTeamAgentInterface队伍接口详解](./05_LyraTeamAgentInterface队伍接口详解.md) — 委托的定义来源
> - [23_Cast为什么返回指针](./23_Cast为什么返回指针_指针基础详解.md) — 为什么要用 `->`
>
> **一句话**：这三行干一件事——**"找到 Controller 身上那个'队伍变了就响铃'的装置，把我的 `OnControllerChangedTeam` 函数接上去。"** 拆开看就是：拿装置（`Get...Delegate`）→ 检查它没坏（`Checked`）→ 接线（`AddDynamic`）。

---

## 一、先看完整代码（你要逐方法搞懂的三行）

```cpp
// ⑤ 监听这个 Controller 以后队伍变了的通知（关键！）
ControllerAsTeamProvider->GetTeamChangedDelegateChecked()
    .AddDynamic(this, &ThisClass::OnControllerChangedTeam);
```

这一行其实是**三个动作串成一条链**：

```
ControllerAsTeamProvider          ← 第 1 环：那个 Controller（队伍提供者）
        ->
   GetTeamChangedDelegateChecked() ← 第 2 环：拿到"队伍变化通知装置"
        .
   AddDynamic(this, &...::OnControllerChangedTeam)  ← 第 3 环：把我的函数接上去
```

下面一环一环拆。

---

## 二、第 1 环：`ControllerAsTeamProvider` 是谁？

```cpp
ControllerAsTeamProvider->...
└────────────┬───────────┘
       就是前面 Cast 出来的那个"队伍提供者"
```

- 它就是上面 `if` 里 `Cast` 得到的变量（还记得吗？是 `NewController` 的"队伍身份马甲"）。
- 后面的 `->` = **顺着它的门牌号，去访问它身上的东西**（指针访问成员）。
- 我们要访问的，是它身上的"**队伍变化通知装置**"。

---

## 三、第 2 环：`GetTeamChangedDelegateChecked()` 是什么？

先理解它背后的东西——**委托（Delegate）**。

### 什么是委托？一句话

> **委托 = 一个"事件响铃器"**。你往里面登记一个函数，等某件事发生时，它就自动调用你登记的那个函数。

类比：
> 你在家装了个"门铃"（委托）。谁来按（事件发生），你就给谁开门（执行登记的函数）。
> 委托就是那个"登记谁来按铃 → 响了就去开哪扇门"的装置。

### 拆名字理解这个方法

```cpp
GetTeamChangedDelegateChecked()
└─┬─┘ └──────┬───────┘ └────┬───┘
 获取     队伍变化的       Checked（已检查/确保有效）
          委托(响铃器)
```

- **`Get...Delegate`**：返回这个对象身上的"队伍变化委托"（那个响铃器）。
- **`Checked`**：UE 的一个约定——带这个词的方法表示"**如果这东西坏了/为空就直接报错崩溃**"，意思是"我保证你现在拿到的绝对能用"。

> ⚠️ 对比：还有个 `GetOnTeamIndexChangedDelegate()`（不带 Checked），它可能返回空。`Checked` 版是"放心用，空了我替你报错"。

### 它返回什么？

返回一个**委托对象的引用**（可以理解为"那个响铃器的操作手柄"），好让下一步往上接东西。

---

## 四、第 3 环：`.AddDynamic(this, &ThisClass::OnControllerChangedTeam)` 是什么？

这是最关键的一环——**"接线"**。

### 为什么有个 `.` 而不是 `->`？

```cpp
GetTeamChangedDelegateChecked()   // ← 这个方法返回的是"委托本身"（不是指针）
    .AddDynamic(...)              // ← 所以用 . 直接访问委托的方法
```

- 上一环用 `->` 是因为 `ControllerAsTeamProvider` 是指针。
- 这一环用 `.` 是因为 `Get...Delegate` **返回的是委托对象本人**（不是指针），对象本人用 `.`。
- （回顾第 23 篇：指针用 `->`，对象本人用 `.`）

### `AddDynamic` 的两个参数

```cpp
AddDynamic(this, &ThisClass::OnControllerChangedTeam)
           └─┬─┘   └──────────────┬──────────────────┘
          参数①                  参数②
         "挂在谁身上"          "响铃时调哪个函数"
```

| 参数 | 是什么 | 大白话 |
|------|--------|--------|
| `this` | 当前这个 LyraPawn 对象自己 | "铃铛装在**我**身上" |
| `&ThisClass::OnControllerChangedTeam` | 指向本类 `OnControllerChangedTeam` 函数的指针 | "铃响时去叫**我这个类的 OnControllerChangedTeam 函数**" |

合起来：**"把这个铃铛装在我（this）身上，等队伍一变，就自动调用我的 `OnControllerChangedTeam` 函数。"**

### `&ThisClass::OnControllerChangedTeam` 怎么读？

```cpp
&ThisClass::OnControllerChangedTeam
└┬┘ └───┬───┘ └────────┬────────┘
 取地址  当前这个类      那个函数的名字
```

- `::` = "所属关系"，读作"……的"。
- 整句 = "**当前这个类的** `OnControllerChangedTeam` **函数的地址**"。
- 这就是 C++ 里"**成员函数指针**"的写法——把一个函数当成数据传出去。

### 为什么叫 `AddDynamic`？

- **`Add`** = 往委托里"添加"一个监听者。
- **`Dynamic`** = UE 特有的"**动态委托**"，特点是**可以被序列化、能在蓝图里用、运行时才确定绑谁**（相对标准 C++ 的死板回调更灵活）。
- UE 里还有 `AddStatic`（绑普通函数）、`AddLambda`（绑 lambda）等，`AddDynamic` 是最常用、最灵活的那个。

---

## 五、三行连起来，完整故事

```
ControllerAsTeamProvider                那个有队伍能力的 Controller
        ->                              （通过门牌号访问）
   GetTeamChangedDelegateChecked()      取出它身上的"队伍变化响铃器"（保证能用）
        .                               （委托是对象本人，用 . ）
   AddDynamic(this, &...::OnControllerChangedTeam)
                                        把铃铛装我身上，铃响就叫我这个类的 OnControllerChangedTeam
```

**人话版**：
> "找到老板（Controller）身上那个'队伍一变就通知'的装置，把我的 `OnControllerChangedTeam` 登记上去——以后老板队伍一变，就自动来叫我，我好跟着更新。"

---

## 六、对称的另一半：`RemoveAll`（出窍时拆线）

理解了"接线"，拆线就好懂了。`UnPossessed` 里是它的反面：

```cpp
ControllerAsTeamProvider->GetTeamChangedDelegateChecked()
    .RemoveAll(this);                   // 把我挂上去的所有监听都拆掉
```

| 动作 | 方法 | 含义 |
|------|------|------|
| 接线（入体） | `AddDynamic(this, ...)` | 把我登记成监听者 |
| 拆线（出窍） | `RemoveAll(this)` | 把登记过的全撤下 |

> **必须对称！** 灵魂走了铃铛还装着，就会通知到一个没人控制的 Pawn，逻辑错乱。

---

## 七、一张图看懂"委托"这个装置

```
        【队伍变化事件】（老板改队伍时触发）
               │
               ▼
   ┌───────────────────────────────┐
   │   委托（响铃器）                │
   │   GetTeamChangedDelegateChecked│
   │                               │
   │   登记的监听者列表：            │
   │    ┌────────────────────────┐ │
   │    │ this(LyraPawn)          │ │  ← AddDynamic 把我加进来
   │    │  → OnControllerChangedTeam│ │  ← 铃响就调这个函数
   │    └────────────────────────┘ │
   └───────────────────────────────┘
               │ 队伍一变，遍历所有登记者
               ▼
      调用每个登记者的对应函数
      （这里就是 LyraPawn::OnControllerChangedTeam）
```

---

## 八、常见误区

| 误区 | 正确理解 |
|------|---------|
| "委托就是个普通函数" | ❌ 委托是"事件响铃器"，能登记多个监听者，事件发生时统一通知 |
| "为什么前面用 `->` 这里用 `.`" | 前者是指针对象用 `->`，委托本身是对象用 `.` |
| "`Checked` 可有可无" | 它表示"空就崩溃"的保证，用它是为了省心、防 bug |
| "`AddDynamic` 只能绑一个" | 可以绑多个，事件发生时逐个调用 |
| "不拆线也没事" | ❌ 必须 `RemoveAll`，否则野监听导致后续逻辑错乱 |

---

## 九、总结

```
三行链式调用，三环相扣：

第1环 ControllerAsTeamProvider->   那个有队伍能力的 Controller（指针访问）
第2环 GetTeamChangedDelegateChecked()  取出它身上的"队伍变化响铃器"（委托，保证可用）
第3环 .AddDynamic(this, &...::OnControllerChangedTeam)
                                   把铃铛装我身上，铃响就调我的 OnControllerChangedTeam

记住三点：
  1. 委托 = 事件响铃器（登记函数，事件发生时自动调用）
  2. -> 用于指针，. 用于对象本人（委托返回的是对象，所以用 .）
  3. AddDynamic / RemoveAll 必须对称（接线与拆线配对）
```

**一句话**：这三行的目的只有一个——**"订阅老板的队伍变化通知"**。一旦订阅成功，老板队伍一变，LyraPawn 就能第一时间知道并跟着更新，这正是队友识别实时刷新的基础。

---

## 十、下一步

- [21_PossessedBy与UnPossessed详解](./21_PossessedBy与UnPossessed详解_队伍绑定与解绑.md) — 完整上下文
- [05_LyraTeamAgentInterface队伍接口详解](./05_LyraTeamAgentInterface队伍接口详解.md) — 委托从哪定义
- [23_Cast为什么返回指针](./23_Cast为什么返回指针_指针基础详解.md) — `->` vs `.` 的根本原因
