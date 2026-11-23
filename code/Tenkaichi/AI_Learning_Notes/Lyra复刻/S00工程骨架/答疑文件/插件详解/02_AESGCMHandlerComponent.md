# 02 — `AESGCMHandlerComponent` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **2** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Misc`
> 官方描述：Provides a packet handler component to do AES GCM encryption and decryption.
> 模块：`AESGCMHandlerComponent`(Runtime)

---

## 一、一句话

AES-256-GCM 网络封包加密组件，防抓包 / 防改包。

## 二、它到底做什么

继承 `FEncryptionComponent`，KeySize=32 字节（AES-256）、BlockSize=16、IV=12、AuthTag=16。收包 `Incoming()` 解密、发包 `Outgoing()` 加密。依赖 `PlatformCrypto` 插件做实际加解密。

## 三、有它 vs 没它

**光启用插件 = 零区别**。引擎只在 `DefaultEngine.ini` 的 `[PacketHandlerComponents]` 段读到 `EncryptionComponent=AESGCMHandlerComponent` 时才把它挂进 Handler 链（`PacketHandler.cpp:207`）。我们和 Lyra 都没配 → 组件从未被创建，网络仍明文。

## 四、Lyra 怎么用它

只在 `.uproject` 里启用，不配 ini，`Build.cs` 也不依赖它。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（除非将来要做反外挂加密）。

## 六、注意

想真正启用：① `"Enabled": true` ② ini 加段 ③ cvar `net.AllowEncryption` 不为 0。三者缺一不生效。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\PacketHandlers\AESGCMHandlerComponent\AESGCMHandlerComponent.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 2 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
