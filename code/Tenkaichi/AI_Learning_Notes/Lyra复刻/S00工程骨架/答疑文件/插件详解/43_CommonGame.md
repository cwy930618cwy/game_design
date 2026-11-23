# 43 — `CommonGame` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **43** 项
> 来源：工程插件（`E:\ue5\game_design\code\Tenkaichi\Plugins\CommonGame\`）
> 当前状态：`"Enabled": true`
> 分类：`Gameplay`
> 官方描述：Generic gameplay classes that use the other Common plugins.
> 模块：`CommonGame`(Runtime)

---

## 一、一句话

★ 通用游戏层 —— `UCommonGameInstance` / UI 策略 / UI 管理层栈。

## 二、它到底做什么

29 个文件。核心：`UCommonGameInstance`、`UCommonLocalPlayer`、`ACommonPlayerController`、`UGameUIManagerSubsystem`（UI 层管理）、`UGameUIPolicy`（UI 策略：HUD/菜单/模态层）、`UPrimaryGameLayout`（主布局控件）、`UCommonUIExtensions`、3 个 `UAsyncAction_*`。

## 三、有它 vs 没它

**不启用 → Lyra 的 `ULyraGameInstance : UCommonGameInstance` 编不过**；UI 层栈体系不存在。

## 四、Lyra 怎么用它

工程插件（`Plugins\CommonGame\`）+ `LyraGame.Build.cs` 私有依赖。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\CommonGame\CommonGame.uplugin` (x2)
- `Plugins\CommonGame\Source\Private\CommonGameInstance.cpp` (x1)
- `Plugins\GameFeatures\ShooterCore\ShooterCore.uplugin` (x1)
- `Plugins\GameFeatures\ShooterCore\Source\ShooterCoreRuntime\ShooterCoreRuntime.Build.cs` (x1)
- `Plugins\UIExtension\UIExtension.uplugin` (x1)
- `Plugins\UIExtension\Source\UIExtension.Build.cs` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S03（插件③）**。

## 六、注意

它是 `UIExtension` 的依赖（45 号），也是 `LyraGameInstance` 的基类来源。

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Plugins\CommonGame\CommonGame.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 43 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
