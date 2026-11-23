# 16 — `Niagara` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **16** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `True`
> 分类：`FX`
> 官方描述：Niagara effect systems.
> 模块：`NiagaraCore`(Runtime), `Niagara`(Runtime), `NiagaraAnimNotifies`(Runtime), `NiagaraShader`(Runtime), `NiagaraVertexFactories`(Runtime), `NiagaraBlueprintNodes`(UncookedOnly), `NiagaraEditor`(Editor), `NiagaraEditorWidgets`(Editor)

---

## 一、一句话

★ Niagara 粒子系统（UE5 的标准 VFX 方案）。

## 二、它到底做什么

8 个模块：`NiagaraCore` / `Niagara` / `NiagaraAnimNotifies` / `NiagaraShader` / `NiagaraVertexFactories` / `NiagaraBlueprintNodes` / `NiagaraEditor` / `NiagaraEditorWidgets`。GPU/CPU 粒子、Niagara 系统与发射器、Niagara Data Channel 等。`EnabledByDefault: true`。

## 三、有它 vs 没它

不启用 → 所有 `.NiagaraSystem` 资产消失、`UNiagaraComponent` 不存在。Lyra 的枪口火焰、命中特效、环境粒子全部挂掉。

## 四、Lyra 怎么用它

`.uproject` + `LyraGame.Build.cs`（公共）+ `S17` 的 `LyraDamagePopStyleNiagara` / `LyraNumberPopComponent_NiagaraText` + TopDownArena。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\GameFeatures\TopDownArena\TopDownArena.uplugin` (x1)
- `Plugins\GameFeatures\TopDownArena\Source\TopDownArenaRuntime\TopDownArenaRuntime.Build.cs` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S17（表现层）**。

## 六、注意

Niagara 的资产是二进制 `.uasset`，代码复刻只能复刻「怎么调用它」（如 `UNiagaraComponent`、`UNiagaraFunctionLibrary`），不能复刻粒子本身。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\FX\Niagara\Niagara.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 16 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
