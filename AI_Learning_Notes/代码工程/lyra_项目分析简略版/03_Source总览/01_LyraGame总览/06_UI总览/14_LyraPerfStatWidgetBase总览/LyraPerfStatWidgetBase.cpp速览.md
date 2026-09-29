# `LyraPerfStatWidgetBase.cpp` 速览

> 187 行。**是学习"如何在 UMG 里嵌手写 Slate 控件"的好例子**。

| 函数 | 干嘛的 |
|---|---|
| `SLyraLatencyGraph::Construct()` | 保存构造参数 |
| `SLyraLatencyGraph::OnPaint()` | 画半透明黑背景 → `DrawTotalLatency()` 画折线 |
| `SLyraLatencyGraph::DrawTotalLatency()` | ⭐ 见下 |
| `ULyraPerfStatGraph::RebuildWidget()` | `SAssignNew(SlateLatencyGraph, SLyraLatencyGraph)` |
| `ULyraPerfStatGraph::ReleaseSlateResources()` | ⭐ **必须释放**，否则 Slate 控件泄漏 |
| `ULyraPerfStatGraph` 四个设置函数 | 转发给 Slate 控件 |
| `ULyraPerfStatWidgetBase::FetchStatValue()` | `CachedStatSubsystem->GetCachedStat(StatToDisplay)` |
| `UpdateGraphData()` | 取采样缓存喂给图表 |
| `NativeConstruct()` | 缓存子系统 + 初始化图表颜色 |
| `GetStatSubsystem()` | 从 GameInstance 上懒加载并缓存 |

## `DrawTotalLatency()` 怎么画

```
1. 用 static TArray<FVector2D> Points（避免每帧分配）
2. XSlice = 控件宽度 / 采样数
3. 遍历每个采样：
       Y = 高度 - Clamp(值 × ScaleFactor, 0, MaxY) / MaxY × 高度
       Y 再钳到 [Border, 高度-Border]
       Points.Add(XSlice × i, Y)
4. FSlateDrawElement::MakeLines 画折线
```

> 💡 **两个细节**：
> - 用 `static` 数组复用内存，避免每帧 new（注释里还留了句吐槽 `// Why does this not just draw a straight line??`）
> - `MaxYAxisOfGraph` 默认 **33**（约 30 FPS 对应的毫秒数），所以超过 33ms 的部分会被压平

**优先级**：`DrawTotalLatency` → `RebuildWidget`
