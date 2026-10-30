# P08 Inventory —— 阶段总 md

> **定位**：P08 Inventory 阶段的**目录/路线图**（这一节有哪些类、各依赖什么、先教谁）。
> 具体某一个类怎么教，由**该类自己的总 md** 负责（铁律 35：上级总 md 不能替代子类总 md）。
>
> **建立日期**：2026-10-29（P07 Teams 全部完成后进入）
> **阶段状态**：⬜ 待教（第一个类的总 md 待建）

---

## 一、这一节要解决什么

P05 搭好 GAS、P06 把 GAS 接到角色上、P07 加了阵营。
现在能打能死了，但**手里的枪是哪来的？子弹怎么算？捡到的东西放哪？**

Lyra 的答案是 **Inventory（物品栏）**：

- **一把枪是什么** → 物品定义（`InventoryItemDefinition`），纯数据资产，蓝图可配
- **我手里这把具体的枪**（有耐久、有弹药数） → 物品实例（`InventoryItemInstance`），运行时对象
- **我背包里有哪些东西** → 物品管理器组件（`InventoryManagerComponent`），挂在角色上、会复制
- **地上那个能捡的东西** → 拾取接口（`IPickupable`）
- **物品长什么样、装备时用什么、初始属性多少** → 物品碎片（`InventoryFragment_*`）

一句话：**P08 = 给游戏加"物品"这一层，用「定义（不变模板）+ 实例（可变个体）+ 碎片（可插拔特性）」三件套**。

### 为什么需要"定义 vs 实例"分开？

| | 定义（Definition） | 实例（Instance） |
|---|------------------|-----------------|
| 是什么 | **CDO（类默认对象）**，一份 | **运行时 NewObject**，每个物品一个 |
| 存什么 | 名字、图标、可装备性、初始属性（**所有同类物品都一样**） | 当前弹药、当前耐久（**每件都不一样**） |
| 会不会变 | ❌ 永不变（`UCLASS(Const)`） | ✅ 随时变 |
| 复制吗 | ❌ 不复制（靠类引用同步） | ✅ 复制 |
| 类比 | **菜谱** | **做出来的那盘菜** |

> 💡 100 个人各拿一把 AK，**定义只有 1 份**（AK 菜谱），**实例有 100 份**（每把枪弹药不同）。
> 如果合并成一个类，要么浪费内存（100 份重复数据），要么没法存个体状态。

---

## 二、全景：Lyra `Inventory/` 文件夹 16 个文件（9 个类/结构组）

