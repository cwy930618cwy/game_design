# 26 — `OpenXRHMD` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **26** 项
> 来源：**本机没有此插件**（引擎与工程 `Plugins\` 均无）
> 当前状态：`"Enabled": false`
> 分类：`NONE`
> 官方描述：NONE
> 模块：（无模块 / 空）

---

## 一、一句话

OpenXR 头显（HMD）支持。

## 二、它到底做什么

UE5.4 已把 `OpenXRHMD` 合并进 `OpenXR` 主插件，独立插件不再分发。

## 三、有它 vs 没它

**本机没有** + `Enabled: false` → 零影响。

## 四、Lyra 怎么用它

`.uproject` 里 `Enabled: false`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

别改成 `true`。

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 26 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
