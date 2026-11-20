# `LyraGameSettingRegistry_PerfStats.cpp` 速览

> 352 行。**性能统计页**（和 `UI/PerformanceStats` 是一对：这里管"显示哪些"，那边管"怎么画"）。

| 内容 | 干嘛的 |
|---|---|
| CVar `Lyra.Settings.LatencyMarkersRequireNVIDIA` | 延迟标记只在 N 卡上启用（默认 true） |
| 3 个自定义 EditCondition 类 | `..._LatencyStatsSupported`、`..._LatencyStatsCurrentlyEnabled`、`..._LatencyMarkersSupported` |
| `AddPerformanceStatPage()` | ⭐ 见下 |

## 三个分组，共 18 个 stat

| 分组 | 包含 |
|---|---|
| **Performance** | 客户端 FPS、服务器 FPS、帧时间、空闲时间、游戏线程、渲染线程、RHI 线程、GPU |
| **Network** | Ping、进出丢包率、进出包速率、进出包大小 |
| **Latency** | 延迟统计开关、延迟标记开关、总延迟、游戏延迟、渲染延迟 |

## 关键设计：三层启用条件

```
① LatencyStatsSupported      平台支不支持延迟统计
② LatencyStatsCurrentlyEnabled  延迟统计当前开没开（会监听设置变化自动刷新）
③ LatencyMarkersSupported    平台支不支持延迟标记 + 是不是 N 卡
```

延迟标记必须**同时满足 ①②③**，延迟类的 stat 需要满足 ①②。

> 💡 **`FGameSettingEditCondition_LatencyStatsCurrentlyEnabled` 的做法值得学**：它在 `Initialize` 时订阅设置变更事件，变化时 `BroadcastEditConditionChanged` —— 这样"开了延迟统计"之后，下面灰着的选项会**立刻自动亮起来**。
>
> ⚠️ 顶部有 `static_assert((int32)ELyraDisplayablePerformanceStat::Count == 18, ...)` —— **加新 stat 时必须回来改这个数字**，否则编译不过。

**优先级**：`AddPerformanceStatPage` → 三个 EditCondition 类
