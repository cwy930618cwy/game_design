# 69 — `GameplayStateTree` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **69** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Gameplay`
> 官方描述：StateTree for AI/Gameplay Behaviors
> 模块：`GameplayStateTreeModule`(Runtime)

---

## 一、一句话

StateTree for AI/Gameplay Behaviors —— 状态树扩展。

## 二、它到底做什么

模块：`GameplayStateTreeModule`（Runtime）。让 `UStateTree` 能驱动 GameplayBehavior / SmartObject 行为。

## 三、有它 vs 没它

不启用 → AI 状态树里没有 Gameplay 相关节点。

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

StateTree 本身是引擎内置（`StateTreeModule`），这个是玩法扩展。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\GameplayStateTree\GameplayStateTree.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 69 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
