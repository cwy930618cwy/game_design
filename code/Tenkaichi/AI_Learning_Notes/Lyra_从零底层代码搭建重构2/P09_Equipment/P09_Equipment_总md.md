# P09 Equipment —— 阶段总 md

> **定位**：本文件是 P09 Equipment（装备系统）的**阶段总 md**（为什么 + 全景 + 依赖分析 + 教学顺序）。
> 每个类另有自己的总 md + `.h` + `.cpp` 教学 md（铁律 35）。
>
> **建立时间**：2026-10-30
> **教学状态**：🟡 阶段总 md 已建（停等确认后再建 P09.1 自己的总 md）

---

## 一、本阶段要解决什么

P08 讲完了"物品"（Inventory）——背包里怎么存、怎么管、怎么捡。但**"装备"（Equipment）是另一件事**：

- **物品**（Inventory）：可以有一堆，躺背包里，是"资源"。
- **装备**（Equipment）：**穿在角色身上的东西**，会生成 Actor（比如武器模型）、授予技能（GAS Ability）、挂标签。

P09 就是"把物品变成穿在身上的装备"这套系统。它和 P08 的关系：**装备的定义（`EquipmentDefinition`）通常被物品定义（`InventoryItemDefinition`）的某个 Fragment 引用**（P08.4 `InventoryFragment_EquippableItem` 里就有 `UTenkaichiEquipmentDefinition` 前置声明），这样"背包里某件物品"能对应"一件装备"。

一句话：**P09 = 装备的定义 / 实例 / 管理器 / 快捷栏 / 拾取定义 / 装备技能**，是"穿上装备"的完整闭环。

---

## 二、全景：6 个类（12 文件）

| 编号 | 类 | 文件 | 作用 |
|------|-----|------|------|
| P09.1 | `TenkaichiEquipmentDefinition` | `.h` 56 / `.cpp` 12 | 装备的"菜谱"：要生成的 Actor、要授予的技能集 |
| P09.2 | `TenkaichiEquipmentInstance` | `.h` 74 / `.cpp` 100 | 装备的"实例"：穿在身上的那件，持有生成的 Actor |
| P09.3 | `TenkaichiEquipmentManagerComponent` | `.h` 158 / `.cpp` 250 | 装备管理器：FastArray 同步已穿装备，增删查 |
| P09.4 | `TenkaichiPickupDefinition`（含 WeaponPickup） | `.h` 69 / `.cpp` 5 | 地上的拾取物配置（DataAsset） |
| P09.5 | `TenkaichiQuickBarComponent` | `.h` 107 / `.cpp` 190 | 快捷栏：3 个槽位，切换装备 |
| P09.6 | `TenkaichiGameplayAbility_FromEquipment` | `.h` 36 / `.cpp` 45 | 装备授予的技能基类：拿到"关联的装备" |

> 命名：全部有 `Lyra` 前缀 → 换 `Tenkaichi`（铁律 24）。

---

## 三、依赖分析（铁律 30/31）

```
P09.6 GameplayAbility_FromEquipment ── 依赖 P05 LyraGameplayAbility + P09.2 Instance + P08 InventoryItemInstance
P09.5 QuickBarComponent ── 依赖 P08 InventoryItemInstance + P09.3 Manager
P09.4 PickupDefinition ── 依赖 P08 InventoryItemDefinition（相对独立）
P09.3 EquipmentManagerComponent ── 依赖 P09.1 Definition + P09.2 Instance + P05 AbilitySet/ASC
P09.2 EquipmentInstance ── 依赖 P09.1 的 FLyraEquipmentActorToSpawn
P09.1 EquipmentDefinition ── 依赖 P05 AbilitySet（最底）
```

**关键依赖（已就位 ✅）：**
- `LyraAbilitySet` / `LyraAbilitySystemComponent` / `LyraGameplayAbility` → **P05 AbilitySystem 已全部教完**
- `LyraInventoryItemDefinition` / `LyraInventoryItemInstance` → **P08 已全部教完**

**P09 内部依赖**：Definition（底）→ Instance → Manager → QuickBar / FromEquipment。

> 🎉 **P09 零新挂起**：所有外部依赖（P05 GAS + P08 Inventory）都已就位，P09 内部按自下而上顺序教即可。这是继 P08 之后又一个"全链可编译"的阶段。

---

## 四、教学顺序（自下而上，铁律 20/29/35）

