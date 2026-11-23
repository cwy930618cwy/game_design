# 65 — `GameplayInteractions` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **65** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Gameplay`
> 官方描述：Player and NPC interactions
> 模块：`GameplayInteractionsModule`(Runtime)

---

## 一、一句话

玩法交互框架 —— 基于 SmartObject 的「玩家/NPC 与场景物体交互」。

## 二、它到底做什么

模块：`GameplayInteractionsModule`（Runtime）。提供 `UGameplayInteraction` 状态树节点、交互任务。

## 三、有它 vs 没它

不启用 → 不能做「走到椅子前坐下」「按 E 撬开箱子」这类 SmartObject 交互。

## 四、Lyra 怎么用它

只在 `.uproject` 启用（Lyra 的 Interact 系统是 `S16` 自己写的，不完全靠它）。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（Lyra 的交互在 S16 是自有实现）。

## 六、注意

它依赖 66 号（`SmartObjects`）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\GameplayInteractions\GameplayInteractions.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 65 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
