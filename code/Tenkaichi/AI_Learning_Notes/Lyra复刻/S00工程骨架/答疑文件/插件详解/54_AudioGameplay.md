# 54 — `AudioGameplay` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **54** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`
> 分类：`Audio`
> 官方描述：Core plugin for audio gameplay
> 模块：`AudioGameplay`(Runtime), `AudioGameplayTests`(Editor)

---

## 一、一句话

音频玩法核心 —— 53 号体积插件的底层框架。

## 二、它到底做什么

2 个模块：`AudioGameplay`（Runtime）+ `AudioGameplayTests`（Editor）。提供 `UAudioGameplayComponent` 基类与「音频代理」机制。

## 三、有它 vs 没它

不启用 → 53 号（`AudioGameplayVolume`）加载失败。

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

纯底层，业务不会直接碰它。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\AudioGameplay\AudioGameplay.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 54 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
