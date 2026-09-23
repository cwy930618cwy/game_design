# MEMORY — 每次对话自动加载（开场必读）

> 定位：本文件是「跨会话唯一可靠的记忆落点」。用户甩经验目录 = 触发，我写 MEMORY.md = 固化，两头一夹成闭环。
> 依据：`AI_Learning_Notes\教学经验新对话必看\00_主要经验.md` §二 + `13_每次对话必先过一遍教学经验_禁止凭直觉开干.md` §二

---

## ★ 开场强制动作（每次对话无条件先做）

**每次对话开始 / 每次要查源码 / 每次要给教学代码前：**

1. **先读 `E:\ue5\game_design\code\Tenkaichi\AI_Learning_Notes\教学经验新对话必看\00_主要经验.md`**（总入口）→ 拿到三套路径 + 铁律索引。
2. **按场景补读对应铁律文件**：
   - 要给教学代码 → `11`（两个核对）、`12`（符号核实）
   - 要查 Lyra / 引擎源码 → 用 `00` / `06` 写死的路径，**引擎在 D 盘 `D:\ue5\Epic Games\UE_5.6`**
   - 要新建 md / 文件夹 / 决定代码放哪个文件 → `05` / `07` / `08` / `09` / `10`
3. **用户甩经验目录过来时**：无条件逐个读完再干活，不许敷衍「我记住了」。
4. **绝不凭直觉跳步**：凡「找源码 / 写代码 / 给路径」，先确认依据再动手。

> ⚠️ AI 没有「每次自动读某目录」的能力，只有 MEMORY.md 会每次自动加载。所以本文件顶部这段开场动作是唯一保险。

---

## 一、三套权威路径（写死，禁止凭直觉改）

