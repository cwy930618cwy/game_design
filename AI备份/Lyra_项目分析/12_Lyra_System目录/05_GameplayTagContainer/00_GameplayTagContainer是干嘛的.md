# GameplayTagContainer 是干嘛的

> 源码位置（本机引擎）：`d:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\GameplayTags\Classes\GameplayTagContainer.h`
> 本文所有行号都指这个文件，方便你对着看。
> 看完你就懂：**它是"一袋标签"，专门用来做"是不是拥有某些标签"的判断**。

---

## 0. 先说人话：它和一个 Tag 是什么关系

`FGameplayTag` 是**一个**标签，比如 `Status.Debuff.Poison`。
`FGameplayTagContainer` 是**一袋**标签，里面可以装很多个。

```
 FGameplayTag            → 一个标签        （像一张卡片）
 FGameplayTagContainer   → 一袋标签        （像一个卡包）
```

**为什么需要一袋？** 因为游戏里的判断大多不是"你有没有这一个标签"，而是：

- "你有**任意一个**负面状态吗？" → 要看一堆标签里有没有命中
- "你**同时**满足无敌、霸体、不可推动这三个标签吗？" → 要看一堆标签是否全都有

单个 Tag 只能回答"我是不是它"，回答不了"这堆里有没有/是不是全都有"。所以需要一个容器来装一堆，并提供批量判断的方法。

一句话：**Tag 是名词，Container 是"名词的集合 + 一套集合运算"**。

---

## 1. 它肚子里装了什么（第 695～701 行）

```cpp
/** Array of gameplay tags */
TArray<FGameplayTag> GameplayTags;      // 你显式加进去的标签

/** Array of expanded parent tags... Used to accelerate parent searches. */
TArray<FGameplayTag> ParentTags;        // 父标签缓存（自动算出来的，为了查得快）
```

重点在第 2 个数组。为什么还要存一份"父标签"？这就要讲到它最核心的概念：**父子层级**。

---

## 2. 最核心的概念：加了子标签，等于也拥有了父标签

标签名字是用点分层的，比如：

```
 Status
 └─ Debuff
     └─ Poison        ← 你实际加进去的是这个
```

当你往容器里 `Add` 一个 `Status.Debuff.Poison` 时，UE 会认为**你同时拥有**：

```
 Status.Debuff.Poison   （你加的）
 Status.Debuff          （父，自动算出来塞进 ParentTags）
 Status                 （爷爷，也塞进去）
```

这件事的意义是**让上层代码可以写得很粗**：
写"免疫所有 Debuff"的逻辑时，只需判断 `Status.Debuff`，不需要把中毒、流血、减速一个个列出来。

对应到代码里，`HasTag` 就是"显式列表 + 父标签列表"两个都查（第 313～321 行）：

```cpp
bool HasTag(const FGameplayTag& TagToCheck) const
{
    // Check explicit and parent tag list
    return GameplayTags.Contains(TagToCheck) || ParentTags.Contains(TagToCheck);
}
```

而 `HasTagExact` 只查显式列表（第 330～338 行），**不认父标签**：

```cpp
bool HasTagExact(const FGameplayTag& TagToCheck) const
{
    // Only check explicit tag list
    return GameplayTags.Contains(TagToCheck);
}
```

源码注释里给了现成的例子（第 306～309 行）：

| 容器里有 | 查询 | `HasTag` | `HasTagExact` |
|---|---|---|---|
| `A.1` | `A` | **true**（父匹配） | false（不是精确） |
| `A` | `A.1` | false（子不等于父） | false |

**这个"父匹配"是这类标签体系的关键，也是最容易用错的地方。**

---

## 3. 四个查询 API，看懂"Exact"和"空"的区别

### 3.1 单个标签：`HasTag` / `HasTagExact`
上面讲过：一个认父、一个只认精确。

### 3.2 一个就够：`HasAny` / `HasAnyExact`（第 347、370 行）
用法：`容器A.HasAny(容器B)` = "B 里**任意一个**标签，我有没有？"

```
 {"A.1"}.HasAny({"A","B"})   → true （A.1 的父 A 命中）
 {"A"}.HasAny({"A.1","B"})   → false
```

### 3.3 全都要：`HasAll` / `HasAllExact`（第 393、416 行）
用法：`容器A.HasAll(容器B)` = "B 里的标签我**是不是全都有**？"

### 3.4 ⚠️ 空容器时的"反直觉"行为（务必记住）

源码注释和实现写得很明确：

| 调用 | 传空容器时结果 | 为什么 |
|---|---|---|
| `HasAny(空)` | **false** | 空容器里挑不出任何一个能命中 |
| `HasAll(空)` | **true** | 一个都没失败，所以算"全都通过" |

所以如果你要写"配置了条件才判断"，一定先判 `IsEmpty()`，否则空条件会被当成"全部满足"，逻辑就穿帮了。

常用的还有三个"看规模"的函数（第 432～448 行）：`Num()`（几个标签）、`IsValid()`（有没有标签）、`IsEmpty()`（是不是空的）。

---

## 4. 增删改的方法（按用途记）

### 4.1 加

| 方法 | 干什么 | 什么时候用 |
|---|---|---|
| `AddTag`（第 508 行） | 加一个，**会去重**，并更新父标签缓存 | 常规添加 |
| `AddTagFast`（第 517 行） | 加一个，**不查重复** | 自己确定不会重复时，批量构建更快 |
| `AddLeafTag`（第 526 行） | 加子标签，并**顺手删掉它的直接父标签** | 避免容器里同时存在父子两个标签 |
| `AppendTags`（第 486 行） | 把另一个容器的标签全倒进来（相当于并集） | 合并两袋标签 |
| `CreateFromArray`（第 292 行） | 直接从数组一次建好容器 | 比一个个 `AddTag` 高效 |

