# P11 Weapons —— 阶段总 md

> **定位**：本文件是 P11 Weapons（武器系统）的**阶段总 md**（为什么 + 全景 + 依赖分析 + 教学顺序）。
> 每个类另有自己的总 md + `.h` + `.cpp` 教学 md（铁律 35）。
>
> **建立时间**：2026-11-06
> **教学状态**：✅ 完成 —— P11.1 `ULyraWeaponInstance` 总md+.h+.cpp 已教（2026-11-03）；P11.2 `ULyraRangedWeaponInstance` 总md+.h+.cpp 已教（2026-11-04）；P11.3 `ULyraWeaponStateComponent` 总md+.h+.cpp 已教（2026-11-04）；P11.4 `ULyraWeaponSpawner` 总md+.h+.cpp 已教（2026-11-04）；P11.5 `ULyraGameplayAbility_RangedWeapon` 总md+.h+.cpp 已教（2026-11-04）；P11.6 `UInventoryFragment_ReticleConfig` 总md+.h 已教（2026-11-04）✅（纯 `.h`，无 `.cpp`）；P11.7 `ULyraWeaponDebugSettings` 总md+.h+.cpp 已教（2026-11-04）✅；P11.8 `ULyraDamageLogDebuggerComponent` 总md+.h+.cpp 已教（2026-11-04）✅（零挂起）

---

## 一、本阶段要解决什么

P09 讲了"装备"（把物品穿在身上），P10 讲了"交互"（靠近捡东西）。但还有一个核心没做：**角色拿起武器，怎么开火？怎么命中敌人？**

P11 Weapons 就是**"武器系统"**，它回答：

1. **"武器是什么？"** → `ULyraWeaponInstance`（武器实例，继承装备实例）。它记录"上次开火时间、上次装备时间"，还管理**武器震动属性**（手柄震动）。
2. **"远程武器（枪）怎么算命中？"** → `ULyraRangedWeaponInstance`（远程武器实例，继承武器实例）。它实现**散布（Spread）/ 过热（Heat）/ 伤害衰减**机制，是 GAS 的"伤害来源"。
3. **"开火这个技能怎么实现？"** → `ULyraGameplayAbility_RangedWeapon`（远程武器技能，继承 P09 的 `GameplayAbility_FromEquipment`）。它做**射线追踪（Trace）、目标选取（Targeting）、开火**。
4. **"命中提示（准星）怎么显示？"** → `ULyraWeaponStateComponent`（武器状态组件）+ `UInventoryFragment_ReticleConfig`（准星配置片段）。
5. **"地图上怎么摆一把武器让玩家捡？"** → `ULyraWeaponSpawner`（武器生成器，继承 `UTenkaichiWeaponPickupDefinition`）。
6. **调试辅助** → `ULyraWeaponDebugSettings`（子弹轨迹调试）+ `ULyraDamageLogDebuggerComponent`（伤害日志调试）。

一句话：**P11 = 一整套"武器"机制**——武器实例（数据）+ 远程武器（散布/伤害）+ 开火技能（GAS）+ 命中提示（UI）+ 武器生成器（捡拾）。

---

## 二、全景：8 个教学内容（16 文件）

| 编号 | 类 | 文件 | 作用 |
|------|------|------|------|
| P11.1 | `ULyraWeaponInstance` | `.h` + `.cpp` | 武器实例基类（继承装备实例，记录开火/装备时间 + 手柄震动属性） |
| P11.2 | `ULyraRangedWeaponInstance` | `.h` + `.cpp` | 远程武器实例（散布/过热/伤害衰减，实现 `ILyraAbilitySourceInterface`） |
| P11.3 | `ULyraWeaponStateComponent` | `.h` + `.cpp` | 武器状态组件（命中标记 + 屏幕空间命中位置 + 最近伤害时间） |
| P11.4 | `ULyraWeaponSpawner` | `.h` + `.cpp` | 武器生成器（地图上摆武器，玩家走近拾取，带冷却重生） |
| P11.5 | `ULyraGameplayAbility_RangedWeapon` | `.h` + `.cpp` | 远程武器技能（射线追踪 + 目标选取 + 开火） |
| P11.6 | `UInventoryFragment_ReticleConfig` | `.h`（无 `.cpp`） | 准星配置片段（指定该武器用什么准星 UI） |
| P11.7 | `ULyraWeaponDebugSettings` | `.h` + `.cpp` | 武器调试设置（子弹轨迹/命中点调试绘制） |
| P11.8 | `ULyraDamageLogDebuggerComponent` | `.h` + `.cpp` | 伤害日志调试组件（监听伤害消息并记录） |

