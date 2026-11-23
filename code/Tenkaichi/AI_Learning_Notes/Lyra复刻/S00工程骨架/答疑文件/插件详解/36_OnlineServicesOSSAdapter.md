# 36 — `OnlineServicesOSSAdapter` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **36** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Online Platform`
> 官方描述：Online Services adapter for Online Subsystem implementations.
> 模块：`OnlineServicesOSSAdapter`(Runtime)

---

## 一、一句话

把旧 OnlineSubsystem 适配到新 OnlineServices 的桥接层。

## 二、它到底做什么

模块：`OnlineServicesOSSAdapter`。让你用新接口写代码，底层实际走旧 OSS 实现（比如 Steam OSS）。

## 三、有它 vs 没它

不启用 → 新接口用不了旧 OSS 后端（Steam 只能走旧接口）。

## 四、Lyra 怎么用它

`.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

Lyra 同时启用 33/34/35/36，是为了「新旧接口都能跑、离线也能跑」。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Online\OnlineServicesOSSAdapter\OnlineServicesOSSAdapter.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 36 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
