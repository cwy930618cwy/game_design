# 宏是"工具"，类型是"产品"：`FOnLyraTeamIndexChangedDelegate` 和 `DECLARE_..._DELEGATE` 到底谁才是宏？

> **定位**：解开一个极易晕的点——"**不是 `DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams` 才是宏吗？`FOnLyraTeamIndexChangedDelegate` 跟宏啥关系？**"。把"宏（工具）"和"它造出来的类型（产品）"彻底分开讲。
>
> **关联**：
> - [26_宏Macro详解_从define到UE委托宏](./26_宏Macro详解_从define到UE委托宏.md) — 宏的根本定义
> - [27_宏为什么长得像函数_去引擎里追宏源码实战](./27_宏为什么长得像函数_去引擎里追宏源码实战.md) — 怎么追宏、看展开
> - [32_类型vs对象_为什么OnTeamChangedDelegate啥都没有却能AddDynamic](./32_类型vs对象_为什么OnTeamChangedDelegate啥都没有却能AddDynamic.md) — 类型 vs 对象
>
> **一句话**：`DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams` 是**宏（工具/模具）**，`FOnLyraTeamIndexChangedDelegate` 是用这个工具**造出来的类型（产品/饼干）**。宏是"动作"，类型是"结果"。你调用一次宏，就生产出一个类型。

---

## 一、先直接回答你的困惑

你的疑问本质是三个东西搅在一起，先把它们拆开：

```cpp
// LyraTeamAgentInterface.h 第 15 行
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnLyraTeamIndexChangedDelegate,   // ← 这是【要造的类型名】
    UObject*, ObjectChangingTeam,
    int32, OldTeamID,
    int32, NewTeamID);
```

| 名字 | 它是什么 | 类比 |
|------|---------|------|
| `DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams` | **宏（工具）** | 饼干模具 / 打印机 |
| `FOnLyraTeamIndexChangedDelegate` | **类型（产品）** | 用模具压出的一块饼干 / 打印出的文件 |
| 那一整行调用 | **一次"生产动作"** | 把面团塞进模具压一下 |

> **核心关系**：**宏是"动词/动作"，类型是"名词/结果"。**
> 你执行一次"打印"（调宏），就产出一份"文件"（一个类型）。

---

## 二、为什么说 `FOnLyra...` "也是宏的产物"？

上一轮你说"它也是个宏嘛"——严格讲**不准确**，准确说法是：

> **`FOnLyraTeamIndexChangedDelegate` 不是宏，它是宏【生成出来的东西】。**

打个比方：

- `DECLARE_..._DELEGATE` = **打印机**（机器，是工具）
- `FOnLyraTeamIndexChangedDelegate` = **打印出来的一份合同**（产品）

你不会说"这份合同是一台打印机"，但你会说"这份合同是**打印机造出来的**"。同理：

- ❌ "`FOnLyra...` 是个宏" —— 不对，它不是宏本身
- ✅ "`FOnLyra...` 是宏生成的类型" —— 正确

---

## 三、完整的生产流程（一眼看懂）

```
【宏 / 工具】                          【类型 / 产品】
 ┌──────────────────────────┐        ┌─────────────────────────────┐
 │ DECLARE_DYNAMIC_MULTICAST │  生产   │ FOnLyraTeamIndexChangedDelegate │
 │ _DELEGATE_ThreeParams(    │ ─────► │                             │
 │     FOnLyraTeamIndexed.., │        │ 一个真实的 class，天生带：    │
 │     参数...)              │        │  · AddDynamic() 记联系人      │
 └──────────────────────────┘        │  · Broadcast() 通知          │
        ↑                            │  · 联系人名单数组(TArray)     │
   这是"模具"                         └─────────────────────────────┘
   编译前把参数吃进去                        ↑
   吐出一个 class                       这就是"产品"
                                     后续代码拿它当普通类型用
```

### 时间线：编译前 vs 编译后

```
编译前（预处理器干活）：                    编译后（编译器看到的）：
┌────────────────────────────┐           ┌────────────────────────────┐
│ 你写的：                     │           │ 真正的 class（宏被展开了）： │
│ DECLARE_..._DELEGATE_       │  展开     │ class FOnLyraTeamIndexed    │
│   ThreeParams(              │ ───────►  │     ChangedDelegate : ... { │
│   FOnLyraTeamIndexed...,    │  文本替换  │     void AddDynamic(...);   │
│   参数...)                  │           │     void Broadcast(...);    │
│                            │           │     TArray<...> 名单;        │
│ 看起来像"神秘一行"           │           │ };                          │
└────────────────────────────┘           └────────────────────────────┘
      宏（工具）在干活                        类型（产品）诞生了
```