> 命名注意（铁律 24）：**大部分类有 `Lyra` 前缀 → 换成 `Tenkaichi`**（`ULyraWeaponInstance`→`UTenkaichiWeaponInstance` 等）。**唯一无前缀的** `UInventoryFragment_ReticleConfig` → 保持原样。

---

## 三、依赖分析（铁律 30/31）

```
P11.5 GameplayAbility_RangedWeapon ── 依赖 P09 GameplayAbility_FromEquipment + P11.2 RangedWeaponInstance + P05 GAS
P11.2 RangedWeaponInstance ── 依赖 P11.1 WeaponInstance + P05 ILyraAbilitySourceInterface
P11.1 WeaponInstance ── 依赖 P09 EquipmentInstance + P17 FLyraAnimLayerSelectionSet(Cosmetics) + 引擎 InputDeviceProperty
P11.3 WeaponStateComponent ── 依赖 P05 FGameplayAbilityTargetDataHandle + FGameplayTag
P11.4 WeaponSpawner ── 依赖 P09.4 WeaponPickupDefinition + P08 InventoryItemDefinition
P11.6 InventoryFragment_ReticleConfig ── 依赖 P08 InventoryItemFragment + P16 ULyraReticleWidgetBase(UI)
P11.7 WeaponDebugSettings ── 引擎 UDeveloperSettingsBackedByCVars
P11.8 DamageLogDebuggerComponent ── 依赖 P01 FLyraVerbMessage + GameplayMessageSubsystem
```

**已就位 ✅：** P05 GAS、P08 Inventory、P09 Equipment（含 `WeaponPickupDefinition`）、P01 Messages（`FLyraVerbMessage`）、P00 CollisionChannels。

**关键断点（⚠️ 注释挂起）：**

| 断点 | 依赖 | 处理 |
|------|------|------|
| `WeaponInstance.cpp` | `Cosmetics/LyraCosmeticAnimationTypes.h` 的 `FLyraAnimLayerSelectionSet`（P17） | `.cpp` 挂起 P17（铁律 33/34） |
| `WeaponInstance.cpp` | `ULyraReticleWidgetBase`（P16 UI） | `.cpp` 挂起 P16 |
| `InventoryFragment_ReticleConfig` | `ULyraReticleWidgetBase`（P16 UI） | 挂起 P16 |
| `WeaponDebugSettings` | `UDeveloperSettingsBackedByCVars`（引擎，P18） | 引擎底层，不挂起（P18 配置化） |

> 按死结处理铁律：**代码照写 + 注释挂起 + 登记备忘录 + 继续推进**，不停下纠结。

---

## 四、教学顺序（自下而上，铁律 20/29/35）

| 序 | 编号 | 内容 | 为什么排这里 |
|----|------|------|-------------|
| 1 | **P11.1** | `ULyraWeaponInstance` | 武器基类，被 P11.2 继承 |
| 2 | **P11.2** | `ULyraRangedWeaponInstance` | 继承 P11.1，被 P11.5 依赖 |
| 3 | **P11.3** | `ULyraWeaponStateComponent` | 相对独立（命中标记） |
| 4 | **P11.4** | `ULyraWeaponSpawner` | 依赖 P08/P09（已就位） |
| 5 | **P11.5** | `ULyraGameplayAbility_RangedWeapon` | 收口，依赖 P11.2 + P09 + P05 |
| 6 | **P11.6** | `UInventoryFragment_ReticleConfig` | 小片段（依赖 P08 + P16） |
| 7 | **P11.7** | `ULyraWeaponDebugSettings` | 调试设置（独立） |
| 8 | **P11.8** | `ULyraDamageLogDebuggerComponent` | 调试组件（依赖 P01） |

---

## 五、核心难点预读

