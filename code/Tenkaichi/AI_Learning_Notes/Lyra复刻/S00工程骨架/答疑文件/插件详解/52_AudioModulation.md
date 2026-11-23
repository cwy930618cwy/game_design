# 52 — `AudioModulation` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **52** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Audio`
> 官方描述：Default implementation of Audio Modulation in the Unreal Audio Engine.
> 模块：`AudioModulation`(Runtime), `AudioModulationEditor`(Editor)

---

## 一、一句话

音频调制 —— 用参数化方式实时调混音（总线音量、滤波器）。

## 二、它到底做什么

2 个模块：`AudioModulation`（Runtime）+ `AudioModulationEditor`（Editor）。提供 `USoundControlBus` / `USoundModulationParameter` / `USoundModulationGenerator`，可用于「水下的闷声」「低血量心跳加响」。

## 三、有它 vs 没它

不启用 → `DefaultEngine.ini` 里 `AudioModulationPlugin=DefaultModulationPlugin` 失效。`LyraGame.Build.cs` 私有依赖它，**缺了编不过**。

## 四、Lyra 怎么用它

`.uproject` + `LyraGame.Build.cs` 私有依赖 + `S17` 的 `LyraAudioSettings` / `LyraAudioMixEffectsSubsystem`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S17（表现层 - 音频）**。

## 六、注意

Lyra 用它做「混音状态」（水下/濒死/暂停）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\AudioModulation\AudioModulation.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 52 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
