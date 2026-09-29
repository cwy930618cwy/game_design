# `LyraPerfStatWidgetBase.h` 速览

> 单个性能数值的显示控件。**一个头文件里放了 3 个类**，其中一个还是手写 Slate 的。

| 类 | 干嘛的 |
|---|---|
| `SLyraLatencyGraph` | ⭐ **手写 Slate** 的折线图控件（`SLeafWidget`），自己 `OnPaint` 画线 |
| `ULyraPerfStatGraph` | UMG 包装：把上面那个 Slate 控件包成 UWidget |
| `ULyraPerfStatWidgetBase` | ⭐ 本文件的主体：显示一个 stat（FPS / Ping 等） |

## `SLyraLatencyGraph`（Slate 折线图）

| 成员 | 干嘛的 |
|---|---|
| `SLATE_BEGIN_ARGS` 里的默认值 | `DesiredSize(150,50)`、`MaxLatencyToGraph(33)`、`LineColor` 白、`BackgroundColor` 半透明黑 |
| `OnPaint()` | 画背景 → 画折线 |
| `ComputeVolatility()` | 返回 `true`（每帧可能变，需要重绘） |
| `DrawTotalLatency()` | 真正的画线逻辑 |
| `SetLineColor` / `SetMaxYValue` / `SetBackgroundColor` / `UpdateGraphData` | 四个设置接口 |

## `ULyraPerfStatWidgetBase`

| 成员 | 干嘛的 |
|---|---|
| `: UCommonUserWidget`（`Abstract`） | 标准控件 |
| `GetStatToDisplay()` | 我要显示哪个 stat |
| `FetchStatValue()` | ⭐ `BlueprintPure`，取当前值（给蓝图绑定） |
| `UpdateGraphData()` | 把采样数据喂给图表 |
| `GetStatSubsystem()` | 懒加载并缓存 `ULyraPerformanceStatSubsystem` |
| `PerfStatGraph` | `BindWidget, OptionalWidget` 的图表 |
| `GraphLineColor` / `GraphBackgroundColor` / `GraphMaxYValue` | 图表外观 |
| `StatToDisplay` | 要显示哪个 stat（`ELyraDisplayablePerformanceStat`） |

**优先级**：`FetchStatValue` → `SLyraLatencyGraph::OnPaint`
