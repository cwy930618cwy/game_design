# 67 — `ContextualAnimation` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **67** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Animation`
> 官方描述：NONE
> 模块：`ContextualAnimation`(Runtime), `ContextualAnimationEditor`(Editor)

---

## 一、一句话

情境动画 —— 翻越、开门、爬梯这类「按环境播特定动画」的框架。

## 二、它到底做什么

2 个模块：`ContextualAnimation`（Runtime）+ `ContextualAnimationEditor`（Editor）。提供 `UContextualAnimSceneAsset`（场景动画资产：定义多个角色的同步动画）、`UContextualAnimSceneInstance`（运行时实例）。

## 三、有它 vs 没它

不启用 → 不能做「两个人一起抬箱子」「翻越掩体」这类双人/环境同步动画。

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（除非做攀爬/掩体系统）。

## 六、注意

与 Motion Matching 是互补关系，不冲突。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Animation\ContextualAnimation\ContextualAnimation.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 67 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
