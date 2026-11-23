# 57 — `MovieRenderPipeline` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **57** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Rendering`
> 官方描述：Advanced movie rendering pipeline for use in creating rendered cinematics or other multi-media creation.
> 模块：`MovieRenderPipelineCore`(Runtime), `MovieRenderPipelineSettings`(Runtime), `MovieRenderPipelineRenderPasses`(Runtime), `MovieRenderPipelineEditor`(Editor), `UEOpenExrRTTI`(Runtime)

---

## 一、一句话

影片渲染管线 —— 离线出高质量视频（Movie Render Queue）。

## 二、它到底做什么

模块：`MovieRenderPipelineCore` / `MovieRenderPipelineSettings` / `MovieRenderPipelineEditor` 等。提供「按帧精确渲染」「多通道输出」「抗锯齿累积采样」。

## 三、有它 vs 没它

不启用 → 编辑器里没有 Movie Render Queue（做宣传片/过场动画时用）。**运行时零影响。**

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

做预告片才用；不做出片可以直接关。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\MovieScene\MovieRenderPipeline\MovieRenderPipeline.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 57 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
