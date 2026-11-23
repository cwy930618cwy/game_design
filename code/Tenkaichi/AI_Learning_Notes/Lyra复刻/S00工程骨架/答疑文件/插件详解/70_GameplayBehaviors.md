# 70 — `GameplayBehaviors` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **70** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Gameplay`
> 官方描述：Encapsulated fire-and-forget behaviors for AI agents
> 模块：`GameplayBehaviorsModule`(Runtime), `GameplayBehaviorsModule`(UncookedOnly), `GameplayBehaviorsEditorModule`(Editor)

---

## 一、一句话

封装好的「发射即忘」AI 行为（fire-and-forget behaviors）。

## 二、它到底做什么

2 个模块：`GameplayBehaviorsModule`（Runtime + UncookedOnly 各一份）。提供 `UGameplayBehavior`（一个短时行为：跑到某处、看向某物、播放动画）与 `UGameplayBehavior_AnimationBased`。

## 三、有它 vs 没它

不启用 → 不能给 AI 下发「独立任务」。

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

相比 StateTree 的「持续状态」，GameplayBehavior 是「一次性任务」。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\GameplayBehaviors\GameplayBehaviors.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 70 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
