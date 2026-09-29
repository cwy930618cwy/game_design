# `LyraLobbyBackground.h` 速览

> **只有一个字段的数据资产**。

| 成员 | 干嘛的 |
|---|---|
| `: UPrimaryDataAsset`（`config=EditorPerProjectUserSettings`） | 注意是**编辑器用户设置**级别的 config |
| `BackgroundLevel` | `TSoftObjectPtr<UWorld>`，大厅背景用哪个关卡 |

**说明**：类注释写着 "Developer settings / editor cheats" —— 这是个编辑器辅助用的资产，不是运行时功能。
