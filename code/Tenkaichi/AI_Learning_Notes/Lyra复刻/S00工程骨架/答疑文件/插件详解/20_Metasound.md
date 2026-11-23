# 20 — `Metasound` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **20** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `True`
> 分类：`Audio`
> 官方描述：A high-performance audio system that enables sound designers to have complete control over audio DSP graph generation of sound sources, via sample-accurate control and modulation of sound using audio parameters and audio events from game data and Blueprints
> 模块：`MetasoundGraphCore`(Runtime), `MetasoundGenerator`(Runtime), `MetasoundFrontend`(Runtime), `MetasoundStandardNodes`(Runtime), `MetasoundEngine`(Runtime), `MetasoundEngineTest`(Runtime), `MetasoundEditor`(UncookedOnly)

---

## 一、一句话

★ MetaSounds —— UE5 的新一代程序化音频系统，替代旧 SoundCue。

## 二、它到底做什么

7 个模块：`MetasoundGraphCore` / `MetasoundGenerator` / `MetasoundFrontend` / `MetasoundStandardNodes` / `MetasoundEngine` / `MetasoundEngineTest` / `MetasoundEditor`。用节点图定义音频 DSP（合成器、调制、采样播放）。`EnabledByDefault: true`。

## 三、有它 vs 没它

不启用 → 所有 `.MetaSoundSource` 资产消失。**UE5 默认音频管线本身依赖它**（新工程的默认音效都是 MetaSound）。

## 四、Lyra 怎么用它

`.uproject` + `DefaultEngine.ini` 的音频段（S17 补）+ `S17` 的 `LyraAudioSettings` / `LyraAudioMixEffectsSubsystem`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

**S17（表现层 - 音频）**。

## 六、注意

旧项目还在用 SoundCue 的话，MetaSound 可以先只启用不深究；但 Lyra 用的是 MetaSound。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\Metasound\Metasound.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 20 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