对照 `E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Inventory\`：

| 编号 | Lyra 文件 | 工程文件（待建） | 行数（h/cpp） | 依赖 | 挂起 |
|------|----------|-----------------|--------------|------|------|
| **P08.1** | `LyraInventoryItemDefinition` | `TenkaichiInventoryItemDefinition.h/.cpp` | 57 / 45 | 引擎 `UBlueprintFunctionLibrary` | ✅ **零** |
| **P08.2** | `LyraInventoryItemInstance` | `TenkaichiInventoryItemInstance.h/.cpp` | 79 / 74 | P08.1 + **P02 `GameplayTagStack`** | ✅ **零** |
| **P08.3** | `InventoryFragment_SetStats` | `InventoryFragment_SetStats.h/.cpp` | 27 / 25 | P08.1 + P08.2 | ✅ **零** |
| **P08.4** | `InventoryFragment_EquippableItem` | `InventoryFragment_EquippableItem.h/.cpp` | 21 / 6 | P08.1（P09 仅前置声明） | ✅ **零** |
| **P08.5** | `InventoryFragment_PickupIcon` | `InventoryFragment_PickupIcon.h/.cpp` | 29 / 10 | P08.1 | ✅ **零** |
| **P08.6** | `InventoryFragment_QuickBarIcon` | `InventoryFragment_QuickBarIcon.h/.cpp` | 26 / 6 | P08.1 | ✅ **零** |
| **P08.7** | `LyraInventoryManagerComponent` | `TenkaichiInventoryManagerComponent.h/.cpp` | 173 / 320 | P08.1 + P08.2 + **GameplayMessageRouter 插件** | ✅ **零** |
| **P08.8** | `IPickupable` | `IPickupable.h/.cpp` | 89 / 54 | P08.7 | ✅ **零** |

> 🎉 **全阶段 16 文件、零挂起** —— 这是继 P00 Physics / P04 Camera 之后**第三个「纯叶子」阶段**。

### 单个文件里的类/结构清单

| 文件 | 里面有什么 |
|------|-----------|
| `TenkaichiInventoryItemDefinition` | ① `UTenkaichiInventoryItemFragment`（抽象碎片基类）② `UTenkaichiInventoryItemDefinition`（定义）③ `UTenkaichiInventoryFunctionLibrary`（蓝图工具库） |
| `TenkaichiInventoryItemInstance` | `UTenkaichiInventoryItemInstance`（实例） |
| `InventoryFragment_SetStats` | `UInventoryFragment_SetStats`（初始属性） |
| `InventoryFragment_EquippableItem` | `UInventoryFragment_EquippableItem`（可装备） |
| `InventoryFragment_PickupIcon` | `UInventoryFragment_PickupIcon`（掉落物外观） |
| `InventoryFragment_QuickBarIcon` | `UInventoryFragment_QuickBarIcon`（快捷栏图标） |
| `TenkaichiInventoryManagerComponent` | ① `FTenkaichiInventoryChangeMessage` ② `FTenkaichiInventoryEntry` ③ `FTenkaichiInventoryList` ④ `UTenkaichiInventoryManagerComponent` |
| `IPickupable` | ① `FPickupTemplate` ② `FPickupInstance` ③ `FInventoryPickup` ④ `UPickupable`+`IPickupable` ⑤ `UPickupableStatics` |

> 📌 **命名注意**：4 个 `InventoryFragment_*` 和 `IPickupable` **Lyra 原文就没有 `Lyra` 前缀**
> → 按铁律 24 机械替换规则，**保持原样**（和 P07.9/P07.10 的 `AsyncAction_*` 同理）。
> 另外 3 个有 `Lyra` 前缀 → 换成 `Tenkaichi`。

---

## 三、依赖分析（P08 全阶段，铁律 30/31）

统一方向依赖树（被依赖在下）：

```
IPickupable（P08.8，拾取接口）                        ← 最顶
   │ 依赖
   └── TenkaichiInventoryManagerComponent（P08.7）
          │ 依赖
          ├── TenkaichiInventoryItemInstance（P08.2）
          │     └── TenkaichiInventoryItemDefinition（P08.1）
          ├── TenkaichiInventoryItemDefinition（P08.1）
          ├── GameplayMessageSubsystem（GameplayMessageRouter 插件 ✅ 已就位）
          └── NativeGameplayTags.h（引擎 GameplayTags 模块 ✅）

InventoryFragment_SetStats（P08.3）
   │ 依赖
   ├── TenkaichiInventoryItemDefinition（P08.1）
   └── TenkaichiInventoryItemInstance（P08.2）  ← .cpp 里硬调用 AddStatTagStack

InventoryFragment_EquippableItem（P08.4）
   │ 依赖
   ├── TenkaichiInventoryItemDefinition（P08.1）
   └── UTenkaichiEquipmentDefinition（P09）⚠️ 仅前置声明！不阻塞编译

InventoryFragment_PickupIcon（P08.5）  → P08.1 + 引擎 USkeletalMesh
InventoryFragment_QuickBarIcon（P08.6）→ P08.1 + 引擎 FSlateBrush

TenkaichiInventoryItemInstance（P08.2）
   │ 依赖
   ├── TenkaichiInventoryItemDefinition（P08.1）  ← .cpp 里 GetDefault<>()
   ├── System/GameplayTagStack.h（P02 ✅ 已落盘）
   └── Net/UnrealNetwork.h（引擎）

TenkaichiInventoryItemDefinition（P08.1）  ← 最底，只依赖引擎
   └── Kismet/BlueprintFunctionLibrary.h（引擎）
