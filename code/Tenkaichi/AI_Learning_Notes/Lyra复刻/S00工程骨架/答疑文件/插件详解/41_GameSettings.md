# 41 — `GameSettings` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **41** 项
> 来源：工程插件（`E:\ue5\game_design\code\Tenkaichi\Plugins\GameSettings\`）
> 当前状态：`"Enabled": true`
> 分类：`UI`
> 官方描述：System for defining game-specific settings and exposing them to the UI.
> 模块：`GameSettings`(Runtime)

---

## 一、一句话

★ 游戏设置框架 —— 定义设置项并暴露给 UI。

## 二、它到底做什么

60 个文件。核心：`UGameSetting`（设置基类）、`UGameSettingValue`（标量/离散值）、`UGameSettingRegistry`（注册表）、`UGameSettingCollection`（分组）、`UGameSettingScreen` / `UGameSettingPanel`（界面）、`UGameSettingDataSource`（数据源）。

## 三、有它 vs 没它

不启用 → 设置界面无从搭起。Lyra 的画质/音量/按键/语言设置全靠它。

## 四、Lyra 怎么用它

工程插件（`Plugins\GameSettings\`）+ `LyraGame.Build.cs` 私有依赖。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\GameSettings\GameSettings.uplugin` (x2)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S04（插件④，分 4 批教）+ S19（Settings，34 个文件）**。

## 六、注意

S04 批 1 是核心基类（12 个文件），批 2 是集合/注册表（14 个），批 3 是 Widgets（20 个），批 4 是响应式面板（8 个）。

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Plugins\GameSettings\GameSettings.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 41 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
