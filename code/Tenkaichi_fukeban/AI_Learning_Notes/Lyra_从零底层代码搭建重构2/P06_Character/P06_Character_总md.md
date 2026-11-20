# P06 Character —— 阶段总 md

> **定位**：P06 Character 阶段的**目录/路线图**（这一节有哪些类、各依赖什么、先教谁）。
> 具体某一个类怎么教，由**该类自己的总 md** 负责（铁律 35：上级总 md 不能替代子类总 md）。
>
> **最后更新**：2026-10-27（md 命名规范化 + 进度同步）

---

## 一、这一节要解决什么

P05 把 GAS 核心（ASC / GA / 属性集 / 效果上下文 / 技能）全部搭好了。
但 **GAS 是"能力系统"，它自己不会主动挂到角色身上** —— P06 就是**把 GAS 接到角色上**，并补上最后一个关键组件：**血量**。

一句话：**P06 = 让角色真正"能用技能 + 有血量 + 会死"**。

---

## 二、全景：Lyra `Character/` 文件夹 16 个文件（8 个类）

对照 `E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Character\`：

| 编号 | Lyra 文件 | 工程文件 | 状态 |
|------|----------|---------|------|
| **P06.1** | `LyraPawnData` | `TenkaichiPawnData.h/.cpp` | ✅ 已完成（零挂起） |
| **P06.2** | `LyraPawnExtensionComponent` | `TenkaichiPawnExtensionComponent.h/.cpp` | ✅ 已完成（⑥ 挂起 **已解开**） |
| **P06.3** | `LyraHeroComponent` | `TenkaichiHeroComponent.h/.cpp` | ✅ 已教（挂起 ⑦ 待 P15 GameFeatures） |
| **P06.4** | `LyraPawn` | `TenkaichiPawn.h/.cpp` | ✅ 已完成（零挂起） |
| **P06.5** | `LyraCharacter` | `TenkaichiCharacter.h/.cpp` | ✅ 已完成（⑧ 已解，仅剩 ㉙ Significance 待 P14） |
| **P06.6** | `LyraCharacterWithAbilities` | `TenkaichiCharacterWithAbilities.h/.cpp` | ✅ 已完成（⑨ 挂起 **已解开**） |
| **P06.7** | `LyraCharacterMovementComponent` | `TenkaichiCharacterMovementComponent.h/.cpp` | ✅ 已完成（零挂起） |
| **P06.8** | `LyraHealthComponent` | `TenkaichiHealthComponent.h/.cpp` | ✅ 已完成（挂起 ㉘ `DamageSelfDestruct` 待 P14） |

> ✅ **工程 `Character\` 16 / 16 全部落盘**，与 Lyra 完全一致（2026-10-26 补齐 HealthComponent 后达成）。

### 教学 md 清单（2026-10-27 重命名后）

| 编号 | 文件 |
|------|------|
| 节总览 | `P06.4-7_CharacterPawn_总md.md`（P06.4~P06.7 四个角色/Pawn 类的路线图） |
| P06.1 | `P06.1_TenkaichiPawnData_总md/_h/_cpp.md` |
| P06.2 | `P06.2_TenkaichiPawnExtensionComponent_总md/_h/_cpp.md` |
| P06.3 | `P06.3_TenkaichiHeroComponent_总md/_h/_cpp.md` |
| P06.4 | `P06.4_TenkaichiPawn_总md/_h/_cpp.md` |
| P06.5 | `P06.5_TenkaichiCharacter_总md/_h/_cpp.md` |
| P06.6 | `P06.6_TenkaichiCharacterWithAbilities_总md/_h/_cpp.md` |
| P06.7 | `P06.7_TenkaichiCharacterMovementComponent_总md/_h/_cpp.md` |
| P06.8 | `P06.8_TenkaichiHealthComponent_总md/_h/_cpp.md` |
| P06 总结 | **`P06_一张图看懂_全局总结.md`**（⭐ 反晕眩版：类比 + 全局架构图 + 完整链路图 + 依赖图 + 混淆点对照） |
| 归档 | `04_角色体系_总md(重构版1归档).md`（重构版1 遗留，仅参照，**不再作为推进依据**） |

> 📌 **命名规范（铁律 37）**：统一 `P06.x_Tenkaichi类名_总md/_h/_cpp`，下划线分隔，类从 `.1` 起编。
> 旧编号（`4.3.x` / `4.6.x` / `4.7.x` / `4.8.x`）全部作废。

---

## 三、依赖分析（P06 全阶段，铁律 30/31）

统一方向依赖树（被依赖在下）：

```
ATenkaichiCharacterWithAbilities（P06.6，最重）
   │ 继承
ATenkaichiCharacter（P06.5）
   │ 继承
AModularCharacter（引擎 ModularGameplay 插件 ①，只讲不拆）
   │ 组合
   ├── UTenkaichiPawnExtensionComponent（P06.2 ✅）
   ├── UTenkaichiHealthComponent（P06.8 ✅）
   └── UTenkaichiCameraComponent（P04.2-3 ✅）
   │
ATenkaichiPawn（P06.4，独立可编译）
   │ 继承
AModularPawn（引擎 ModularGameplay 插件 ①）
   │ 实现
   └── ITenkaichiTeamAgentInterface（P07 ✅）

UTenkaichiCharacterMovementComponent（P06.7，独立可编译）
   │ 继承 UCharacterMovementComponent（引擎）

UTenkaichiHeroComponent（P06.3）
   └── 依赖 P15 GameFeatures ⚠️（挂起 ⑦）