| 资源 | 路径 |
|------|------|
| **Tenkaichi（用户自己的项目）** | `E:\ue5\game_design\code\Tenkaichi` |
| **Lyra 参考项目** | `E:\ue5\LyraStarterGame5.6\LyraStarterGame` |
| **UE 5.6 引擎源码** | `D:\ue5\Epic Games\UE_5.6` ← **D 盘！不是 E 盘、不是 UE_5.0** |
| 笔记根目录 | `E:\ue5\game_design\code\Tenkaichi\AI_Learning_Notes\` |
| 教学经验目录 | `E:\ue5\game_design\code\Tenkaichi\AI_Learning_Notes\教学经验新对话必看\` |
| 从零开发教学目录 | `E:\ue5\game_design\code\Tenkaichi\AI_Learning_Notes\Lyra_从零开发教学\` |
| workspace | `Tenkaichi.code-workspace`（含 Tenkaichi / LyraStarterGame / UE5.6 三个文件夹） |

> 只有一套路径，不再区分公司/家里。

---

## 二、教学铁律速查（00~13）

| # | 铁律（一句话） |
|---|----------------|
| 00 | **最高铁律**：教学必须基于真实源码，**先查后教**；找不到就明说「未找到源码依据」，**绝不编**；区分「源码事实」与「我的类比」 |
| 01 | **一比一还原 Lyra** + **只教不写代码**（用户工程的代码由用户自己写）；**例外：打印日志（UE_LOG/PrintString）我来写、也我来删** |
| 02 | **严格按 `Lyra_从零开发教学\` 目录课次顺序推进**，不跳步、不自创路线 |
| 03 | 教学中**每次遇到问题**（疑问/踩坑/报错/易混点），**我主动新建 md 总结**，不等用户提醒 |
| 04 | **小步慢走**：一次只教一步，**禁止一次性甩一大坨代码**；讲完停下等用户「懂了/做完/继续」再往下 |
| 05 | 每课落地成**多个小 md**，**每个 md 只放 3~5 个知识点**；用户明确说过「我很讨厌文件太大」 |
| 06 | 三套路径统一，不再区分公司/家里 |
| 07 | **每教完一小步，当轮立刻建对应 md**；拆几步=建几个 md；「先建后教」也行；**禁止只口头讲不落文件，禁止等用户催（催一次=失职一次）** |
| 08 | **先讲「为什么这么写」，再给代码**；给完代码要逐段回扣「为什么」。禁止上来丢代码让用户猜 |
| 09 | **新建文件/文件夹的路径要一比一映射 Lyra 目录**，禁止自创 Lyra 里没有的文件夹（如 `AttributeSet/` 是错的，正确是 `AbilitySystem/Attributes/`） |
| 10 | **代码放哪个文件也要对应 Lyra**：Lyra 一个类一个文件就教一个类一个文件，**禁止擅自合并/拆分**，禁止拿「简化」当借口乱放 |
| 11 | 给教学代码前做**两个核对**：①对着 Lyra 源码核有没有缺行漏声明 ②`.h` 声明 ↔ `.cpp` 实现**成对**（名字/参数/返回值/const/override 一致） |
| 12 | 教的**每个符号**（类/属性/方法/宏/include/流程）都要到源码核实「真实存在 + 签名正确」，**禁止凭脑子觉得有**；判断标准：能指出「在哪个文件第几行」 |
| 13 | 元铁律：经验不会自动进脑子，每次对话/查源码前**必须先过一遍教学经验**，禁止凭直觉开干 |

**核对链条顺序**：先 12（符号真实存在）→ 再 11①（照 Lyra 没漏）→ 再 11②（.h/.cpp 成对）。

---

## 三、Lyra → Tenkaichi 目录映射（LyraGame 换 Tenkaichi、Lyra 前缀换 Tenkaichi）

| Lyra 真实路径（`Source/LyraGame/`） | Tenkaichi 对应路径（`Source/Tenkaichi/`） |
|---|---|
| `AbilitySystem/Attributes/LyraAttributeSet.h/.cpp` | `AbilitySystem/Attributes/TenkaichiAttributeSet.h/.cpp`（基类：宏+空基类+工具，**不含属性**） |
| `AbilitySystem/Attributes/LyraHealthSet.h/.cpp` | `AbilitySystem/Attributes/TenkaichiHealthSet.h/.cpp`（血量集：真正定义 Health） |
| `AbilitySystem/LyraAbilitySystemComponent.h` | `AbilitySystem/TenkaichiAbilitySystemComponent.h` |
| `AbilitySystem/Abilities/LyraGameplayAbility.h` | `AbilitySystem/Abilities/...` |
| `AbilitySystem/Effects/...` | `AbilitySystem/Effects/...` |
| `AbilitySystem/Executions/...` | `AbilitySystem/Executions/...` |
| `Character/LyraCharacterWithAbilities.h` | `Character/TenkaichiCharacter.h` |

> 映射原则：**Lyra 目录树 = Tenkaichi 目录树**，只换项目前缀/类名，保留层级。

---

## 四、用户项目状态 & 当前进度

- **Tenkaichi**：UE5.6 空白 C++ Game 工程（2026-09-22 已重置为干净状态，无 GAS 痕迹）。
- **教学目录现状**：
  - `00_教学开篇与路线.md`、`00a_按Lyra写法开发前的准备.md`、`01_教程设计总纲_贴合Tenkaichi项目.md` ✅ 已有
  - `阶段一：GAS 打通\` 下已有：`02_1_给工程加GAS依赖`、`02_2_1_基类TenkaichiAttributeSet_h`、`02_2_2_基类TenkaichiAttributeSet_cpp`、`02_2_3_血量集TenkaichiHealthSet_h`、`02_2_4_血量集TenkaichiHealthSet_cpp`、`02_3_1_为什么要有自定义ASC`（2026-09-24 开教第 3 步）
  - `教学疑问解答\` 已有 18 个 md
- **继续点**：第 02 课 · 第 3 步（建自定义 ASC `TenkaichiAbilitySystemComponent`）。已定**路线 A**：本步只还原 Lyra 里不依赖其他 Lyra 子系统的部分（类声明 + 构造函数）；`EndPlay` / `InitAbilityActorInfo` / 输入处理 / 激活组函数等**在第 03 课后回填同一个文件**（属教学分阶段，非结构简化，最终仍一比一）。下一步 = `02_3_2_自定义ASC_h骨架`。每次开课前先重新 ls `Lyra_从零开发教学\` 确认最新进度，再定点推进。

---

## 五、讲课方式偏好

1. 每课前**先给「拆几步」清单**（用户要先看到地图）。
2. 每小步顺序：**为什么 → 概念/类比 → Lyra 真实源码片段（文件+函数+行）→ 落到 Tenkaichi 哪个文件哪个函数 → 停，等反馈**。
3. 每小步讲完**当轮立刻建 md**（`课次_步次_主题.md`，如 `02_2_1_xxx.md`，用 `_` 分层不用 `.`）。
4. 每个 md 含：①这步做出什么+验收 ②概念 ③Lyra 源码依据 ④Tenkaichi 落点 ⑤常见坑/一句话结论。
5. 讲解用**大白话 + 类比**，禁止堆术语；类比要显式标注「这是我的类比，非源码」。
