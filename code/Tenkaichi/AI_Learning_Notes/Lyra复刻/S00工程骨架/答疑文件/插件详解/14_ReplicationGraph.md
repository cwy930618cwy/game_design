# 14 — `ReplicationGraph` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **14** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Performance`
> 官方描述：The Replication Graph plugin provides a Replication Driver implementation designed for a large number of actors and connections by mainting persistent replicated actor lists in a graph structure.
> 模块：`ReplicationGraph`(Runtime)

---

## 一、一句话

★ 复制图 —— 按空间网格决定「谁同步给谁」，大幅省带宽。

## 二、它到底做什么

提供 `UReplicationGraph` / `UReplicationGraphNode` / `UReplicationDriver`。把世界切成网格单元，每个客户端只同步它附近的 Actor，而不是全图广播。

## 三、有它 vs 没它

不启用 → 退回 UE 默认复制（`UNetDriver` 全量相关性判断），50+ 人场景带宽吃不消。**单机开发完全无感。**

## 四、Lyra 怎么用它

`.uproject` + `LyraGame.Build.cs`（公共）+ `S21` 的 `LyraReplicationGraph.h/.cpp`（41.8KB，Lyra 最大单文件）。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S21（System 进阶）**。

## 六、注意

这是「大规模多人」的必需品，但单机/小规模局域网可以先不管。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\ReplicationGraph\ReplicationGraph.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 14 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
