# 99 — `LyraPawnData` 系列总结：`.h` 和 `.cpp` 到底各干了什么？

> **定位**：本系列（01~03）的收束篇。用一页回答：
> 1. **`ULyraPawnData` 到底是什么？**（一句话定位）
> 2. **它的 `.h` 干了什么、`.cpp` 干了什么？**（两份文件分工全貌）
>
> **配套**：01（概念）、02（五大字段）、03（.cpp 构造）。这篇把三篇合并收口，末尾画大图。

---

## 〇、一句话定位（整个系列就为这句）

> **`ULyraPawnData` = 一份"Pawn 的配置单 / 配方"**（`UPrimaryDataAsset` 纯数据资产）。它不造 Pawn、不跑逻辑，只**用数据回答 5 个问题**：这个角色**用哪个身体（PawnClass）、会什么（AbilitySets）、遵守什么规则（TagRelationshipMapping）、怎么操作（InputConfig）、用什么镜头（DefaultCameraMode）**。

```
   它的全部工作浓缩成一句话：用数据描述"一个 Pawn 该怎么被造出来"。
   真正的"读取并应用它"的人，是别的系统（GameMode / PawnExtension / HeroComponent）。
```

---

## 一、`.h` 干了什么？（57 行 = "一份配方的字段清单"）

`.h` 没有任何逻辑，只**声明"这份配置单长什么样"**。

### A. 类声明（L24~27）—— 它是什么

```cpp
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "Lyra Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class ULyraPawnData : public UPrimaryDataAsset
```

| UCLASS 标签 | 含义 |
|---|---|
| `UPrimaryDataAsset` | 它是**数据资产**（UObject→UDataAsset→UPrimaryDataAsset），能存成 `.uasset` 让策划配 |
| `Const` | 运行时**只读**，不可被游戏逻辑改动 |
| `BlueprintType` | 蓝图里能用 |
| `MinimalAPI` | 只导出类型信息（成员逐个放行） |

### B. 五大配置字段（L35~53）—— 它装了什么

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Pawn")
TSubclassOf<APawn> PawnClass;                          // ① 造哪个身体（类选择器）

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Abilities")
TArray<TObjectPtr<ULyraAbilitySet>> AbilitySets;       // ② 给哪些能力包（数组，可多组）

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Abilities")
TObjectPtr<ULyraAbilityTagRelationshipMapping> TagRelationshipMapping;  // ③ 技能互斥/允许规则表

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Input")
TObjectPtr<ULyraInputConfig> InputConfig;              // ④ 按键怎么绑

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Camera")
TSubclassOf<ULyraCameraMode> DefaultCameraMode;        // ⑤ 默认什么镜头
```

**五个字段的分工（02 篇）**：
```
① PawnClass         → 身体：造哪个 Pawn 类        （决定"是什么"）
② AbilitySets       → 技能：给哪些能力（可多组）   （决定"会什么"）
③ TagRelationshipMapping → 规则：技能间互斥/允许   （决定"技能怎么互相约束"）
④ InputConfig       → 操作：按键绑什么动作        （决定"怎么操作"）
⑤ DefaultCameraMode → 视角：默认哪种镜头          （决定"怎么看"）
```

**共同的 UPROPERTY 标注**：`EditDefaultsOnly`（只能编辑器里配默认值）+ `BlueprintReadOnly`（蓝图只读）→ **"只配一次、运行时只读"**，符合 DataAsset 不可变设计（02 篇）。

### C. 头部设施（L1~17）
- include `Engine/DataAsset.h` + generated.h + 导出宏 `UE_API` + 一排前向声明（`APawn`/`ULyraAbilitySet`/`ULyraInputConfig`/`UObject` 等，全都只以"指针/类选择器"出现 → 前向声明够用）。

> **一句话 `.h`**：声明"我是继承 UPrimaryDataAsset 的只读数据类，装着 5 个'策划填的'配置字段"。**只有字段，没有任何行为。**

---

## 二、`.cpp` 干了什么？（14 行 = "给个干净的起点"）

`.cpp` 全文就一个构造函数（03 篇逐行讲过）：

```cpp
#include "LyraPawnData.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraPawnData)      // UHT 生成代码

