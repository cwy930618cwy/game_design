# 13 — `DataRegistry` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **13** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Gameplay`
> 官方描述：Adds Data Registry system that can be used as a generic interface for acquiring structure data from multiple sources at runtime
> 模块：`DataRegistry`(Runtime), `DataRegistryEditor`(UncookedOnly)

---

## 一、一句话

数据注册表 —— 按 ID 从多个数据源统一取结构化数据。

## 二、它到底做什么

提供 `UDataRegistry` / `UDataRegistrySubsystem` / `FDataRegistryId` / `FDataRegistryType`。可以挂 DataTable、CurveTable、GameplayTag 表等多种数据源，用统一的 `(Type, Name)` 键去查。模块：`DataRegistry`（Runtime）+ `DataRegistryEditor`（UncookedOnly）。

## 三、有它 vs 没它

不启用 → Lyra 的 `ULyraAssetManager`（S07）没法按 ID 查 GameplayCue 配置、没法做「数据驱动的资产加载」。

## 四、Lyra 怎么用它

`.uproject` + `LyraGame.Build.cs`（公共）+ ShooterCore。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\GameFeatures\ShooterCore\Source\ShooterCoreRuntime\ShooterCoreRuntime.Build.cs` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S07（AssetManager）+ S10（GameplayCue 管理）**。

## 六、注意

Lyra 用它把 `GameplayCueNotify` 的路径表做成可热更的注册表 —— 这是 Lyra 支持「热修复」的关键一环。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\DataRegistry\DataRegistry.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 13 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
