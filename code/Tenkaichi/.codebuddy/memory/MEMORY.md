# MEMORY — 长期稳定事实（跨会话）

## 【开场强制动作】每次对话第一步（最高优先，无条件执行）
1. 先读 `AI_Learning_Notes\教学经验新对话必看\00_主要经验.md`（总入口，含三套路径+全部铁律索引）。
2. 要查引擎/Lyra 源码前，用下方写死的路径——**引擎在 `D:\ue5\Epic Games\UE_5.6`（D 盘！不是 E 盘）**。
3. 绝不凭直觉开干：找源码/写代码/给路径前，先确认依据。详见经验 13 号。

## 【固定约定】用户每次新对话会让我读经验目录（2026-09-22 确立）
- 用户会主动甩 `教学经验新对话必看\` 目录让我读。**我读完必须把核心（路径+铁律）焊进本 MEMORY.md**——用户出"触发"，我出"固化"。
- 本 MEMORY.md 每次自动加载，是跨会话记忆的落点；经验目录读到的关键都要同步到这里。

## 三套权威源码路径（教学必用，2026-09-22 固化）
| 资源 | 路径 |
|------|------|
| Tenkaichi 项目 | `E:\ue5\game_design\code\Tenkaichi` |
| Lyra 参考项目 | `E:\ue5\LyraStarterGame5.6\LyraStarterGame` |
| UE5.6 引擎源码 | `D:\ue5\Epic Games\UE_5.6`（**在 D 盘，不是 E 盘**；GAS 宏/引擎头在 `Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Public\` 等） |

- workspace 文件 `Tenkaichi.code-workspace` 已挂全三个根（Tenkaichi / LyraStarterGame / UE5）。查 Lyra/引擎源码优先用工作区根，不要到处 search 乱撞。
- 引擎目录巨大，`search_file` 易超时；已知文件名直接用精确绝对路径 `read_file`。

## 教学铁律（详见 AI_Learning_Notes\教学经验新对话必看\）
- 00：先查后教，基于真实源码，禁止凭记忆/想象编造；区分"源码事实"与"我的类比"。
- 01：一比一还原 Lyra，只教不写代码。
- 04/05：分步骤、每课 3~5 知识点、拆小。
- 07：每教一小步立刻建 md，不等提醒。
- 08：md 开头先讲"为什么"再给代码。
- 11：给教学代码前两个核对（对 Lyra 源码核 + .h↔.cpp 成对核）。
- 12：教的每个符号都要核实真实存在（能指出源码文件+行号）。

## 用户偏好
- 喜欢"新建 md + 剧场故事/类比 + 源码证据"的讲解。故事用 `AI_Learning_Notes\人物介绍\` 的剧场世界观（演员=角色、技能背包=ASC、血型档案卡=属性集、记分牌=PlayerState）。
- 讲解要极简，一次只走一小步；用户会反复要求"再拆细点"。
- 用户极度反感"跑题"和"凭脑子答"——问 A 就答 A，别扯 B。
