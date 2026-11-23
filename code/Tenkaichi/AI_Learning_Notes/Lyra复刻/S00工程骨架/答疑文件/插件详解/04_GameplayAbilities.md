# 04 — `GameplayAbilities` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **4** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Gameplay`
> 官方描述：Adds GameplayEffect and GameplayAbility classes to handle complicated gameplay interactions.
> 模块：`GameplayAbilities`(Runtime), `GameplayAbilitiesEditor`(UncookedOnly)

---

## 一、一句话

★ GAS —— Gameplay Ability System，Lyra 整套技能/属性/效果系统的地基。

## 二、它到底做什么

提供 `UGameplayAbility`（技能）、`UGameplayEffect`（效果/瞬时计算）、`UAbilitySystemComponent`（能力系统组件）、`FGameplayTag` 容器、`UGameplayCueInterface`（表现回调）等全部核心类。模块分两个：`GameplayAbilities`（Runtime）+ `GameplayAbilitiesEditor`（UncookedOnly，编辑器里的 GE/GA 可视化编辑）。

## 三、有它 vs 没它

**没有它 Lyra 一行都编不过** —— `LyraGame.Build.cs` 公共依赖第 9 项；`ULyraAbilitySystemComponent`、`ULyraHealthSet`、`ULyraDamageExecution` 全部派生自它。

## 四、Lyra 怎么用它

`.uproject` + `LyraGame.Build.cs`（公共）+ `LyraEditor.Build.cs`（公共）+ ShooterCore / TopDownArena 插件。全 Lyra 引用最广的插件之一。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\GameFeatures\ShooterCore\ShooterCore.uplugin` (x1)
- `Plugins\GameFeatures\ShooterCore\Source\ShooterCoreRuntime\ShooterCoreRuntime.Build.cs` (x1)
- `Plugins\GameFeatures\TopDownArena\TopDownArena.uplugin` (x1)
- `Plugins\GameFeatures\TopDownArena\Source\TopDownArenaRuntime\TopDownArenaRuntime.Build.cs` (x1)
- `Source\LyraEditor\LyraEditor.Build.cs` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S10（GAS 核心，51 个文件）** 全程在用它。

## 六、注意

GAS 是 UE 里最重的子系统，S10~S16 几乎每一轮都在跟它打交道。先把 `LyraAbilitySet` / `ULyraAbilitySystemComponent` 吃透，后面武器/装备/背包全是它的延伸。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\GameplayAbilities\GameplayAbilities.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 4 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
