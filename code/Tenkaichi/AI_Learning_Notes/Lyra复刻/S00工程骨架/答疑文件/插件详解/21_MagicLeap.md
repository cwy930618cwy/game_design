# 21 — `MagicLeap` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **21** 项
> 来源：**本机没有此插件**（引擎与工程 `Plugins\` 均无）
> 当前状态：`"Enabled": false`
> 分类：`NONE`
> 官方描述：NONE
> 模块：（无模块 / 空）

---

## 一、一句话

MagicLeap AR 平台支持。

## 二、它到底做什么

本机 UE 5.4 已不含此插件（Launcher 版未收录；MagicLeap 平台自 UE5.0 起逐步停止维护）。

## 三、有它 vs 没它

**本机没有**，但 `Enabled: false` → 引擎压根不查，**零影响**。

## 四、Lyra 怎么用它

只在 `.uproject` 里写 `Enabled: false`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

别把 `false` 改成 `true` —— 立刻报 `MagicLeap plugin is referenced by target but not found`。

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 21 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
