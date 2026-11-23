# 38 — `SteamSockets` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **38** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Networking`
> 官方描述：New Steamworks Networking code that supports the SteamSockets interface. NOTE: This plugin is only compatible with the SteamSockets Netdriver. It will not work if the proper netdriver definitions have not been set.
> 模块：`SteamSockets`(Runtime)

---

## 一、一句话

Steam 网络套接字 —— 走 Steam 中继/P2P 的 `ISocketSubsystem` 实现。

## 二、它到底做什么

模块：`SteamSockets`。让 UE 的 `FSocket` 底层走 Steamworks Networking（能穿透 NAT、走 Steam 中继）。

## 三、有它 vs 没它

不启用 → 只能用原生 UDP/TCP。**联机穿透能力下降**，但单机零影响。

## 四、Lyra 怎么用它

`.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

配合 `OnlineSubsystemSteam` 使用；`DefaultEngine.ini` 里可能要设 `[/Script/Engine.NetDriver]` 的 socket subsystem。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\Steam\SteamSockets\SteamSockets.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 38 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
