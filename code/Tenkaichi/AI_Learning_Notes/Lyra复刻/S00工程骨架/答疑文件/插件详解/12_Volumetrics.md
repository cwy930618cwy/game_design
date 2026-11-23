# 12 — `Volumetrics` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **12** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`
> 分类：`Rendering`
> 官方描述：A library of volume creation and rendering tools using Blueprints.
> 模块：（无模块 / 空）

---

## 一、一句话

体积云 / 体积雾的蓝图创建与渲染工具库。

## 二、它到底做什么

模块列表为空（纯蓝图/资产型插件，`CanContainContent` 相关）。提供一套蓝图函数与体积域（Volume Domain）资产，用来在关卡里快速搭体积云、雾、光束。

## 三、有它 vs 没它

不启用 → 关卡里不能放 Volumetrics 的体积云 Actor，蓝图里也没有对应节点。**Lyra 的地图如果没有用体积云，运行时零影响。**

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（除非做天空/天气）。

## 六、注意

UE5 的体积云主流程已内置在 `VolumetricCloud` 组件里；这个插件更像「蓝图封装 + 预设」。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Volumetrics\Volumetrics.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 12 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
