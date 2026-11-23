# 34 — `OnlineServicesEOS` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **34** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Online Platform`
> 官方描述：Online Services implementation for EOS Account and Game services.
> 模块：`OnlineServicesEOS`(Runtime)

---

## 一、一句话

EOS 的**新版** OnlineServices 实现（账号 + 游戏服务）。

## 二、它到底做什么

模块：`OnlineServicesEOS`。对应 `UE::Online` 新接口（`IOnlineServices` / `IAuth` / `ISessions` 等），比旧 OSS 更现代。

## 三、有它 vs 没它

不启用 → 新接口拿不到 EOS 后端。单机零影响。

## 四、Lyra 怎么用它

`.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

新接口是 UE5 力推的方向；旧接口（33）还会长期共存。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Online\OnlineServicesEOS\OnlineServicesEOS.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 34 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
