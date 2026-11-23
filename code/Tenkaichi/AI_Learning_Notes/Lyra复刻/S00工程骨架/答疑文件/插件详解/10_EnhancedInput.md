# 10 — `EnhancedInput` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **10** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `True`
> 分类：`Input`
> 官方描述：Input handling that allows for contextual and dynamic mappings.
> 模块：`EnhancedInput`(Runtime), `InputBlueprintNodes`(UncookedOnly), `InputEditor`(Editor)

---

## 一、一句话

★ 增强输入系统：上下文式、可动态切换的输入映射。

## 二、它到底做什么

提供 `UInputAction`（动作）、`UInputMappingContext`（映射上下文）、`UInputModifier`（修饰器：死区/反向/缩放）、`UInputTrigger`（触发器：按下/长按/连发）。模块：`EnhancedInput`（Runtime）+ `InputBlueprintNodes`（UncookedOnly）+ `InputEditor`（Editor）。`EnabledByDefault: true`。

## 三、有它 vs 没它

不启用 → 退回 UE4 的旧输入系统（`UInputComponent` + `BindAxis`），Lyra 的输入系统完全不成立。

## 四、Lyra 怎么用它

`.uproject` + `LyraGame.Build.cs` 私有依赖 + `DefaultInput.ini` 的 `DefaultPlayerInputClass=EnhancedPlayerInput` / `DefaultInputComponentClass=EnhancedInputComponent`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\GameFeatures\ShooterCore\ShooterCore.uplugin` (x1)
- `Plugins\GameFeatures\ShooterCore\Source\ShooterCoreRuntime\ShooterCoreRuntime.Build.cs` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S13（Input 输入系统，14 个文件）**。

## 六、注意

S13 会把 `DefaultInputComponentClass` 从 `EnhancedInputComponent` 改成 Lyra 自己的 `LyraInputComponent`（带「按住/松开」绑定）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\EnhancedInput\EnhancedInput.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 10 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
