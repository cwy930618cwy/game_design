# 58 — `MoviePipelineMaskRenderPass` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **58** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Rendering`
> 官方描述：Additional render passes for the Movie Render Queue. This currently includes the ObjectId pass (Editor Only) which generates object mattes with some limitations (using the Cryptomatte specification), and a Panoramic pass with better Sequencer integration than the Panoramic Capture plugin.
> 模块：`MoviePipelineMaskRenderPass`(UncookedOnly)

---

## 一、一句话

影片渲染的遮罩通道（ObjectId / 自定义模板）。

## 二、它到底做什么

模块：`MoviePipelineMaskRenderPass`（UncookedOnly）。给 Movie Render Queue 增加「按物体 ID 输出遮罩」的能力（后期合成用）。

## 三、有它 vs 没它

不启用 → MRQ 里少这几个 Pass。**运行时零影响。**

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

依赖 57 号。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\MovieScene\MoviePipelineMaskRenderPass\MoviePipelineMaskRenderPass.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 58 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
