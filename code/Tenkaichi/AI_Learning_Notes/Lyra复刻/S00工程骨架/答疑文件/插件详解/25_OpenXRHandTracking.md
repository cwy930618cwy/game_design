# 25 — `OpenXRHandTracking` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **25** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": false`，`EnabledByDefault` = `False`
> 分类：`Virtual Reality`
> 官方描述：OpenXR Hand Tracking provides XR_EXT_hand_tracking support.
> 模块：`OpenXRHandTracking`(Runtime), `OpenXRHandTrackingEditor`(Editor)

---

## 一、一句话

OpenXR 手部追踪（`XR_EXT_hand_tracking`）。

## 二、它到底做什么

2 个模块：`OpenXRHandTracking`（Runtime，`PostConfigInit`）+ `OpenXRHandTrackingEditor`（Editor）。把手部关节数据接到 UE 的 `IHandTracker`。

## 三、有它 vs 没它

`Enabled: false` → 零影响。本机有。

## 四、Lyra 怎么用它

`.uproject` 里 `Enabled: false`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

同上。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\OpenXRHandTracking\OpenXRHandTracking.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 25 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
