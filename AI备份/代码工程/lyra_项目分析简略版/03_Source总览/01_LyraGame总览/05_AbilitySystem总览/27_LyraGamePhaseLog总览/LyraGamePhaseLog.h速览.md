# `LyraGamePhaseLog.h` 速览

> **8 行，只有一行有效代码**。只有 `.h`，没有 `.cpp`。

| 内容 | 干嘛的 |
|---|---|
| `DECLARE_LOG_CATEGORY_EXTERN(LogLyraGamePhase, Log, All)` | 声明一个日志分类 |

**说明**：`LyraGamePhaseSubsystem` 用它打阶段切换日志（"Beginning Phase X / Ending Phase Y"）。
`.cpp` 里对应的是 `DEFINE_LOG_CATEGORY(LogLyraGamePhase)`（写在 `LyraGamePhaseSubsystem.cpp` 中，不在这个文件里）。