ULyraPawnData::ULyraPawnData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)        // ① 先让父类(UPrimaryDataAsset)初始化
{
	PawnClass = nullptr;              // ② 三个"类选择器/指针"显式清零
	InputConfig = nullptr;
	DefaultCameraMode = nullptr;
}
```

### 它只做两件事

| 行 | 干嘛 | 为什么 |
|---|---|---|
| `: Super(ObjectInitializer)` | 调父类构造 | UE UObject 构造铁律，必须让父类先初始化（03 篇） |
| 三个字段 = nullptr | 显式清零 | "明确优于隐式"，给干净起点；其余字段（TArray 默认空、TagMapping 系统自动清）不用手动 |

### 关键认知：策划填的值不在这

```
构造函数（清空 = 毛坯房）   →   加载 .uasset 时序列化系统把策划填的值写进来（装修）
```
构造函数只保证"有一个干净的起点"；策划在编辑器填的 5 个字段值，在**加载资产文件时**由序列化覆盖（03 篇）。

> **一句话 `.cpp`**：整个文件只给这份数据容器"一个干净的起点"——真正的内容靠策划配、靠序列化填。

---

## 三、`.h` vs `.cpp` 分工总对比

| 维度 | `.h`（57 行） | `.cpp`（14 行） |
|---|---|---|
| 角色 | **字段清单** | **构造初始化** |
| 有什么 | 5 个配置字段 + UCLASS 声明 | 一个构造函数 |
| 有逻辑吗 | 没有 | 几乎没有（就清零） |
| 类比 | 一张空白"配置表" | 给表格"出厂时先填上空白" |
| 对应篇目 | 01 / 02 | 03 |

**和 `ALyraPawn`（活对象）形成鲜明对比**：

| | `ULyraPawnData`（数据） | `ALyraPawn`（活对象） |
|---|---|---|
| `.cpp` 体量 | 极轻（一个构造） | 很重（队伍/网络/初始化逻辑一大堆） |
| 前缀 | `U`（数据资产） | `A`（Actor） |
| 职责 | 只装配置 | 执行行为 |
| 类比 | 图纸/菜谱 | 车/厨师 |

---

## 四、它和"谁"配合？（数据消费链）

`ULyraPawnData` 自己不干活，真正消费它的是这些系统：

| 消费者 | 读哪个字段 | 干嘛 |
|---|---|---|
| `GameMode` | `PawnClass` | spawn 出哪种角色 |
| `PawnExtensionComponent` | 保管整个 `PawnData` + 读 `TagRelationshipMapping` 喂 ASC | 总指挥收下配方、推进初始化 |
| `HeroComponent` | `InputConfig` / `DefaultCameraMode` | 绑输入、设镜头 |
| 能力授予系统 | `AbilitySets` | 授予技能/属性/效果 |

> 完整链路见 LyraPawnExtensionComponent 系列（尤其第 18 篇 PawnData 视角、25 篇三线关系）。

---

## 五、大图：整个类的 h+cpp 全貌（收束图）

```
                     ULyraPawnData（Pawn 的"配置单"）
 ═══════════════════════════════════════════════════════════
 .h（字段清单，57行）                .cpp（构造，14行）
 ┌──────────────────────────┐    ┌──────────────────────────┐
 │ UCLASS(UPrimaryDataAsset)│    │ 构造函数：                │
 │   Const 只读             │    │  : Super(ObjInit) ← 父类  │
 │   BlueprintType          │    │  清零 PawnClass           │
 │──────────────────────────│    │  清零 InputConfig         │
 │ ① PawnClass             │    │  清零 DefaultCameraMode   │
 │   = 造哪个身体            │    └──────────────────────────┘
 │ ② AbilitySets(数组)      │            │ 干净起点
 │   = 给哪些能力包          │            ▼
 │ ③ TagRelationshipMapping │    【加载 .uasset】序列化把策划配的值写入
 │   = 技能互斥规则          │            │
 │ ④ InputConfig           │            ▼
 │   = 按键怎么绑           │    一份"填好数据的配方"诞生
 │ ⑤ DefaultCameraMode     │
 │   = 默认镜头             │
 │──────────────────────────│
 │ 全字段 EditDefaultsOnly  │
 │ + BlueprintReadOnly      │
 │ = 只配一次·运行时只读      │
 └──────────────────────────┘
            ▼ 02 篇                      ▼ 03 篇
 ═══════════════════════════════════════════════════════════
 消费它的系统（自己不干活）：
   GameMode ──读 PawnClass──► 生成角色
   PawnExtension ──保管+喂 TagMapping──► 推初始化
   HeroComponent ──读 Input/Camera──► 绑输入/设镜头
   能力系统 ──读 AbilitySets──► 授予技能
```

---

## 六、本系列 3 篇索引

| 篇目 | 内容 |
|---|---|
| 01 | `ULyraPawnData` 是什么：数据资产 vs 活对象、和 `ALyraPawn` 的关系、数据驱动思想 |
| 02 | 五大配置字段逐个详解 + 为什么都 `EditDefaultsOnly, BlueprintReadOnly` |
| 03 | `.cpp` 构造函数逐行：为什么极简、为什么只清三个字段、策划值何时进入 |
| **本篇 99** | 收束：h 与 cpp 各干什么 + 大图 |

---

## 七、最后的记忆锚点

> **`ULyraPawnData` = 一份用数据描述"Pawn 该怎么造"的配方（UPrimaryDataAsset）。**
> - **`.h` 是字段清单**：声明 5 个"策划填的、运行时只读"的配置项（造谁/会什么/啥规则/咋操作/啥镜头），无任何逻辑（02 篇）。
> - **`.cpp` 是干净起点**：一个构造函数，让父类初始化 + 清零几个指针；真正内容靠加载 `.uasset` 时序列化填（03 篇）。
> - **自己不干活**：真正消费它的是 GameMode（读 PawnClass 生成）、PawnExtension（保管+喂 ASC）、HeroComponent（绑输入镜头）、能力系统（授技能）。
> - **设计哲学**：`U` 数据 vs `A` 对象、"造什么"和"怎么执行"分离、加新角色 = 建新资产不用改代码——这就是 Lyra 全工程的**数据驱动**基石（第 13 篇"读表"思维）。
