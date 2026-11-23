# 51 — `AnimationLocomotionLibrary` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **51** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Animation`
> 官方描述：Collection of techniques for driving locomotion animations
> 模块：`AnimationLocomotionLibraryRuntime`(Runtime), `AnimationLocomotionLibraryEditor`(UncookedOnly)

---

## 一、一句话

移动动画库 —— 步态/转向/倾斜等 locomotion 动画的通用节点。

## 二、它到底做什么

2 个模块：`AnimationLocomotionLibraryRuntime` + `AnimationLocomotionLibraryEditor`。给动画蓝图提供「根据速度算步态」「转向倾斜」等节点。

## 三、有它 vs 没它

不启用 → 动画蓝图里没有这些节点。**Lyra 若有 ABP 用了它，动画会报节点缺失。**

## 四、Lyra 怎么用它

只在 `.uproject` 启用（Lyra 的 `LyraAnimInstance` 在 S17）。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（除非自己搭 ABP）。

## 六、注意

Lyra 的动画资产是二进制的，代码复刻不涉及。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Animation\AnimationLocomotionLibrary\AnimationLocomotionLibrary.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 51 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
