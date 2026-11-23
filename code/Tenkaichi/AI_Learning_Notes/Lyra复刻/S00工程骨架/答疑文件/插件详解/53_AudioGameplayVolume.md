# 53 — `AudioGameplayVolume` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **53** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`
> 分类：`Audio`
> 官方描述：Audio Gameplay Volume Plugin
> 模块：`AudioGameplayVolume`(Runtime), `AudioGameplayVolumeEditor`(Editor)

---

## 一、一句话

音频玩法体积 —— 在关卡里划区域，进入后自动改混音。

## 二、它到底做什么

2 个模块：`AudioGameplayVolume`（Runtime）+ `AudioGameplayVolumeEditor`（Editor）。提供 `AAudioVolume` 的升级版：`UAaudioGameplayVolumeComponent` + 若干「代理」（混响代理、滤波代理、子混音代理）。

## 三、有它 vs 没它

不启用 → 关卡里不能放这类体积。**单机开发零影响。**

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（除非做区域混音）。

## 六、注意

依赖 `AudioGameplay`（54 号）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\AudioGameplayVolume\AudioGameplayVolume.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 53 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
