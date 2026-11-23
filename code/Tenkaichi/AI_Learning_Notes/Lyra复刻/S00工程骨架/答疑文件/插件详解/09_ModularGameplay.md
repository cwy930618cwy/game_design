# 09 — `ModularGameplay` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **9** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Gameplay`
> 官方描述：Base classes and subsystems to support modular use of the gameplay framework
> 模块：`ModularGameplay`(Runtime)

---

## 一、一句话

★ 模块化 Gameplay 基础：组件式挂载与初始化状态机。

## 二、它到底做什么

提供 `UGameFrameworkComponentManager`（让 Actor 在运行时动态挂组件）、`IGameFrameworkInitStateInterface`（初始化状态机，如 `GameplayActor` → `DataAvailable` → `DataInitialized`）、`UGameStateComponent` / `UPawnComponent` 等组件基类。

## 三、有它 vs 没它

**Lyra 的「一切皆组件」架构就靠它** —— `ULyraPawnExtensionComponent`、`ULyraHeroComponent`、`ULyraExperienceManagerComponent` 全部基于它的初始化状态机。缺了编不过。

## 四、Lyra 怎么用它

`.uproject` + `LyraGame.Build.cs`（公共）+ `ModularGameplayActors` 插件依赖它 + ShooterCore / ShooterTests。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\GameFeatures\ShooterCore\ShooterCore.uplugin` (x1)
- `Plugins\GameFeatures\ShooterCore\Source\ShooterCoreRuntime\ShooterCoreRuntime.Build.cs` (x1)
- `Plugins\GameFeatures\ShooterTests\Source\ShooterTestsRuntime\ShooterTestsRuntime.Build.cs` (x1)
- `Plugins\ModularGameplayActors\ModularGameplayActors.uplugin` (x1)
- `Plugins\ModularGameplayActors\Source\ModularGameplayActors\ModularGameplayActors.Build.cs` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S01（ModularGameplayActors 插件依赖它）→ S08 / S11 / S12** 全程。

## 六、注意

S01 会先建 `ModularGameplayActors`（15 个文件），它的 `.uplugin` 里依赖 `ModularGameplay` —— 所以 S00 阶段 `Build.cs` 一写就编不过，要等 S01。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\ModularGameplay\ModularGameplay.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 9 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
