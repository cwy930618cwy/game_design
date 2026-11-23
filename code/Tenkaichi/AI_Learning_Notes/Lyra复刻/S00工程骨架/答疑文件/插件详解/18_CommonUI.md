# 18 — `CommonUI` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **18** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`UI`
> 官方描述：A repository for game independent UI elements.
> 模块：`CommonUI`(Runtime), `CommonUIEditor`(Editor), `CommonInput`(Runtime)

---

## 一、一句话

★ CommonUI 框架 —— 手柄/键鼠/触屏统一 UI 输入与焦点管理。

## 二、它到底做什么

3 个模块：`CommonUI`（Runtime）、`CommonUIEditor`（Editor）、`CommonInput`（Runtime）。提供 `UCommonActivatableWidget`（可激活控件，带输入栈）、`UCommonUserWidget`、`UCommonButtonBase`、`UCommonUIActionRouter`（输入路由）、`UCommonInputSubsystem`（输入类型切换）。

## 三、有它 vs 没它

**不启用 → Lyra 的整套 UI 体系不存在**（Lyra 的每个 Widget 都派生自 `ULyraActivatableWidget : UCommonActivatableWidget`）。

## 四、Lyra 怎么用它

`.uproject` + `CommonGame` 插件依赖它 + `GameSettings` 插件依赖它 + ShooterCore。Lyra 引用最多的插件（10 处）。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\CommonGame\CommonGame.uplugin` (x1)
- `Plugins\CommonGame\Source\CommonGame.Build.cs` (x1)
- `Plugins\GameFeatures\ShooterCore\ShooterCore.uplugin` (x1)
- `Plugins\GameFeatures\ShooterCore\Source\ShooterCoreRuntime\ShooterCoreRuntime.Build.cs` (x1)
- `Plugins\GameSettings\GameSettings.uplugin` (x1)
- `Plugins\GameSettings\Source\GameSettings.Build.cs` (x1)
- `Plugins\UIExtension\UIExtension.uplugin` (x1)
- `Plugins\UIExtension\Source\UIExtension.Build.cs` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S18（UI，79 个文件）** 全程。

## 六、注意

CommonUI 的核心是「**输入栈**」：打开菜单时自动把游戏输入压栈、Esc 逐层退出、手柄导航焦点环 —— 这些引擎原生 UMG 都没有。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\CommonUI\CommonUI.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 18 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
