# 31 — `OnlineFramework` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **31** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Online Platform`
> 官方描述：Shared code for interacting with online gameplay services.
> 模块：`Qos`(Runtime), `Party`(Runtime), `Lobby`(Runtime), `PatchCheck`(Runtime), `Hotfix`(Runtime), `Rejoin`(Runtime), `PlayTimeLimit`(Runtime), `LoginFlow`(ClientOnly)

---

## 一、一句话

在线框架：Qos / Party / Lobby / PatchCheck / Hotfix 等共享代码。

## 二、它到底做什么

一个 `.uplugin` 里打包了 10+ 个模块：`Qos`（网络质量探测）、`Party`（跨平台组队）、`Lobby`（大厅）、`PatchCheck`（版本校验）、`Hotfix`（热更）等。为上层 OSS / OnlineServices 提供共用能力。

## 三、有它 vs 没它

不启用 → `CommonGame` 插件编不过（它的 `.uplugin` 依赖 `OnlineFramework`）。

## 四、Lyra 怎么用它

`.uproject` + `CommonGame` 插件依赖。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\CommonGame\CommonGame.uplugin` (x1)

## 五、本复刻线什么时候碰它

**S03（CommonGame 插件）** 时必须就位。

## 六、注意

这是「一堆小模块打成一个插件」的典型例子 —— 看 `.uplugin` 的 `Modules` 数组会很长。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Online\OnlineFramework\OnlineFramework.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 31 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
