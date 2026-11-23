# 63 — `Spatialization` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **63** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`
> 分类：`Audio`
> 官方描述：Plugin featuring a variety of basic audio spatialization solutions.
> 模块：`Spatialization`(Runtime), `SpatializationEditor`(Editor)

---

## 一、一句话

空间化基础库 —— 提供若干基础 3D 音频空间化方案。

## 二、它到底做什么

2 个模块：`Spatialization`（Runtime）+ `SpatializationEditor`（Editor）。含「Simple ITD」（双耳时间差）等实现。

## 三、有它 vs 没它

不启用 → `DefaultEngine.ini` 的 `SpatializationPlugin=Simple ITD`（Lyra 配的）找不到实现。**我们是空的，所以零影响。**

## 四、Lyra 怎么用它

`.uproject` 启用 + `DefaultEngine.ini` 的 `[/Script/WindowsTargetPlatform.WindowsTargetSettings] SpatializationPlugin=Simple ITD`（Lyra `:282`）。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

S17 补音频配置时会用到（Lyra 配了 `Simple ITD`）。

## 六、注意

我们现在 `SpatializationPlugin=` 是空的，所以暂时没用到它。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\Spatialization\Spatialization.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 63 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
