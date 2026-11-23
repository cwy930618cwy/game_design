# 61 — `ResonanceAudio` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **61** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": false`，`EnabledByDefault` = `True`
> 分类：`Audio`
> 官方描述：3D audio spatialization and room acoustics simulation plugin by Google.
> 模块：`ResonanceAudio`(Runtime), `ResonanceAudioEditor`(Editor)

---

## 一、一句话

Google Resonance Audio —— 3D 空间音频与房间声学模拟。

## 二、它到底做什么

2 个模块：`ResonanceAudio`（Runtime）+ `ResonanceAudioEditor`（Editor）。提供 HRTF 双耳渲染、早期反射、混响。

## 三、有它 vs 没它

`Enabled: false` → 不加载。**本机有该插件**，将来改 `true` 也不会报错。

## 四、Lyra 怎么用它

`.uproject` 里 `Enabled: false`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（除非做 VR 空间音频）。

## 六、注意

与 63 号（`Spatialization`）是竞品；Lyra 两个都没开（用引擎内置的空间化）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\ResonanceAudio\ResonanceAudio.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 61 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