UTenkaichiPawnData（P06.1，纯数据资产，独立）
```

### 依赖就位情况

| 依赖 | 状态 |
|------|------|
| P05.2 `TenkaichiAbilitySystemComponent` | ✅ 已落盘 |
| P05.4-1 `TenkaichiHealthSet` / `CombatSet` / `AttributeSet` | ✅ 已落盘（**已移到 `AbilitySystem\Attributes\` 正确子目录**） |
| P04.2-3 `TenkaichiCameraComponent` | ✅ 已落盘 |
| P07 `TenkaichiTeamAgentInterface` | ✅ 已落盘 |
| 第 0 步 `TenkaichiLogChannels` / `TenkaichiGameplayTags` | ✅ 已落盘 |
| P01 `TenkaichiVerbMessage` + `Helpers` | ✅ 已落盘 |
| **P14 `TenkaichiAssetManager`** | ❌ 未落盘（挂起 ㉘ / ㉙） |
| **P15 GameFeatures** | ❌ 未落盘（挂起 ⑦） |

---

## 四、🔓 阶段开头必做：翻注释备忘录（铁律 34）

P06 是**多个挂起项的解锁时机**。当前状态（2026-10-27 二次全量复查后）：

| 条目 | 内容 | 状态 |
|------|------|------|
| **㉖** | `TenkaichiGameplayAbility_Death.cpp`：HealthComponent include + `StartDeath`/`FinishDeath` 函数体 | ✅ **已解开** |
| **㉗** | `TenkaichiGameplayAbility_Reset.cpp`：`TenkaichiChar->Reset();` | ✅ **已解开** |
| **⑥** | `TenkaichiPawnExtensionComponent.cpp` 整个 `.cpp` | ✅ **已解开** |
| **⑧** | `TenkaichiCharacter.cpp` 整个 `.cpp` | ✅ **已解开**（新增 ㉙ 仅 Significance 段待 P14） |
| **⑨** | `TenkaichiCharacterWithAbilities.cpp` 整个 `.cpp` | ✅ **已解开** |
| **㉒** | `TenkaichiGameplayAbility.cpp` 6 函数 + 2 include | ✅ **已解开** |
| **㉔** | `TenkaichiAbilitySystemGlobals.cpp` `AllocGameplayEffectContext` | ✅ **已解开** |
| **⑦** | `TenkaichiHeroComponent.h` + `.cpp` | ⏸ 仍需 **P15 GameFeatures**，继续挂起 |
| **㉘** | `TenkaichiHealthComponent.cpp` `DamageSelfDestruct` + AssetManager include | ⏸ 待 **P14 AssetManager** |
| **㉙** | `TenkaichiCharacter.cpp` Significance 两段 | ⏸ 待 **P14 SignificanceManager** |

> ⚠️ **按死结处理铁律**：挂起不是障碍。已到时机的已解开，没到的（⑦ ㉘ ㉙）继续挂起，**不停下、不纠结**。

---

## 五、教学顺序（铁律 20/29/35）

**每个类都已走完「自己的总 md → `.h` → `.cpp`」三步**。

| 序 | 内容 | 状态 |
|----|------|------|
| 1 | P06.1 `TenkaichiPawnData` | ✅ 三件套已建，工程已落盘，零挂起 |
| 2 | P06.2 `TenkaichiPawnExtensionComponent` | ✅ 三件套已建，⑥ 已解开 |
| 3 | P06.3 `TenkaichiHeroComponent` | ✅ 三件套已建，⑦ 待 P15 |
| 4 | P06.4 `TenkaichiPawn` | ✅ 三件套已建，零挂起 |
| 5 | P06.5 `TenkaichiCharacter` | ✅ 三件套已建，⑧ 已解，仅剩 ㉙ |
| 6 | P06.6 `TenkaichiCharacterWithAbilities` | ✅ 三件套已建，⑨ 已解开 |
| 7 | P06.7 `TenkaichiCharacterMovementComponent` | ✅ 三件套已建，零挂起 |
| 8 | P06.8 `TenkaichiHealthComponent` | ✅ 三件套已建，㉘ 待 P14 |

**→ P06 教学全部完成，可进入 P07 Teams。**

---

## 六、几个要记住的点

1. **P06 = GAS 接入角色 + 血量组件**。GAS 本身（P05）不会自己挂上来。
2. **HealthComponent 是桥梁**：把 `UTenkaichiHealthSet`（GAS 侧数值）翻译成"游戏层的血量/死亡事件"。
3. **`ETenkaichiDeathState` 三态**：`NotDead` → `DeathStarted` → `DeathFinished`，和 Death 技能的 `StartDeath`/`FinishDeath` 一一对应。死亡闭环：`HandleOutOfHealth` → GameplayEvent.Death → Death 技能 → `StartDeath`/`FinishDeath` → 三态流转。
4. **`UGameFrameworkComponent` 基类**（引擎 ①，只讲不拆）—— 配合 `ModularGameplay` 总线的初始化时机。
5. **Pawn vs Character 分工**：Pawn 只管队伍归属；Character 组合 PawnExt/Health/Camera 三大组件并暴露 GAS 接口。
6. **✅ 已修待办**：`TenkaichiAttributeSet` / `TenkaichiCombatSet` / `TenkaichiHealthSet` 已在 `AbilitySystem\Attributes\` 正确子目录（2026-10-26 迁移完成）。

---

## 七、下一步

P06 教学全部收官。按总纲下一步是 **P07 Teams**（依赖 GAS + Character + GameState，均已在 P05/P06 就位）。

> 📌 提示：P07 开头**先翻注释备忘录**，Teams 链式挂起（④ TeamSubsystem → ② TeamInfoBase → ③ TeamPublicInfo → ⑤ TeamStatics）已可解开。