```

### 依赖就位情况（逐个核实）

| 依赖 | 状态 | 证据 |
|------|------|------|
| P02 `System/GameplayTagStack.h/.cpp` | ✅ 已落盘 | `E:\ue5\game_design\code\Tenkaichi\Source\TenkaichiGame\System\GameplayTagStack.h` |
| **GameplayMessageRouter 插件** | ✅ **已就位** | `E:\ue5\game_design\code\Tenkaichi\Plugins\GameplayMessageRouter\`（Lyra 项目自带插件，非引擎） |
| `NativeGameplayTags.h`（`UE_DEFINE_GAMEPLAY_TAG_STATIC`） | ✅ 引擎 GameplayTags 模块 | Build.cs 已含 `"GameplayTags"` |
| `Net/Serialization/FastArraySerializer.h` | ✅ 引擎 | P02 已用过同样机制 |
| `Net/UnrealNetwork.h` | ✅ 引擎 | P07.3 已用过 |
| `Styling/SlateBrush.h`（`FSlateBrush`） | ✅ 引擎 Slate | Build.cs 已含 `"Slate"` |
| `USkeletalMesh` | ✅ 引擎 | 前置声明即可 |
| `UTenkaichiEquipmentDefinition`（P09） | ⚠️ 未就位 | **仅 `class U...;` 前置声明**（P08.4 `.h:10`）→ 不阻塞编译 |
| `TenkaichiGameplayAbility_FromEquipment`（P09） | ⚠️ 未就位 | **P08 不引用**（反向：P09 引用 P08） |

> 🔑 **为什么 P08 能零挂起？**
> ① 依赖的 P02 `GameplayTagStack` **早就教完落盘了**
> ② `GameplayMessageRouter` 虽是 Lyra 插件，但**工程 `Plugins\` 里已经有**
> ③ 唯一牵涉 P09 的地方（`InventoryFragment_EquippableItem.h:10`）**只是前置声明**——
>    `TSubclassOf<U>` 的模板参数用前置声明就够了（铁律 30：`.h` 前置声明不阻塞编译）

---

## 四、🔓 阶段开头必做：翻注释备忘录（铁律 34）

P08 是 **⑳ 号挂起的解锁时机之一**。

| 条目 | 内容 | 阻塞点 | 状态 |
|------|------|--------|------|
| **⑳** | `TenkaichiAbilityCost_ItemTagStack.cpp` 整个 `.cpp` | ① P09 `TenkaichiGameplayAbility_FromEquipment` ② **P08 `TenkaichiInventoryItemInstance`** | ⏸ **本阶段只能解一半**（P09 未就位 → 继续挂） |

核实（`Source\TenkaichiGame\AbilitySystem\Abilities\TenkaichiAbilityCost_ItemTagStack.cpp:3`）：

```cpp
#if 0  // ⚠️【注释挂起】依赖 P09 Equipment（TenkaichiGameplayAbility_FromEquipment）+ P08 Inventory（TenkaichiInventoryItemInstance），工程均无此文件 → 编译不过。
#include "Equipment/TenkaichiGameplayAbility_FromEquipment.h"
#include "Inventory/TenkaichiInventoryItemInstance.h"
```

> 📌 **结论**：P08 完成后，`Inventory/TenkaichiInventoryItemInstance.h` 就位，
> 但 `Equipment/TenkaichiGameplayAbility_FromEquipment.h`（P09）**仍不存在** → **⑳ 继续挂起**。
> **等到 P09 Equipment 教完再一起解开 ⑳**（在 P09 阶段开头翻备忘录时提醒）。

---

## 五、教学顺序（铁律 20/29/35）

严格**自下而上**（A 依赖 B 就先教 B）。每个类走「**自己的总 md → `.h` → `.cpp`**」三步，一步一反馈。

| 序 | 编号 | 内容 | 规模 | 为什么排这个位置 |
|----|------|------|------|-----------------|
| 1 | **P08.1** | `TenkaichiInventoryItemDefinition`（含 Fragment 基类 + FunctionLibrary） | `.h` 57 / `.cpp` 45 | **最底**，只依赖引擎；后面 7 个类全依赖它 |
| 2 | **P08.2** | `TenkaichiInventoryItemInstance` | `.h` 79 / `.cpp` 74 | 依赖 P08.1 + P02 TagStack |
| 3 | **P08.3** | `InventoryFragment_SetStats` | `.h` 27 / `.cpp` 25 | 依赖 P08.1 + P08.2（`.cpp` 硬调 `AddStatTagStack`） |
| 4 | **P08.4** | `InventoryFragment_EquippableItem` | `.h` 21 / `.cpp` 6 | 依赖 P08.1（最小，`.cpp` 只有 6 行） |
| 5 | **P08.5** | `InventoryFragment_PickupIcon` | `.h` 29 / `.cpp` 10 | 依赖 P08.1 |
| 6 | **P08.6** | `InventoryFragment_QuickBarIcon` | `.h` 26 / `.cpp` 6 | 依赖 P08.1 |
| 7 | **P08.7** | `TenkaichiInventoryManagerComponent` | `.h` 173 / `.cpp` 320 | ⭐ **本阶段最大**，依赖 P08.1 + P08.2 + 消息插件 |
| 8 | **P08.8** | `IPickupable` | `.h` 89 / `.cpp` 54 | **最顶**，依赖 P08.7 |

> 📌 **4 个 Fragment 排在一起教**（P08.3~P08.6）——它们结构相似（都继承 `UTenkaichiInventoryItemFragment`），
> 连着教能形成对照，避免"教完一个忘了上一个"。

---

## 六、几个要记住的点

1. **「定义 / 实例 / 碎片」是 P08 的灵魂** —— 定义是菜谱（永不变，`UCLASS(Const)`），实例是那盘菜（有个体状态、会复制），碎片是可插拔的配料（`EditInlineNew` + `Instanced`）。
2. **碎片机制靠三个 UCLASS 标记**：`DefaultToInstanced` + `EditInlineNew` + `Abstract`（`LyraInventoryItemDefinition.h:17`）。有了它们，蓝图里才能**往数组里直接添加不同子类的对象**。
3. **`GameplayTagStackContainer` 是 P02 的老朋友** —— 实例用它存"弹药数/耐久"这类**带数量的属性**（`AddStatTagStack` / `GetStatTagStackCount`）。
4. **`FLyraInventoryList` 用 FastArraySerializer 做增量复制** —— 和 P02 `FGameplayTagStackContainer` 同款机制（P08.7 会详细讲）。
5. **P08.7 是 P08 最大的类（493 行）** —— 内含 3 个 USTRUCT + 1 个 UCLASS，是本阶段的重头戏。
6. **P08 零挂起** —— 依赖的 P02 / 消息插件都就位，唯一牵涉 P09 的地方只是前置声明。
7. **⑳ 本阶段不能解** —— 还差 P09 的 `GameplayAbility_FromEquipment`，到 P09 再解。

---

## 七、进度状态（铁律 35/38）

按铁律 35：**每个独立教学内容必须先建它自己的总 md**，上级总 md（本文件）不能替代。

| 编号 | 内容 | 状态 |
|------|------|------|
| P08.1 | `TenkaichiInventoryItemDefinition` | ✅ 三件套完成 |
| P08.2 | `TenkaichiInventoryItemInstance` | ✅ 三件套完成 |
| P08.3 | `InventoryFragment_SetStats` | ✅ 三件套完成 |
| P08.4 | `InventoryFragment_EquippableItem` | ✅ 三件套完成 |
| P08.5 | `InventoryFragment_PickupIcon` | ✅ 三件套完成 |
| P08.6 | `InventoryFragment_QuickBarIcon` | ✅ 三件套完成 |
| P08.7 | `TenkaichiInventoryManagerComponent` | ✅ 三件套完成（2026-10-30） |
| P08.8 | `IPickupable` | ✅ 三件套完成（2026-10-30） |

- ✅ 本阶段总 md 已建（2026-10-29）
- 🎉 **P08 Inventory 阶段全部完成**（8 类 / 16 文件，2026-10-30）

> ⚠️ 命名：文件叫 `P08.8_IPickupable_总md.md`（铁律 37）。⚠️ `IPickupable` Lyra 原文无 `Lyra` 前缀 → 文件名/类名保持原样，不加 `Tenkaichi`。
> 本文件（P08.1）**一个 `.h` 里有 3 个类**（Fragment 基类 + Definition + FunctionLibrary），
> 总 md 要一次讲清这 3 个的关系——**但 `.h` 和 `.cpp` 仍分开教、分开建 md**。
</content>
