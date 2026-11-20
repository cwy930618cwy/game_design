# 从头讲：FGameplayTagStack.h 到底在干嘛

> 文件：`LyraStarterGame/Source/LyraGame/System/GameplayTagStack.h`
> 这篇只讲 `.h`（头文件），里面的函数「怎么做」写在 `.cpp` 里，这里不贴 `.cpp` 的代码。
> 写法是：**先讲人话，再回去看代码长什么样**。

---

## 0. 先别管代码，讲个打游戏的场景

你玩一个角色扮演游戏，角色身上可能同时挂着这些东西：

- "加速" 叠了 **3 层**
- "中毒" 叠了 **2 层**
- "无敌" 叠了 **1 层**

程序里怎么记？其实就是一张小清单：

```
 加速 → 3
 中毒 → 2
 无敌 → 1
```

问题有三个：

1. **"加速"这个名字用什么表示？**
   答案：UE 里现成的 `GameplayTag`（比如 `Status.Speed` 这种标签）。它比"随便写个字符串"安全，写错了编译期/运行期就能发现。
2. **这个清单放哪儿？** 放在某个游戏对象身上。
3. **联机的时候怎么让别人的电脑上也有一份一样的清单？**

**这个头文件就是 Epic 给出的标准答案**：用两个结构，一个存"一条"，一个存"一整本清单"，并且顺手把网络同步这件事也解决了。

所以你打开这个文件只会看到两个东西：

| 名字 | 类比 | 作用 |
|---|---|---|
| `FGameplayTagStack` | 一条笔记 | 一个标签 + 它的层数 |
| `FGameplayTagStackContainer` | 整本笔记本 | 装很多条笔记，能加能减能查，还能联网 |

---

## 1. 一条笔记：`FGameplayTagStack`

它在文件里的核心就两行（第 35～39 行）：

```
 FGameplayTag Tag;          // 记哪个标签
 int32 StackCount = 0;      // 这个标签有几层
```

一条笔记的内容就这么朴素。但它周围有 3 个地方，是故意这么设计的：

### ① 它是 `private` 的，只 `friend` 给笔记本（第 32～33 行）

```cpp
private:
    friend FGameplayTagStackContainer;
```

**为什么？** 因为"改层数"这件事不是你自己偷偷改一下就完了——改完还得**通知网络上的其他人**（别人的电脑也要更新成一样的数）。

所以笔记本把规则定成：**别自己改，要改就来找我（容器）**。你调容器的 `AddStack`，容器改完之后就会顺手把"变化"发给大家。你要是绕过容器直接改数字，网络上就乱套了。

> 生活类比：公司报销，你不能自己拿笔改账本上的数字，得走财务（容器）。财务改完会同步给所有部门。

### ② 它继承了一个奇怪名字的类（第 17 行）

```cpp
struct FGameplayTagStack : public FFastArraySerializerItem
```

`FFastArraySerializerItem` 你可以理解成一张**"能进网络同步名单的资格证"**。
普通的 `USTRUCT` 只是普通结构，没这张证；没证的东西，UE 的"增量同步机制"不管它。有了这张证，笔记本才能把这些条目放到"会自动同步的名单"里。

### ③ 它有两个构造函数（第 21～28 行）

一个空的（网络把数据从别人那里收回来、需要现场造一个对象时用），一个带参数的（自己写代码时方便）：

```cpp
FGameplayTagStack(FGameplayTag InTag, int32 InStackCount)
```

还有个 `GetDebugString()`（第 30 行），纯粹是打日志/调试时把这条笔记变成人能读的字符串，具体内容在 `.cpp`。

---

## 2. 整本笔记本：`FGameplayTagStackContainer`

这是这个文件真正的重点。它可以分成四块来看。

### 2.1 给外面用的 4 个方法（第 55～70 行）

```cpp
void AddStack(FGameplayTag Tag, int32 StackCount);     // 加层
void RemoveStack(FGameplayTag Tag, int32 StackCount);  // 减层
int32 GetStackCount(FGameplayTag Tag) const;           // 查层数（没有就返回 0）
bool ContainsTag(FGameplayTag Tag) const;              // 查有没有这个标签
```

**这就是你用这个类需要的全部知识**：加、减、查数、查有无。
（加和减的具体实现写在 `.cpp` 里，本篇不展开。）

注意 `GetStackCount` 和 `ContainsTag` 的写法，它们不是去翻数组，而是去查一张"表"：

```cpp
return TagToCountMap.FindRef(Tag);   // 查表，非常快
```

为什么要这么做？往下看第 3 点就明白了。

### 2.2 它内部存了**两份**数据（第 85～89 行）

