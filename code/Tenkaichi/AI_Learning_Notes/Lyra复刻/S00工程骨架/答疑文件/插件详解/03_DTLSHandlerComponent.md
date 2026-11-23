# 03 — `DTLSHandlerComponent` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **3** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Misc`
> 官方描述：Provides a packet handler component to do DTLS encryption and decryption.
> 模块：`DTLSHandlerComponent`(Runtime)

---

## 一、一句话

DTLS 网络封包加密组件，主要给语音 / UDP 实时流用。

## 二、它到底做什么

与 AESGCM 同属 `Runtime\PacketHandlers\` 家族，基于 DTLS（TLS 的 UDP 版）。同样继承 `FEncryptionComponent`，通过 `[PacketHandlerComponents]` 配置挂载。

## 三、有它 vs 没它

不配 ini 就零区别。**但 Lyra 的 `LyraGame.Build.cs` 私有依赖里写了它**（第 04 轮讲义第 25 项），所以模块会编译、头文件可 include。

## 四、Lyra 怎么用它

`.uproject` 启用 + `Source\LyraGame\LyraGame.Build.cs` 私有依赖。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

S00 `Build.cs` 已经依赖它（不用改）；功能本身永远不用管。

## 六、注意

与 AESGCM 的区别：AESGCM 管「整体封包加密」，DTLS 管「需要 TLS 语义的流」（如 Vivox / EOS Party 语音）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\PacketHandlers\DTLSHandlerComponent\DTLSHandlerComponent.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 3 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
