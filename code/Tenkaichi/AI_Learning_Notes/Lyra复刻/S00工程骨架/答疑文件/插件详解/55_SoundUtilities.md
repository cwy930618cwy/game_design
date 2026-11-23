# 55 — `SoundUtilities` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **55** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Audio`
> 官方描述：A variety of BP functions, objects, and utilities for audio.
> 模块：`SoundUtilities`(Runtime), `SoundUtilitiesEditor`(Editor)

---

## 一、一句话

音频工具函数库 —— 蓝图里常用的音频辅助节点。

## 二、它到底做什么

2 个模块：`SoundUtilities`（Runtime）+ `SoundUtilitiesEditor`（Editor）。如「按名字生成 SoundWave」「音频包络分析」「采样率转换」。

## 三、有它 vs 没它

不启用 → 蓝图里少一批音频节点。**游戏运行时零影响（除非蓝图用了）。**

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

工具库性质，不启用不报错。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\SoundUtilities\SoundUtilities.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 55 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
