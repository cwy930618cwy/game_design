# 35 — `OnlineServicesNull` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **35** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Online Platform`
> 官方描述：Online Services implementation without an external service.
> 模块：`OnlineServicesNull`(Runtime)

---

## 一、一句话

OnlineServices 的**空实现** —— 不连任何外部服务。

## 二、它到底做什么

模块：`OnlineServicesNull`。所有接口都返回「成功但什么也没做」，用于离线开发/单机测试。

## 三、有它 vs 没它

不启用 → 离线时新接口可能拿不到默认实现。**它是离线开发的兜底。**

## 四、Lyra 怎么用它

`.uproject` 启用 + `DefaultEngine.ini` 的 `[OnlineServices] DefaultServices=Null`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（它保证你断网也能启动）。

## 六、注意

★ 建议永远保持启用 —— 这是「没网也能跑编辑器」的保障。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Online\OnlineServicesNull\OnlineServicesNull.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 35 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