### 4.2 减

| 方法 | 干什么 |
|---|---|
| `RemoveTag`（第 534 行） | 删一个（还有个 `bDeferParentTags` 参数，跳过父标签重算，为了性能） |
| `RemoveTags`（第 541 行） | 把另一个容器里的标签都删掉 |
| `Reset`（第 544 行） | 清空 |

---

## 5. 集合运算：Filter（交集）

- `Filter`（第 460 行）：取"我"和"另一个容器"的**交集**，**但会做父匹配**（`Color.Green` 和 `Color.Red` 会因为共同的父 `Color` 而算命中）。
- `FilterExact`（第 469 行）：只取**精确相同**的交集。

源码在 `AppendMatchingTags` 的注释里专门警告了这件事（第 488～500 行）：**想要严格交集就用 `FilterExact`**，否则父匹配会让你意外命中。

还有一个更高级的：`MatchesQuery`（第 478 行），配合 `FGameplayTagQuery` 能表达"A 且（B 或 C）"这类复杂组合条件，Ability 的激活条件那一套就靠它。

---

## 6. 怎么遍历、怎么打印

```cpp
// 遍历（内部实现了 begin/end，可以直接 for-each，第 717～718 行）
for (const FGameplayTag& T : MyContainer) { ... }

// 拿出数组（第 580 行）
const TArray<FGameplayTag>& Arr = MyContainer.GetGameplayTagArray();

// 打印（第 559、565 行）
FString S = MyContainer.ToString();        // 带括号的完整形式
FString S2 = MyContainer.ToStringSimple(); // 简洁一点
```

---

## 7. 网络同步和值语义（做联机要知道的）

- 它是 `USTRUCT(BlueprintType)`（第 258 行）：**蓝图里能当变量**，能存盘（`SaveGame`）。
- 特性表里写着 `WithNetSerializer = true`（第 730～744 行）：说明它有专门的网络序列化。
- `NetSerialize` 上面那句注释是重点（第 549 行）："**Efficient network serialize, takes advantage of the dictionary**"——它同步时不发标签名字符串，而是利用标签字典发编号，省流量。
- 它是**值类型**：赋值 = 复制整袋（两个数组都拷），所以函数传参尽量写 `const FGameplayTagContainer&`。

---

## 8. 在 Lyra 里它被用来干嘛（搜源码的真实例子）

Lyra 到处都是它，套路都是"**用标签袋当过滤条件**"：

| 文件 | 成员 | 用途 |
|---|---|---|
| `LyraAbilityTagRelationshipMapping.h` | `AbilityTagsToBlock` / `AbilityTagsToCancel` / `ActivationRequiredTags` / `ActivationBlockedTags` | 能力激活的"需要什么/被什么禁止" |
| `LyraGameplayAbility.h` | `FailureTags` | 能力失败时挂的标签 |
| `LyraTaggedActor.h` | `StaticGameplayTags` | 给关卡里的 Actor 打标签，配合 `GetOwnedGameplayTags` 供别人查询 |
| `LyraTaggedWidget.h` | `HiddenByTags` | 玩家有这些标签就把 UI 藏起来 |
| `LyraPlayerStart.h` | `StartPointTags` | 出生点打标签，按玩法筛选用哪个出生点 |
| `PhysicalMaterialWithTags.h` | `Tags` | 给物理材质打标签，用来区分草地/金属的脚步声 |
| `LyraVerbMessage.h` | `InstigatorTags` / `TargetTags` / `ContextTags` | 消息（比如击杀播报）带上一堆标签当上下文 |
| `LyraContextEffectComponent.h` | `CurrentContexts` | 当前环境标签，决定播哪套特效音效 |

可以看出它的典型用法：**不是"我是什么"，而是"我在什么条件下该怎么样"**——判断条件时一句 `HasAny(条件袋)` 或 `HasAll(条件袋)` 就完事。

---

## 9. 和上一篇 `FGameplayTagStack` 的关系

| | `FGameplayTagContainer` | `FGameplayTagStackContainer` |
|---|---|---|
| 回答的问题 | **有没有**这些标签 | 某个标签**有几层** |
| 内部 | 一堆 `FGameplayTag` + 父标签缓存 | 一堆 `(Tag, 层数)` 记录 + 查询表 |
| 典型场景 | 激活条件、UI 显隐、音效选择 | 层数类状态：中毒叠 3 层、物品充能 |

两者都会做"标签集合"这件事，但**前者管有无，后者管数量**。

---

## 10. 新手最容易踩的 5 个坑

1. **拿 `HasTag` 当精确判断用**：它认父标签。要"就是它本身"必须用 `HasTagExact`。
2. **`HasAll(空)` 返回 true**：空条件被当成"全部满足"。先 `IsEmpty()` 再判断。
3. **`Filter` 不是严格交集**：它会因为共同的父标签而命中，要严格就用 `FilterExact`。
4. **一个个 `AddTag` 建大容器**：`AddTag` 每次要去重 + 更新父标签，量大时慢，改用 `CreateFromArray` 或 `AddTagFast`。
5. **按值传参**：容器里有两个 `TArray`，拷贝不便宜，传参加 `const&`。

---

## 11. 一句话总结

`FGameplayTagContainer` = **一袋 `FGameplayTag` + 父子层级缓存 + 一套集合判断方法（有没有 / 任意一个 / 全都有）+ 高效网络同步**。
你用它写"条件"，而不是写"一堆 if 比较字符串"。
记住三件事就够日常用：**`HasTag` vs `HasTagExact` 的区别、`HasAny`/`HasAll` 的用法、空容器那两条反直觉规则。**
