# 08 — `GameFeatures` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **8** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Gameplay`
> 官方描述：Support for modular Game Feature Plugins
> 模块：`GameFeatures`(Runtime), `GameFeaturesEditor`(Editor)

---

## 一、一句话

★ GameFeature 机制 —— 把玩法做成可插拔功能包，运行时按需加载/卸载。

## 二、它到底做什么

提供 `UGameFeatureData`（功能包数据）、`UGameFeaturesSubsystem`（加载器）、`UGameFeatureAction`（功能包里可执行的动作基类）。Lyra 的 Experience 系统靠它在加载地图时把「玩法插件」挂上来。模块：`GameFeatures`（Runtime）+ `GameFeaturesEditor`（Editor）。

## 三、有它 vs 没它

**没有它 Lyra 的 Experience 系统完全不成立** —— S08 的 `ULyraExperienceManagerComponent` 就是靠它加载 GameFeature 插件。`LyraGame.Target.cs` 里那段 `ConfigureGameFeaturePlugins` 也是为它服务的。

## 四、Lyra 怎么用它

`.uproject` + `LyraGame.Target.cs` + `LyraGame.Build.cs`（公共）+ S20 的 18 个 `GameFeatureAction_*`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Source\LyraGame.Target.cs` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)
- `Source\LyraGame\GameFeatures\GameFeatureAction_AddAbilities.cpp` (x1)
- `Source\LyraGame\GameFeatures\GameFeatureAction_AddGameplayCuePath.cpp` (x1)
- `Source\LyraGame\GameFeatures\GameFeatureAction_AddInputBinding.cpp` (x1)
- `Source\LyraGame\GameFeatures\GameFeatureAction_AddInputContextMapping.cpp` (x1)

## 五、本复刻线什么时候碰它

**S20（GameFeatures 动作，18 个文件）+ S08（Experience）**。

## 六、注意

它启动时会检查 `DefaultEngine.ini` 的 `AssetManagerSettings` 里有没有 `GameFeatureData` 的扫描规则 —— 缺了就报 `Asset manager settings do not include a rule for assets of type GameFeatureData`（我们已修，见 `00B`）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\GameFeatures\GameFeatures.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 8 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
