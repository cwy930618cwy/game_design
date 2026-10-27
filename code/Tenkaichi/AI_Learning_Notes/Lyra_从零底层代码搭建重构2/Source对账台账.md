# Source 对账台账（工程 vs Lyra，2026-10-26）

> 对照基准：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame`（Lyra 主模块 486 个文件）
> 被查对象：`E:\ue5\game_design\code\Tenkaichi\Source\TenkaichiGame`（工程主模块 149 个文件）
> 查法：①文件清单比对 ②**函数实现符号级比对**（提取 `类名::函数名` 集合，归一化前缀后做集合差）

---

## 一、总览：按文件夹还原进度

| 文件夹 | 已还原/总数 | 进度 | 备注 |
|--------|-----------|------|------|
| ✅ **Physics** | 3 / 3 | **100%** | P00 完成 |
| ✅ **Messages** | 9 / 9 | **100%** | P01 完成 |
| ✅ **Camera** | 12 / 12 | **100%** | P04 完成 |
| 🟡 **Input** | 12 / 14 | 86% | 缺 `InputComponent`（P03） |
| 🟡 **AbilitySystem** | 45 / 51 | 88% | ⚠️ 6 个是**路径错位**不是缺（见二） |
| 🟡 **Character** | 14 / 16 | 88% | 缺 `HealthComponent`（P06 下一步） |
| 🟡 **Teams** | 14 / 22 | 64% | 缺 8 个（P07） |
| 🟡 **System** | 12 / 27 | 44% | 缺 15 个（P14，多为收口类） |
| 🔴 **Player** | 6 / 16 | 38% | 缺 10 个（P12） |
| ⬜ Animation | 0 / 2 | 0% | P17 |
| ⬜ Audio | 0 / 4 | 0% | P17 |
| ⬜ Cosmetics | 0 / 11 | 0% | P17 |
| ⬜ Development | 0 / 6 | 0% | P18 |
| ⬜ Equipment | 0 / 12 | 0% | P09 |
| ⬜ Feedback | 0 / 19 | 0% | P17 |
| ⬜ GameFeatures | 0 / 16 | 0% | P15 |
| ⬜ GameModes | 0 / 20 | 0% | P13 |
| ⬜ Hotfix | 0 / 6 | 0% | P18 |
| ⬜ Interaction | 0 / 17 | 0% | P10 |
| ⬜ Inventory | 0 / 16 | 0% | P08 |
| ⬜ Performance | 0 / 6 | 0% | P18 |
| ⬜ Replays | 0 / 4 | 0% | P18 |
| ⬜ Settings | 0 / 35 | 0% | P18 |
| ⬜ Tests | 0 / 4 | 0% | P18 |
| ⬜ UI | 0 / 79 | 0% | P16 |
| ⬜ Weapons | 0 / 16 | 0% | P11 |
| ✅ **(根目录)** | 6 / 7 | 86% | 只缺 `README.md`（无关） |

**合计：149 / 486 ≈ 31%**（Lyra 主模块口径）

---

## 二、⚠️ 真缺失 vs 路径错位（重要）

### 2.1 路径错位 ✅ **已修正（2026-10-26）**

> 6 个文件已从 `AbilitySystem\` 根目录移动到 `AbilitySystem\Attributes\`：`TenkaichiAttributeSet` / `TenkaichiCombatSet` / `TenkaichiHealthSet`（各 h/cpp）。
> 全库扫描确认**无任何文件引用旧路径**，且新路径 `AbilitySystem/Attributes/...` 与 Lyra 完全一致。

（以下为修正前的原始记录）

| 现在的位置（错误） | Lyra 正确位置 |
|------------------|--------------|
| `AbilitySystem\TenkaichiAttributeSet.h/.cpp` | `AbilitySystem\Attributes\` |
| `AbilitySystem\TenkaichiCombatSet.h/.cpp` | `AbilitySystem\Attributes\` |
| `AbilitySystem\TenkaichiHealthSet.h/.cpp` | `AbilitySystem\Attributes\` |

> 铁律 09：文件路径一比一映射 Lyra。**内容已一比一，只需移动**（3 对 6 个文件）。

### 2.2 真缺失（按阶段）

| 阶段 | 缺失文件 |
|------|---------|
| **P03 Input** | `TenkaichiInputComponent.h/.cpp` |
| **P06 Character** | `TenkaichiHealthComponent.h/.cpp` ← **下一步就教** |
| **P07 Teams** | `AsyncAction_ObserveTeam`、`AsyncAction_ObserveTeamColors`、`TenkaichiTeamCheats`、`TenkaichiTeamCreationComponent`（各 h/cpp） |
| **P12 Player** | `TenkaichiCheatManager`、`TenkaichiDebugCameraController`、`TenkaichiPlayerBotController`、`TenkaichiPlayerSpawningManagerComponent`、`TenkaichiPlayerStart`（各 h/cpp） |
| **P14 System** | `TenkaichiAssetManager`、`AssetManagerStartupJob`、`TenkaichiGameInstance`、`TenkaichiGameSession`、`TenkaichiReplicationGraph`、`ReplicationGraphSettings`、`ReplicationGraphTypes`、`TenkaichiSignificanceManager`（各 h/cpp） |

> ✅ **符合预期**：这些正是"还没教到的阶段"，不是遗漏。

---

## 三、✅ 内容一比一核对结果（重点）

**核对方法**：对每个已还原的 `.cpp`，提取所有 `类名::函数名` 实现，把 `Lyra`/`Tenkaichi` 统一归一化后做集合差。

**核对覆盖：66 个 `.cpp` 文件**

### 结论：**全部对齐，零缺失、零多余** ✅

抽样明细：

| 文件 | 函数实现数 | 结果 |
|------|-----------|------|
| `TenkaichiAbilitySystemComponent.cpp` | 37 | ✅ 全部对齐 |
| `TenkaichiGameplayAbility.cpp` | 31 | ✅ 全部对齐 |
| `TenkaichiHeroComponent.cpp` | 28 | ✅ 全部对齐 |
| `TenkaichiPawnExtensionComponent.cpp` | 22 | ✅ 全部对齐 |
| `TenkaichiTeamSubsystem.cpp` | 27 | ✅ 全部对齐 |
| `TenkaichiSystemStatics.cpp` | 7 | ✅ 全部对齐 |
| ...其余 60 个 | — | ✅ 全部对齐 |

> 唯一告警项 `TenkaichiCharacter.cpp` 的 `FSharedRepMovement` 三处 —— 经人工核对是**脚本 `-replace` 误伤**（`FSharedRepMovement` 不含 `Lyra` 前缀不该被替换），**实际代码正确**。

### 关于"行数差异 ±3~4"

机器比对发现若干文件代码行数差 3~4 行，人工抽查确认**全部是注释挂起造成的 `#if 0` / `#endif` 包裹行**，符合铁律 33/34，**不是缺代码**：

