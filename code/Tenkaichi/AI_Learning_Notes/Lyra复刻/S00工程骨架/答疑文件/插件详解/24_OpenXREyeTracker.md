# 24 — `OpenXREyeTracker` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **24** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": false`，`EnabledByDefault` = `False`
> 分类：`Virtual Reality`
> 官方描述：OpenXR Eye Tracker provides XR_EXT_eye_gaze_interaction support.
> 模块：`OpenXREyeTracker`(Runtime)

---

## 一、一句话

OpenXR 眼动追踪（`XR_EXT_eye_gaze_interaction`）。

## 二、它到底做什么

Runtime 模块，`LoadingPhase=PostConfigInit`。给头显的注视点数据接到 UE 的 `IEyeTracker` 接口。

## 三、有它 vs 没它

`Enabled: false` → 不加载，零影响。**本机有该插件**，所以将来改 `true` 也不会报错。

## 四、Lyra 怎么用它

`.uproject` 里 `Enabled: false`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

与 25 / 26 一起构成 OpenXR 家族；26（`OpenXRHMD`）本机已合并进 `OpenXR` 主插件。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\OpenXREyeTracker\OpenXREyeTracker.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 24 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
