# 45 — `UIExtension` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **45** 项
> 来源：工程插件（`E:\ue5\game_design\code\Tenkaichi\Plugins\UIExtension\`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`UI`
> 官方描述：A subsystem that allows extending UI elements in a modular way.
> 模块：`UIExtension`(Runtime)

---

## 一、一句话

★ UI 扩展点 —— 让玩法模块往 HUD 里「插」自己的控件。

## 二、它到底做什么

8 个文件。核心：`UUIExtensionSubsystem`（注册/注销扩展点）、`UUIExtensionPointWidget`（在布局里占位，运行时填入扩展内容）、`FUIExtensionRequest`（请求）。

## 三、有它 vs 没它

不启用 → GameFeature 插件没法把自己的 HUD 元素（弹药数/技能条）插进主 HUD。

## 四、Lyra 怎么用它

工程插件（`Plugins\UIExtension\`）+ `LyraGame.Build.cs` 私有依赖 + `CommonGame` 依赖？不，`UIExtension` **依赖** `CommonGame`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\UIExtension\UIExtension.uplugin` (x2)
- `Plugins\UIExtension\Source\Private\Widgets\UIExtensionPointWidget.cpp` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S03（插件③）**。

## 六、注意

这是 Lyra「玩法模块化」的关键：ShooterCore 插件加载后，把准星、弹药、技能条插到 Lyra 主 HUD 的预定义扩展点上。

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Plugins\UIExtension\UIExtension.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 45 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