| 文件 | 差值 | 原因 |
|------|------|------|
| `TenkaichiGameplayAbility.cpp` | +14 | ㉒ 挂起（6 函数 + 2 include） |
| `TenkaichiGameplayAbility_Death.cpp` | +4 | ㉖ 挂起 |
| `TenkaichiDamageExecution.cpp` | +4 | ㉕ 挂起 |
| `TenkaichiPawnExtensionComponent.cpp` | +4 | ⑥ 挂起 |
| `TenkaichiAbilitySystemGlobals.cpp` | +4 | ㉔ 挂起 |
| `TenkaichiGameData.cpp` / `SystemStatics.cpp` / `DevelopmentStatics.cpp` | +4 | ⑮⑯⑰ 挂起 |

---

## 四、✅ md 搬运结果（重构版1 → 重构版2）

| 来源 | 数量 | 去向 |
|------|------|------|
| `阶段四：角色体系\` | **56** | 按内容拆分映射（见下） |
| `阶段三：系统基础\` | 8 | `P14_System`（5）+ `S1_底层插件`（3） |
| `教学疑问解答\` | 13 | `归档_重构版1\教学疑问解答\`（保留原子目录） |
| `重构遗留代码\` | 1 | `归档_重构版1\重构遗留代码\` |
| `00_学习路径总纲_自下而上.md` | 1 | `归档_重构版1\`（重命名为「(重构版1归档)」） |

### 阶段四 56 个 md 的拆分映射

| 原编号 | 去向 | 数量 |
|--------|------|------|
| `4.1/4.2/4.4/4.5`（Team 系列） | **P07_Teams** | 21 |
| `4.3/4.6/4.7` + `04_角色体系_总md` | **P06_Character** | 22 |
| `4.8.0`（CameraAssistInterface） | **P04_Camera** | 2 |
| `4.8.1/4.8.2/4.8.3` + `4.8_玩家类_总md` | **P12_Player** | 10 |
| `4.9`（Experience_GameModes） | **P13_GameModes** | 1 |

> ⚠️ **注意**：`4.7.1_CharacterMovementComponent` / `4.7.2_Pawn` / `4.7.3_Character` / `4.7.4_CharacterWithAbilities` 这批**沿用重构版1 的编号**（`4.7.x`），
> 和重构版2 的 `P06.x` 命名不一致。考虑到**编号只是索引、内容有效**，本次**保持原文件名不重命名**，避免破坏已有交叉引用（铁律 37 精神：先看既有格式，不擅自换）。

---

## 五、结论

1. **Source 内容质量合格** —— 66 个 `.cpp` 函数实现**零缺失零多余**，一比一还原到位。
2. **3 对属性集文件需移动**到 `AbilitySystem\Attributes\`（路径错位，内容没问题）。
3. **真缺失的文件全部属于"还没教到的阶段"**，符合教学进度，不是遗漏。
4. **重构版1 的 md 已全部搬入重构版2**（78 个），旧目录可留作参照。

---

## 六、待办

- [x] 移动 `TenkaichiAttributeSet` / `TenkaichiCombatSet` / `TenkaichiHealthSet` 到 `AbilitySystem\Attributes\`（6 个文件）✅ 2026-10-26
- [x] 教 P06.8 `TenkaichiHealthComponent`（`.h` 148 行 + `.cpp` 329 行已落盘核对通过）✅ 2026-10-26
- [x] 解开挂起：**㉖ ㉒ ㉔ ⑨ ⑥ ㉗ ⑧** ✅ 2026-10-26（详见注释备忘录）
  - ⑨ `TenkaichiCharacterWithAbilities.cpp` 整体放开（47→43 行）
  - ⑥ `TenkaichiPawnExtensionComponent.cpp` 整体放开（332→329 行）；**顺带修复 2 处误入的 markdown 围栏 ```**（原 83、195 行）+ 统一 `PawnData` include 路径
  - ⑧ `TenkaichiCharacter.cpp` 整体放开（754→753 行），**仅留 ㉙ Significance 两段**
- [ ] **新建 `TenkaichiInputComponent.h/.cpp`**（🔴 P03 唯一缺口，见 `P00-P05_阶段对账报告.md`）
- [ ] 新建 `TenkaichiSignificanceManager.h/.cpp`（P14 `System\`，解锁 ㉙）
- [x] 解开 **㉑** `AbilityCost_PlayerTagStack.cpp` ✅ 2026-10-26（P05.3-B 早已就位，对账时发现）
- [ ] 解开 **⑬** `VerbMessageHelpers.cpp` GAS 段（P05 已就位）
- [ ] 整理 md 命名：删 `P04_Camera\4.8.0_*`（被 P04.1 取代）、修 `P00.1_LyraCollisionChannels_h.md` 前缀
- [ ] 补 `TenkaichiInputComponent.h/.cpp`（P03 唯一未还原的一对）
- [ ] 继续 P07 Teams（补 8 个）
