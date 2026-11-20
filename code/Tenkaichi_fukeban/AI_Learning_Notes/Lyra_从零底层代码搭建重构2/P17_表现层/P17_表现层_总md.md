# P17 表现层 —— Feedback / Animation / Audio / Cosmetics（阶段总 md）

> **定位**：本文件是 P17（表现层）的**总入口 + 全景地图**。目标：还原 Lyra 的"游戏表现"——特效反馈、动画、音频、外观定制。
> 源码依据：`Source/LyraGame/` 下 4 个目录（36 个文件，约 20 个类）
> 命名映射：`Lyra`→`Tenkaichi`，其余一字不改（铁律 24）。

---

## 〇、为什么先建这个总 md（铁律 35）

P17 是 4 个目录、20 个类的大块内容，必须先建阶段总 md（全景 + 依赖 + 子阶段规划），**停下等确认**。

> 上一节 P16（UI，42 个类）已全部完成。

---

## 一、这一节解决什么问题（先讲为什么）

前面 P00~P16 把"游戏逻辑"搭完了（角色、能力、武器、UI）。但游戏还缺"**感觉**"：
- 打中敌人要有**数字飘字**（伤害数字）。
- 角色走路要有**脚步声**、开枪要有**枪声**。
- 角色要有**动画**（跑/跳/瞄准）。
- 角色外观要能**换装**（皮肤/配件）。

P17 就是这四块"表现层"。

> **类比**：P00~P16 是"骨架和肌肉"（能跑能打），P17 是"皮肤、声音、表情"（让它像个活人）。

---

## 二、全景地图（4 个目录）

### 1. `Feedback/`（19 文件，9 个类）—— 反馈特效

| 子目录 | 类 | 作用 |
|--------|----|------|
| `ContextEffects/` | `ULyraContextEffectComponent` | 上下文特效组件（按"表面类型"播特效/音效） |
| | `ULyraContextEffectsInterface` | 接口（让动画通知能触发特效） |
| | `ULyraContextEffectsLibrary` | 特效库（数据资产：表面类型 → 特效） |
| | `ULyraContextEffectsSubsystem` | 子系统（管理特效库） |
| | `UAnimNotify_LyraContextEffects` | 动画通知（在动画某帧触发特效） |
| `NumberPops/` | `ULyraNumberPopComponent` | 数字飘字组件基类 |
| | `ULyraNumberPopComponent_MeshText` | 用 3D 文字网格实现 |
| | `ULyraNumberPopComponent_NiagaraText` | 用 Niagara 实现 |
| | `ULyraDamagePopStyle` / `ULyraDamagePopStyleNiagara` | 飘字样式数据资产 |

### 2. `Animation/`（2 文件，1 个类）—— 动画

| 类 | 作用 |
|----|------|
| `ULyraAnimInstance` | 角色动画实例（算速度/朝向/落地等动画参数） |

### 3. `Audio/`（4 文件，2 个类）—— 音频

| 类 | 作用 |
|----|------|
| `ULyraAudioSettings` | 音频设置（开发者设置：默认控制总线混音） |
| `ULyraAudioMixEffectsSubsystem` | 混音效果子系统（按"音频模式"切换混音） |

### 4. `Cosmetics/`（11 文件，6 个类）—— 外观定制

| 类 | 作用 |
|----|------|
| `FLyraCharacterPart` / `FLyraCharacterPartList` | 角色部件（数据：部件类 + 骨骼名 + 碰撞） |
| `ULyraPawnComponent_CharacterParts` | Pawn 上的部件组件（管理部件增删） |
| `ULyraControllerComponent_CharacterParts` | Controller 上的部件组件（跨 Pawn 保留） |
| `ULyraCosmeticAnimationTypes` | 外观动画类型（动画层选择） |
| `ULyraCosmeticDeveloperSettings` | 外观开发者设置 |
| `ULyraCosmeticCheats` | 外观作弊命令 |

---

## 三、依赖分析（铁律 30/31）

| 目录 | 主要依赖 | 状态 |
|------|---------|------|
| `Animation/` | P06 `ATenkaichiCharacter` + `UTenkaichiCharacterMovementComponent`（已教）+ GAS | ✅ 零挂起 |
| `Audio/` | **`Settings/TenkaichiSettingsLocal.h`（P18 未建）** + `LoadingScreenManager`（CommonLoadingScreen 插件 S1） | ⚠️ 挂起 |
| `Feedback/ContextEffects/` | 引擎（Niagara/AnimNotify）+ P06 Character | ✅ 基本零挂起 |
| `Feedback/NumberPops/` | 引擎（Niagara/TextRender）+ P05 GAS | ✅ 基本零挂起 |
| `Cosmetics/` | P06 Character + P12 PlayerController + P18 DeveloperSettings | ⚠️ 部分挂起 |

> ⚠️ **主要断点**：`Audio/` 依赖 P18 Settings；`Cosmetics/` 部分依赖 P18 DeveloperSettings。

---

## 四、教学顺序规划（子阶段拆分）

| 子阶段 | 内容 | 优先级 |
|--------|------|--------|
| **P17.1** | **Animation**（1 类）：`ULyraAnimInstance` | 🔴 最高（最基础，被其他依赖） |
| **P17.2** | **Cosmetics 数据**（2 类）：`FLyraCharacterPart` / `FLyraCharacterPartList` | 🔴 高（被组件依赖） |
| **P17.3** | **Cosmetics 组件**（2 类）：`ULyraPawnComponent_CharacterParts` / `ULyraControllerComponent_CharacterParts` | 🔴 高 |
| **P17.4** | **Cosmetics 其余**（3 类）：`ULyraCosmeticAnimationTypes` / `ULyraCosmeticDeveloperSettings` / `ULyraCosmeticCheats` | 🟡 中 |
| **P17.5** | **Feedback ContextEffects**（5 类） | 🟡 中 |
| **P17.6** | **Feedback NumberPops**（4 类） | 🟡 中 |
| **P17.7** | **Audio**（2 类） | 🟢 低（挂 P18） |

> 建议从 **P17.1（Animation）** 开始——最基础，且零挂起。

---

## 五、待确认

1. P17 是 4 个目录、20 个类，按 **7 个子阶段**拆分（先 Animation）认可吗？
2. 从 **P17.1 `ULyraAnimInstance`** 开始，OK 吗？
3. 每个类仍走「先自己的总 md → 停 → 教 h → 停 → 教 cpp」（铁律 20/29/35）吗？
4. 确认后我建 **P17.1 子阶段总 md**，然后从 `ULyraAnimInstance` 开始教。

---

## 六、源码依据

- `Source/LyraGame/Feedback/`（19 文件）
- `Source/LyraGame/Animation/`（2 文件）
- `Source/LyraGame/Audio/`（4 文件）
- `Source/LyraGame/Cosmetics/`（11 文件）
- 你的工程：`Source/TenkaichiGame/{Feedback,Animation,Audio,Cosmetics}/`（目录待建）