```cpp
TArray<FGameplayTagStack> Stacks;          // 第一份：数组，一条一条排好
TMap<FGameplayTag, int32> TagToCountMap;   // 第二份：哈希表，标签 → 层数
```

一个东西存两份，新手看着别扭，但这是**整个设计的精髓**：

- **数组 `Stacks` 是"真身"**：它长得规规矩矩，适合"只把变化的部分发给别人"。
- **哈希表 `TagToCountMap` 是"影子/字典"**：它查起来飞快，适合 `GetStackCount` 这种高频查询。

那为什么不只用一份？

- 只用数组：每次查层数要从头找到尾，慢。
- 只用哈希表：它没有"我是第几条、哪条被改了"这种结构，网络同步不好做。

所以：**数组负责"和网络打交道"，哈希表负责"给人查"，两个配合用。**

> 生活类比：图书馆有两样东西——**书架**（书摆成一排排，方便盘点、进出库）和**电脑检索目录**（查某本书快）。
> 搬进来一本书，书架更新了，目录也得跟着更新，否则查不到。

### 2.3 谁来保证"两份数据一致"？三个回调（第 72～76 行）

```cpp
void PreReplicatedRemove(...);   // 有人删了一条
void PostReplicatedAdd(...);     // 有人加了一条
void PostReplicatedChange(...);  // 某条的数量变了
```

**什么时候会被调用？** 当别的电脑（服务器）把变化同步到你这台机器上时，引擎框架会自动喊这三个函数。

**喊你干嘛？** 就一件事：**更新影子表 `TagToCountMap`**。

流程是这样的：

```
 服务器：AddStack("加速", 1)
    │  ① 改数组 Stacks（真身变了）
    │  ② 改自己的 TagToCountMap
    ▼
 引擎自动发现数组变了 → 只把"加速 变成 4 层"这一条打包发出去（不发整本）
    ▼
 客户端：收到 → 引擎喊 PostReplicatedAdd / PostReplicatedChange
    │  ③ 客户端在这个回调里更新自己的 TagToCountMap
    ▼
 客户端：GetStackCount("加速") 立刻能查到 4
```

（这三个函数的具体实现也在 `.cpp` 里，本篇不贴。）

### 2.4 那行"开关"：告诉引擎"我支持增量同步"（第 78～99 行）

```cpp
bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
    return FFastArraySerializer::FastArrayDeltaSerialize<FGameplayTagStack, FGameplayTagStackContainer>(Stacks, DeltaParms, *this);
}

// 文件最后：
template<> struct TStructOpsTypeTraits<FGameplayTagStackContainer> ...
{
    enum { WithNetDeltaSerializer = true };
};
```

这两段是**固定套路，照着抄就行**，你不需要背：

- `NetDeltaSerialize`：告诉引擎"同步这个容器时，走我指定的这套逻辑（只传变化）"。
- `WithNetDeltaSerializer = true`：这是**总开关**。没它，前面所有网络打算都作废。

**"增量同步"是什么意思？**
打个比方：清单有 100 条，你只改了 1 条。笨办法是把 100 条全部重发一遍；增量同步只发"第 37 条从 3 变成 4"。联机时这能省下大量流量。

---

## 3. 实际用起来长什么样

假设你有个角色类，想给他挂一份"状态层数清单"，大概就是这样：

```cpp
// 声明（这是用法示例，不是本文件里的代码）
FGameplayTagStackContainer MyStacks;

// 加 3 层"加速"
MyStacks.AddStack(SpeedTag, 3);

// 现在是几层？
int32 N = MyStacks.GetStackCount(SpeedTag);   // 3

// 减 1 层
MyStacks.RemoveStack(SpeedTag, 1);

// 有没有"中毒"？
bool b = MyStacks.ContainsTag(PoisonTag);
```

在 Lyra 里，真正用它的地方有三个（我查过源码）：

| 谁在用 | 拿来记什么 |
|---|---|
| `LyraPlayerState` | 玩家身上的各种标签层数 |
| `LyraInventoryItemInstance` | 物品实例上的标签层数 |
| `LyraTeamInfoBase` | 队伍信息上的标签层数 |

---

## 4. 记不住就记这 4 句

1. 这个文件就两个结构：**一条笔记**（Tag+层数）和**一本笔记本**（装笔记、能同步）。
2. 一条笔记的数据是私有的，**改必须走笔记本**，因为改完要通知网络。
3. 笔记本内部**存两份**：数组负责同步、哈希表负责查询，靠三个回调保持一致。
4. 你要在外面用，只会碰到 4 个方法：`AddStack` / `RemoveStack` / `GetStackCount` / `ContainsTag`。