**关键**：宏只在**编译前**存在，展开完就消失了；留下的是它造出来的那个 class（类型）。所以你事后搜不到宏，只能搜到"用它造出来的类型"被到处使用。

---

## 四、再举两个例子，你就彻底通了

这种"宏造类型"的模式，UE 里到处都是：

| 宏（工具） | 造出来的类型（产品） |
|---|---|
| `DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLyra..., ...)` | `FOnLyraTeamIndexChangedDelegate` |
| `UCLASS()` + `GENERATED_BODY()` | 一个能被反射的 UClass |
| `UPROPERTY()` | 一个能被蓝图识别的属性 |
| `DECLARE_EVENT_TwoParams(FOnHealth, ...)` | `FOnHealth`（一个事件类型） |

**规律**：凡是看到 `DECLARE_..._DELEGATE_xxx(某个名字, ...)`，那个"某个名字"就是**这次宏调用要生产的类型**。

---

## 五、一张图记住三者关系

```
        【宏 = 工具】              【调用 = 动作】           【类型 = 产品】
   ┌─────────────────────┐    ┌──────────────────┐    ┌──────────────────────┐
   │ DECLARE_DYNAMIC_    │    │ 把"类名+参数"    │    │ FOnLyraTeamIndexed   │
   │ MULTICAST_DELEGATE_ │ ◄──│ 喂给宏           │ ──►│ ChangedDelegate      │
   │ ThreeParams         │    │ (第15行那一行)   │    │ (一个能用的class)     │
   │ (模具/打印机)        │    │                  │    │                      │
   └─────────────────────┘    └──────────────────┘    └──────────────────────┘
          工具                      动作                     结果

   记忆：宏是"怎么做"，类型是"做出什么"。
```

---

## 六、常见误区

| 误区 | 正确理解 |
|------|---------|
| "`FOnLyra...` 是个宏" | ❌ 它是宏**生成的类型**，不是宏本身 |
| "宏和类型是一回事" | ❌ 宏是工具(动词)，类型是结果(名词) |
| "宏运行后变成类型" | ⚠️ 更准确：宏在**编译前**被文本替换，展开成一个 class |
| "能找到这个类型的 .cpp 源文件" | ❌ 找不到，它是宏拼的，编译完没有独立源文件 |
| "每个类型都是手写的" | ❌ UE 里很多类型（委托、反射类）都是宏生成的 |

---

## 七、总结

```
Q：不是 DECLARE_..._DELEGATE 才是宏吗？FOnLyra... 跟宏啥关系？

A：
  • DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams = 宏（工具/模具/打印机）
  • FOnLyraTeamIndexChangedDelegate               = 类型（产品/饼干/打印的合同）

  关系：调用一次宏 → 生产出一个类型。
        宏是"动词/动作"，类型是"名词/结果"。

  严格说法：
    ❌ "FOnLyra... 是个宏"
    ✅ "FOnLyra... 是宏生成的类型"

  那行为什么难懂：
    你看到的是"产品名"(F开头像普通类型)，
    但它的"生产工具"(宏)藏在另一行，
    所以光看名字不知道它哪来的 —— 得搜 DECLARE_..._DELEGATE(它的名) 才找到出生地。
```

**一句话**：你没看错——**`DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams` 才是宏（工具），`FOnLyraTeamIndexChangedDelegate` 是它造出来的类型（产品）**。宏是"动词"，类型是"名词"；你喂一行参数给宏，它就吐出一个能用的 class。`FOnLyra...` 本身不是宏，但它"出身于宏"，所以找不到手写的 class 源文件——这正是 UE 用宏造类型的劝退之处。

---

## 八、下一步

- [26_宏Macro详解_从define到UE委托宏](./26_宏Macro详解_从define到UE委托宏.md) — 宏的根本定义
- [27_宏为什么长得像函数_去引擎里追宏源码实战](./27_宏为什么长得像函数_去引擎里追宏源码实战.md) — 怎么追宏、看展开
- [32_类型vs对象_为什么OnTeamChangedDelegate啥都没有却能AddDynamic](./32_类型vs对象_为什么OnTeamChangedDelegate啥都没有却能AddDynamic.md) — 类型 vs 对象
