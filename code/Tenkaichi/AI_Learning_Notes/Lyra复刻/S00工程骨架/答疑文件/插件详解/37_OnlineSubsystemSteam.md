# 37 — `OnlineSubsystemSteam` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **37** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Online Platform`
> 官方描述：Access to Steam platform
> 模块：`OnlineSubsystemSteam`(Runtime)

---

## 一、一句话

Steam 平台的 OnlineSubsystem 实现。

## 二、它到底做什么

模块：`OnlineSubsystemSteam`。需要 Steamworks SDK + `steam_appid.txt`。

## 三、有它 vs 没它

不启用 → Steam 登录/好友/成就不可用。单机零影响。

## 四、Lyra 怎么用它

`.uproject` 启用（Lyra 支持 Steam 打包）。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（除非上架 Steam）。

## 六、注意

要真用需要 Steamworks SDK，否则启动会警告找不到 `steam_api64.dll`。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Online\OnlineSubsystemSteam\OnlineSubsystemSteam.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 37 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
