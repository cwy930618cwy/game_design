# 线A-1 GameplayTagStack —— 总 md

> **定位**：阶段三（L3 系统基础）**线 A 第 1 项**的**总入口 + 全景**。
> 目标：理解并一比一还原 Lyra 的 `System\GameplayTagStack.h/.cpp`——一个"可复制的、带计数的 Tag 容器"。
> 源码依据：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\System\GameplayTagStack.h/.cpp`
> 命名说明：这个文件在 Lyra 里就叫 `GameplayTagStack`（**没有 `Lyra` 前缀**），类名是 `FGameplayTagStack` / `FGameplayTagStackContainer`，所以**不涉及前缀替换，保持原名**。

---

## 一、为什么先讲它（解决什么问题）

回到阶段三全景：L3 要给游戏几个"全局单例 + 全局数据"。但全局数据里有一种**很特殊、又很常用**的东西——**带计数的 Tag**。

举例你要做的游戏里可能出现的需求：
- 一个敌人身上叠了 3 层"燃烧"（需要记录 `燃烧` 这个 Tag 有 3 层）。
- 玩家身上有"无敌"状态（`无敌` Tag 有 1 层，够 1 次就不算）。
- 某机制需要**复制到所有客户端**（别的客户端也要看到"这有 3 层燃烧"）。

`GameplayTagStack` 干的就是这件事：**把"Tag + 数量"打包成一个可复制的小容器**，能加层、减层、查询层数，还能自动同步到网络。

> 一句话：**GameplayTagStack = 一个可复制（联网同步）的"Tag 计数器"**。

---

## 二、为什么放这里、为什么独立

- **放 L3 系统基础**：它是被上层（角色、战斗、效果）反复使用的"数据底座"，和 GameInstance/AssetManager 一起，都属于"全局/底层数据"那一类。
- **完全独立**：它只依赖引擎的 `GameplayTags`（Tag 类型）+ `FastArraySerializer`（网络复制工具），**不依赖阶段四/五的任何类**。所以放在线 A 第一项，现在就能编译、能讲透，适合热身。

---

## 三、全景清单（这一步要还原的两个结构）

依据 Lyra 真实源码，`GameplayTagStack.h` 里定义了 **2 个 USTRUCT**（`.cpp` 里实现它们的方法）：

### 1. `FGameplayTagStack`（一个"Tag + 数量"的条目）
| 内容 | 说明 |
|------|------|
| 继承 | `FFastArraySerializerItem`（FastArray 复制框架的"条目"基类） |
| 字段 | `FGameplayTag Tag`（哪个 Tag）、`int32 StackCount`（几层） |
| 方法 | `GetDebugString()`（调试打印成 "tagx数量" 字符串） |
| 备注 | 两个字段都是 `private`，只对 `FGameplayTagStackContainer` 开放（`friend`） |

### 2. `FGameplayTagStackContainer`（装一堆条目的容器）
| 内容 | 说明 |
|------|------|
| 继承 | `FFastArraySerializer`（FastArray 复制框架的"容器"基类） |
| 字段 | `Stacks`（可复制的条目数组）、`TagToCountMap`（查询用的加速 Map） |
| 方法 | `AddStack`（加层）、`RemoveStack`（减层）、`GetStackCount`（查层数）、`ContainsTag`（是否含该 Tag） |
| 复制 | `NetDeltaSerialize`（FastArray 增量复制）+ `PreReplicatedRemove` / `PostReplicatedAdd` / `PostReplicatedChange`（复制回调）+ `TStructOpsTypeTraits`（开启增量复制） |

---

## 四、关键概念预告（详细原理到对应步骤再展开）

这一步会反复用到几个底层概念，**先知道是干嘛的，具体机制教到那一步再深讲**：
- **`FGameplayTag`**：引擎的"标签"类型（阶段一已教过 GameplayTags，这里复习应用）。
- **`FFastArraySerializer`**：引擎的"数组增量复制"框架——网络同步时只传变化的部分，不整包传。这是本文件的核心机制。
- **`USTRUCT`**：UE 的"结构体"（非 UObject，不自己分配内存，可被蓝图用）。
- **`NetDeltaSerialize` / `TStructOpsTypeTraits`**：让这个结构体拥有"增量复制"能力的关键开关。

---

## 五、教学节奏（铁律 20/29）

按铁律串行走，一步一反馈：
1. ✅ 本总 md（为什么 + 全景）——**现在在这里，等你确认**
2. ⏳ 教 `.h`（建 `..._h.md`）
3. ⏳ 教 `.cpp`（建 `..._cpp.md`）

> **注意**：本文件**不涉及前缀替换**（Lyra 里就叫 `GameplayTagStack`），代码一比一照抄即可。注释会中文化（铁律 21/28）。

---

## 六、待确认

1. **从 `GameplayTagStack` 开始**，OK 吗？
2. 确认后，我先教 `.h`（建 md），你写完 `.h` 说"下一步"（我会先读你的代码确认写完），再教 `.cpp`。