1. **武器 = 装备的延伸**：`ULyraWeaponInstance` 继承 `ULyraEquipmentInstance`（P09），武器本质是一种"可装备的物品"。
2. **散布（Spread）/ 过热（Heat）系统**：`ULyraRangedWeaponInstance` 用 `FRuntimeFloatCurve`（曲线）驱动散布角度和热量，是远程武器手感的核心。
3. **GAS 伤害来源接口**：`ILyraAbilitySourceInterface`（P05）让武器实例成为"伤害衰减"的来源（距离衰减 + 物理材质衰减）。
4. **开火技能**：`ULyraGameplayAbility_RangedWeapon` 重写 `ActivateAbility`/`EndAbility`，做射线追踪 + 目标选取 + 回调目标数据。
5. **命中提示**：`ULyraWeaponStateComponent` 用 `FLyraScreenSpaceHitLocation` 记录屏幕空间的命中位置，供 UI 显示准星命中标记。

---

## 六、命名对照（铁律 24）

| Lyra | Tenkaichi | 说明 |
|------|-----------|------|
| `ULyraWeaponInstance` | `UTenkaichiWeaponInstance` | ✅ 有前缀，换 |
| `ULyraRangedWeaponInstance` | `UTenkaichiRangedWeaponInstance` | ✅ 有前缀，换 |
| `ULyraWeaponStateComponent` | `UTenkaichiWeaponStateComponent` | ✅ 有前缀，换 |
| `ULyraWeaponSpawner` | `UTenkaichiWeaponSpawner` | ✅ 有前缀，换 |
| `ULyraGameplayAbility_RangedWeapon` | `UTenkaichiGameplayAbility_RangedWeapon` | ✅ 有前缀，换 |
| `UInventoryFragment_ReticleConfig` | `UInventoryFragment_ReticleConfig` | **无前缀，保持原样** |
| `ULyraWeaponDebugSettings` | `UTenkaichiWeaponDebugSettings` | ✅ 有前缀，换 |
| `ULyraDamageLogDebuggerComponent` | `UTenkaichiDamageLogDebuggerComponent` | ✅ 有前缀，换 |
| `FLyraScreenSpaceHitLocation` | `FTenkaichiScreenSpaceHitLocation` | ✅ 有前缀，换 |
| `FLyraServerSideHitMarkerBatch` | `FTenkaichiServerSideHitMarkerBatch` | ✅ 有前缀，换 |
| `ELyraAbilityTargetingSource` | `ETenkaichiAbilityTargetingSource` | ✅ 有前缀，换 |
| `FRangedWeaponFiringInput` | `FRangedWeaponFiringInput` | **无前缀，保持原样** |

---

## 六、教学节奏（铁律 20/29/35）

1. 🟡 本阶段总 md（现在这一步）
2. ✅ P11.1 `UTenkaichiWeaponInstance`（总md+.h+.cpp）→ ✅ P11.2 `UTenkaichiRangedWeaponInstance`（总md+.h+.cpp）→ ✅ P11.3 `UTenkaichiWeaponStateComponent`（总md+.h+.cpp）→ ✅ P11.4 `UTenkaichiWeaponSpawner`（总md+.h+.cpp）→ ✅ P11.5 `UTenkaichiGameplayAbility_RangedWeapon`（总md+.h+.cpp）→ ✅ P11.6 `UInventoryFragment_ReticleConfig`（总md+.h，2026-11-04）→ ✅ P11.7 `UTenkaichiWeaponDebugSettings`（总md+.h+.cpp，2026-11-04）→ ✅ P11.8 `UTenkaichiDamageLogDebuggerComponent`（总md+.h+.cpp，2026-11-04）✅ P11 阶段全部完成

> ⚠️ 注意：
> - P11.6 `UInventoryFragment_ReticleConfig` 是**纯 `.h`（无 `.cpp`）**，节奏简化为「总 md → 教 `.h`」。
> - P11.1 `.cpp` 依赖 P17 Cosmetics + P16 UI → **注释挂起，登记备忘录**。
> - P11.6 依赖 P16 `ULyraReticleWidgetBase` → 该行**注释挂起，登记备忘录**。

---

## 八、一句话结论

P11 Weapons 是一整套"武器"机制：武器实例（数据）+ 远程武器（散布/过热/伤害衰减）+ 开火技能（GAS）+ 命中提示（UI）+ 武器生成器（捡拾）+ 调试工具。8 个教学内容（16 文件），依赖 P05 GAS + P08 Inventory + P09 Equipment + P01 Messages（已就位）。`WeaponInstance.cpp` 依赖 P17 Cosmetics + P16 UI（挂起），`InventoryFragment_ReticleConfig` 依赖 P16（挂起）。除 `UInventoryFragment_ReticleConfig` 无前缀保持原样外，其余类都换 `Tenkaichi` 前缀。