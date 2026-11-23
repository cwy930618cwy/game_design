# 17 — `Water` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **17** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Water`
> 官方描述：Full suite of water tools and rendering techniques to easily add oceans, river, lakes or custom water bodies that carve landscape and interacts with gameplay
> 模块：`Water`(Runtime), `WaterEditor`(Editor)

---

## 一、一句话

★ 水体系统：海洋 / 河流 / 湖泊 / 自定义水体，能刻地形、能和玩法交互。

## 二、它到底做什么

`Water`（Runtime，`LoadingPhase=PostConfigInit`）+ `WaterEditor`（Editor）。提供 `AWaterBodyOcean` / `AWaterBodyRiver` / `AWaterBodyLake` / `AWaterBodyCustom`、`UWaterBodyComponent`（波浪、材质、碰撞）、`UWaterSubsystem`。

## 三、有它 vs 没它

不启用 → 关卡里没有水体 Actor。**但它启动时会硬性检查碰撞配置**：`DefaultEngine.ini` 里没有 `WaterBodyCollision` 就报 `碰撞描述文件设置不包括水体碰撞描述文件的条目`（我们踩过，见 `00B` 第 2 步）。

## 四、Lyra 怎么用它

只在 `.uproject` 启用（Lyra 地图里没用水体，但保留了一比一）。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（除非做水体地图）。

## 六、注意

★ **必须配 `DefaultEngine.ini`**（不是 `DefaultEditor.ini`！）：`[CollisionProfile]` 加 `WaterBodyCollision`。类头是 `UCLASS(config=Engine)`，读取写死 `GEngineIni`。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Water\Water.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 17 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
