# `LyraUserFacingExperienceDefinition.h` 速览

> "菜单里看到的那个条目"。**它不描述玩法，只描述界面展示 + 怎么开这场**。

| 成员 | 干嘛的 |
|---|---|
| `MapID` | 要加载哪张地图（`AllowedTypes="Map"`） |
| `ExperienceID` | 要加载哪份 `LyraExperienceDefinition`（玩法本体） |
| `ExtraArgs` | 作为 URL 选项传给游戏的额外参数 |
| `TileTitle` / `TileSubTitle` / `TileDescription` / `TileIcon` | 菜单卡片上的标题、副标题、描述、图标 |
| `LoadingScreenWidget` | 进入/退出这个模式时显示哪个加载界面 |
| `bIsDefaultExperience` | 是不是默认项（快速开始用，UI 里排前面） |
| `bShowInFrontEnd` | 要不要显示在前端列表里 |
| `bRecordReplay` | 要不要录像 |
| `MaxPlayerCount` | 这个会话最多多少人（默认 16） |
| `CreateHostingRequest()` | ⭐ 生成一个 `UCommonSession_HostSessionRequest`，用于真正开服 |

> 💡 **两个 ID 的分工是关键**：`UserFacingExperience`（界面 + 地图）和 `ExperienceDefinition`（玩法 + 角色 + Action）是**两个不同的资产**，前者通过 URL 参数把后者的名字传进游戏内 —— 这条线索见 `LyraGameMode::HandleMatchAssignmentIfNotExpectingOne`。

**优先级**：`CreateHostingRequest`
