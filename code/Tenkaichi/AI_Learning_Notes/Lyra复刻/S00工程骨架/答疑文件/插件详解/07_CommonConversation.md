# 07 — `CommonConversation` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **7** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Gameplay`
> 官方描述：An *experimental* plugin for authoring graph-based conversation trees
> 模块：`CommonConversationRuntime`(Runtime), `CommonConversationGraph`(UncookedOnly), `CommonConversationEditor`(Editor)

---

## 一、一句话

基于节点图的 NPC 对话树系统（实验性）。

## 二、它到底做什么

三个模块：`CommonConversationRuntime`（运行时对话执行）、`CommonConversationGraph`（节点图数据）、`CommonConversationEditor`（对话编辑器 UI）。提供分支对话、条件判断、任务触发等节点。

## 三、有它 vs 没它

不启用 → 编辑器里没有「对话树」资产类型与编辑面板。运行时若代码里没用它则零影响。

## 四、Lyra 怎么用它

只在 `.uproject` 启用，Lyra 代码 0 处使用（Lyra 是射击游戏，不需要对话树）。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

**本复刻线永远不会用到**（Lyra 也没有对话玩法）。保留是一比一。

## 六、注意

官方标注 `*experimental*`（描述里就写着），API 可能变。将来若要做 RPG/剧情，可以直接用它。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\CommonConversation\CommonConversation.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 7 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
