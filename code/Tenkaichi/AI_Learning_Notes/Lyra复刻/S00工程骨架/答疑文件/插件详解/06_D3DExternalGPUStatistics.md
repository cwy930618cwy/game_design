# 06 — `D3DExternalGPUStatistics` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **6** 项
> 来源：**本机没有此插件**（引擎与工程 `Plugins\` 均无）
> 当前状态：`"Enabled": true`
> 分类：`NONE`
> 官方描述：NONE
> 模块：（无模块 / 空）

---

## 一、一句话

D3D 外接显卡性能统计（GPU 性能计数器采集）。

## 二、它到底做什么

官方无描述可查（本机 UE 5.4 未收录该插件，属特定平台/特定分发渠道）。从命名与 Lyra 用法推断：用于采集外接 GPU（eGPU）/多 GPU 场景的性能数据。

## 三、有它 vs 没它

**本机没有这个插件**，但 `.uproject` 里写了 `"Optional": true` → UBT 记录一条 LogDebug 后跳过，**不报错**。启用它对游戏完全无影响。

## 四、Lyra 怎么用它

只在 `.uproject` 启用（带 `Optional:true` + `SupportedTargetPlatforms:[Win64]`）。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

如果哪天把 `Optional` 删掉而插件仍缺失 → 立刻报 `D3DExternalGPUStatistics plugin is referenced by target but not found`。**别动这两个字段。**

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 6 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
