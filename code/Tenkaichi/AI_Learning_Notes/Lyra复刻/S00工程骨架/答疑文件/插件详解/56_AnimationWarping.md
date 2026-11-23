# 56 — `AnimationWarping` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **56** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Animation`
> 官方描述：Framework for animation and pose warping. This plugin includes Stride, Orientation, and Slope Warping alongside the Root Motion Delta animation attribute.
> 模块：`AnimationWarpingRuntime`(Runtime), `AnimationWarpingEditor`(UncookedOnly)

---

## 一、一句话

动画扭曲 —— 步幅 / 朝向 / 坡度三类 Warping，解决脚部滑动与贴地。

## 二、它到底做什么

2 个模块：`AnimationWarpingRuntime` + `AnimationWarpingEditor`。提供 `FAnimNode_OrientationWarping`（转向时上身不跟着转）、`FAnimNode_StrideWarping`（步幅匹配速度）、`FAnimNode_SlopeWarping`（脚踩斜坡）。

## 三、有它 vs 没它

不启用 → 动画蓝图用不了这些节点。**Lyra 的角色动画如果依赖它，脚会滑。**

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（除非自己搭 ABP）。

## 六、注意

UE5 的 Motion Matching + Warping 是现代角色动画的标配。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Animation\AnimationWarping\AnimationWarping.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 56 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
