# GameplayTagStack 和 GameplayTags 有什么区别

> **疑问来源**：阶段三线 A-1 讲 `GameplayTagStack` 时，对照阶段一已写好的 `TenkaichiGameplayTags.h`，两者名字都带"GameplayTag"，容易混淆。
> **一句话回答**：`GameplayTags.h` 是**"标签本体的注册清单"**；`GameplayTagStack.h` 是**"给标签计数并复制"的容器工具**。一个是"有哪些标签"，一个是"怎么给标签记数"。

---

## 一、两个文件各自是什么（先看各自职责）

### `TenkaichiGameplayTags.h` —— 声明"标签本体的清单"

看用户工程真实源码（`Source\TenkaichiGame\TenkaichiGameplayTags.h`）：

```cpp
namespace TenkaichiGameplayTags
{
	// 声明 Lyra 要用的所有自定义原生标签（Tag）
	TENKAICHIGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_IsDead);
	TENKAICHIGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);
	...
	TENKAICHIGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dead);
	...
}
```

- 它干的事：**把游戏里要用到的每一个具体标签（Tag）注册成全局变量**——`Ability_ActivateFail_IsDead`、`Status_Death_Dead`、`InputTag_Move`……每个都是一个 `FGameplayTag` 类型的**全局对象**。
- 一句话：**它是"标签清单"，定义"这个游戏有哪些标签"。**
- 配套 `.cpp` 里用 `UE_DEFINE_GAMEPLAY_TAG(...)` 给每个标签赋真实名字（比如 `Ability.ActivateFail.IsDead`）。

### `GameplayTagStack.h` —— 定义"给标签计数 + 复制"的容器

看 Lyra 真实源码（`System\GameplayTagStack.h`）：

```cpp
// 一个条目：某个 Tag + 它的层数
USTRUCT(BlueprintType)
struct FGameplayTagStack : public FFastArraySerializerItem
{
	UPROPERTY() FGameplayTag Tag;      // 哪个标签
	UPROPERTY() int32 StackCount = 0;  // 几层
};

// 容器：装一堆 "Tag+数量" 条目，可加层/减层/查询/复制
USTRUCT(BlueprintType)
struct FGameplayTagStackContainer : public FFastArraySerializer
{
	void AddStack(FGameplayTag Tag, int32 StackCount);
	void RemoveStack(FGameplayTag Tag, int32 StackCount);
	int32 GetStackCount(FGameplayTag Tag) const;
	...
};
```

- 它干的事：**不定义任何具体标签**，而是提供一套"给任意标签计数"的工具结构。
- 它的 `Tag` 字段用的是 `FGameplayTag` 类型——**可以装来自 `GameplayTags.h` 的任意标签**。
- 一句话：**它是"计数器工具"，不关心你有啥标签，只负责"记数 + 同步"。**

---

## 二、关键区别对照表

| 维度 | `TenkaichiGameplayTags.h` | `GameplayTagStack.h` |
|------|---------------------------|----------------------|
| **本质** | 标签**本体**的注册清单 | Tag 计数的**容器结构** |
| **定义的东西** | 一堆具体的 `FGameplayTag` 全局对象 | `FGameplayTagStack` / `FGameplayTagStackContainer` 两个结构体 |
| **回答的问题** | "这个游戏有哪些标签？" | "怎么给某个标签记层数并同步网络？" |
| **是否定义具体标签** | ✅ 是（每个 Tag 一个全局变量） | ❌ 否（只是个通用工具） |
| **有没有计数/复制能力** | ❌ 没有（只是标签本身） | ✅ 有（StackCount 计数 + FastArray 复制） |
| **属于哪个文件/阶段** | 阶段一（L1 模块地基） | 阶段三线A-1（L3 系统基础） |
| **是否带 Lyra 前缀** | 有（`TenkaichiGameplayTags`，阶段一前缀替换） | 无（Lyra 里就叫 `GameplayTagStack`） |

---

## 三、它们怎么配合（关系）

**不是替代关系，是"标签 + 计数器"的配合关系：**

```
GameplayTags.h          GameplayTagStack.h
声明了                 提供了"给标签计数"的工具
Status_Death_Dead  →→→  AddStack(Status_Death_Dead, 3)
（标签本体）              （把"死亡"标签叠 3 层）
```

- `GameplayTags.h` 提供**标签**（`Status_Death_Dead` 等）。
- `GameplayTagStack.h` 提供**容器**，往里塞标签并记数。
- 一个 `FGameplayTagStackContainer` 里可以同时装多个不同标签的计数，比如：燃烧 3 层 + 无敌 1 层 + 冰冻 2 层。

---

## 四、什么时候用哪个

| 场景 | 用哪个 |
|------|--------|
| 需要一个标签的"名字/本体"（判断有没有这个状态） | `TenkaichiGameplayTags.h` 里的标签 |
| 需要给标签**叠层数**（3 层燃烧、N 层某 buff） | `GameplayTagStack.h` 的容器 |
| 需要把"叠层数"**复制到客户端**（联网同步） | `GameplayTagStack.h` 的容器（FastArray 复制） |

---

## 五、一句话记忆

> **GameplayTags.h = 标签清单（有哪些标签）；GameplayTagStack.h = 标签计数器（给标签记几层、还带联网同步）。两个配合用，不是一回事。**

---

> 备注：这个疑问已在阶段一 `GameplayTags`（L1）和阶段三 `GameplayTagStack`（L3）都还原后提出，正好是两个文件都实际存在时对照。详细机制（`FGameplayTag` 是什么、`FFastArraySerializer` 增量复制怎么工作）在对应教学步骤里展开。