| 序 | 编号 | 内容 | 为什么排这里 |
|----|------|------|-------------|
| 1 | **P09.1** | `TenkaichiEquipmentDefinition` | **最底**，只依赖 P05 AbilitySet；被后面 4 个类依赖 |
| 2 | **P09.2** | `TenkaichiEquipmentInstance` | 依赖 P09.1 的 `FLyraEquipmentActorToSpawn` |
| 3 | **P09.3** | `TenkaichiEquipmentManagerComponent` | 依赖 P09.1 + P09.2 + P05；本阶段最大（FastArray 同款） |
| 4 | **P09.4** | `TenkaichiPickupDefinition` | 依赖 P08，相对独立（可提前，但放这里对齐 Lyra 文件顺序） |
| 5 | **P09.5** | `TenkaichiQuickBarComponent` | 依赖 P08 + P09.3 Manager |
| 6 | **P09.6** | `TenkaichiGameplayAbility_FromEquipment` | 依赖 P05 + P09.2 + P08；**解开 ⑳ 号挂起的时机** |

---

## 五、核心难点预读

1. **`FLyraEquipmentActorToSpawn`**：装备定义里描述"要生成什么 Actor、挂哪个插槽、什么偏移"。装备实例 `SpawnEquipmentActors` 会按这个生成武器模型等。
2. **`ULyraAbilitySet` 授予**：装备定义里 `AbilitySetsToGrant` 是"穿上这件装备时授予哪些 GAS 技能"。装备管理器 `EquipItem` 时会调用 AbilitySet 的 `GiveToAbilitySystem`（P05 已教）。
3. **FastArray 又来了**：`FLyraEquipmentList` 和 P08 的 `FLyraInventoryList` 是**同款机制**（增量同步 + 三回调），但多了"授予技能句柄 `GrantedHandles`"（记录授予了哪些技能，卸载时要回收）。
4. **快捷栏 QuickBar**：3 个槽位，`CycleActiveSlotForward/Backward` 切换，切换时触发 `EquipItemInSlot`（装备）/`UnequipItemInSlot`（卸下）。

---

## 六、命名对照（铁律 24）

全部有 `Lyra` 前缀 → 换 `Tenkaichi`：

| Lyra | Tenkaichi |
|------|-----------|
| `ULyraEquipmentDefinition` | `UTenkaichiEquipmentDefinition` |
| `ULyraEquipmentInstance` | `UTenkaichiEquipmentInstance` |
| `ULyraEquipmentManagerComponent` | `UTenkaichiEquipmentManagerComponent` |
| `ULyraPickupDefinition` / `ULyraWeaponPickupDefinition` | `UTenkaichiPickupDefinition` / `UTenkaichiWeaponPickupDefinition` |
| `ULyraQuickBarComponent` | `UTenkaichiQuickBarComponent` |
| `ULyraGameplayAbility_FromEquipment` | `UTenkaichiGameplayAbility_FromEquipment` |
| `FLyraEquipmentActorToSpawn` / `FLyraAppliedEquipmentEntry` / `FLyraEquipmentList` | `FTenkaichi...`（同理换前缀） |

---

## 七、教学节奏（铁律 20/29/35）

| 编号 | 内容 | 状态 |
|------|------|------|
| P09.1 | `TenkaichiEquipmentDefinition` | ✅ 三件套完成（`.h` 63 行 / `.cpp` 14 行已落盘） |
| P09.2 | `TenkaichiEquipmentInstance` | ✅ 三件套完成（`.h` 92 行 / `.cpp` 136 行已落盘） |
| P09.3 | `TenkaichiEquipmentManagerComponent` | ✅ 三件套完成（`.h` 170 行 / `.cpp` 300 行已落盘，2026-10-30） |
| P09.4 | `TenkaichiPickupDefinition` | ✅ 三件套完成（`.h` 73 行 / `.cpp` 7 行已落盘，2026-10-30） |
| P09.5 | `TenkaichiQuickBarComponent` | ✅ 三件套完成（`.h` 135 行 / `.cpp` 240 行已落盘，2026-10-30） |
| P09.6 | `TenkaichiGameplayAbility_FromEquipment` | ⬜ **下一步**（解开 ⑳ 号挂起） |

> ⚠️ 到 **P09.6 `GameplayAbility_FromEquipment`** 时，翻注释备忘录解开 **⑳ 号挂起**（`AbilityCost_ItemTagStack.cpp`）。

---

## 八、一句话结论

P09 Equipment 是"把物品穿在身上"的装备系统：6 个类（定义 / 实例 / 管理器 / 拾取定义 / 快捷栏 / 装备技能），依赖 P05 GAS 核心 + P08 Inventory，零新挂起。教学顺序自下而上：Definition → Instance → Manager → PickupDefinition → QuickBar → FromEquipment，到 FromEquipment 时解开 ⑳ 号挂起。