# 32 — `PlayFabParty` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **32** 项
> 来源：**本机没有此插件**（引擎与工程 `Plugins\` 均无）
> 当前状态：`"Enabled": true`
> 分类：`NONE`
> 官方描述：NONE
> 模块：（无模块 / 空）

---

## 一、一句话

PlayFab Party 语音/网络（微软 PlayFab 跨平台组队语音）。

## 二、它到底做什么

本机 UE 5.4 未收录。`PlatformAllowList: [XB1, XSX]` + `SupportedTargetPlatforms: [XB1, XSX]`。

## 三、有它 vs 没它

**本机没有**，但①平台白名单只有主机 ②Win64 下引擎判定「不支持本平台」直接跳过 → **不报错、零影响**。

## 四、Lyra 怎么用它

`.uproject` 里启用（带平台白名单）。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（除非做 Xbox 主机语音）。

## 六、注意

注意它跟 6 号（`D3DExternalGPUStatistics`）的豁免机制**不同**：6 号靠 `Optional:true`，32 号靠平台白名单。

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 32 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
