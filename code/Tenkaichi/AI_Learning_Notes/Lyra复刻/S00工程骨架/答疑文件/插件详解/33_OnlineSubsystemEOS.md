# 33 — `OnlineSubsystemEOS` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **33** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Online Platform`
> 官方描述：Online Subsystem for Epic Online Services
> 模块：`OnlineSubsystemEOS`(Runtime), `OnlineSubsystemEOSPlus`(Runtime)

---

## 一、一句话

Epic Online Services（EOS）的旧版 OnlineSubsystem 实现。

## 二、它到底做什么

2 个模块：`OnlineSubsystemEOS` + `OnlineSubsystemEOSPlus`（组合多个 OSS，比如 Steam 登录 + EOS 社交）。提供 `IOnlineSubsystem` 的 EOS 实现。

## 三、有它 vs 没它

不启用 → `IOnlineSubsystem::Get("EOS")` 返回 null，登录/会话/成就全不可用。**单机开发零影响。**

## 四、Lyra 怎么用它

`.uproject` 启用 + `DefaultEngine.ini` 里有 EOS 相关配置（Lyra 用它做跨平台账号）。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（除非做联机登录）。

## 六、注意

EOS 是「旧接口」；34~36 是「新接口」。Lyra 两套都启用了，因为 CommonUser 插件新旧都支持。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Online\OnlineSubsystemEOS\OnlineSubsystemEOS.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 33 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
