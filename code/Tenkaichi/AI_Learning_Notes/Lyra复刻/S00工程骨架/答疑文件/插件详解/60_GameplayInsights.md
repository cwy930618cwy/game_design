# 60 — `GameplayInsights` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **60** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Insights`
> 官方描述：Allows debugging of animation systems via Unreal Insights
> 模块：`GameplayInsights`(EditorAndProgram), `GameplayInsightsEditor`(Editor), `RewindDebugger`(Editor), `RewindDebuggerVLog`(Editor)

---

## 一、一句话

Unreal Insights 的游戏玩法调试支持（GAS 运行状态可视化）。

## 二、它到底做什么

2 个模块：`GameplayInsights`（EditorAndProgram）+ `GameplayInsightsEditor`（Editor）。把 GameplayAbility / GameplayEffect 的运行时状态导出到 Unreal Insights 时间轴。

## 三、有它 vs 没它

不启用 → Insights 里看不到 GAS 事件。**运行时只有 Trace 开销，功能零影响。**

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

调 GAS 时很有用（S10 之后你会想开它）。

## 六、注意

配合 `-trace=gameplay` 命令行使用。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Animation\GameplayInsights\GameplayInsights.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 60 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
