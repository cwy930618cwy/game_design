# `LyraUserFacingExperienceDefinition.cpp` 速览

> 54 行，只有 `CreateHostingRequest()` 一个函数。**这是"菜单点开始"到"真正开服"的唯一桥梁**。

| 步骤 | 干嘛的 |
|---|---|
| 优先用会话子系统创建 | `UCommonSessionSubsystem::CreateOnlineHostSessionRequest()` |
| 拿不到就手工造一个 | `OnlineMode = Online`、`bUseLobbies = true`、`bUsePresence = !IsRunningDedicatedServer()` |
| `Result->MapID = MapID` | 要加载的地图 |
| `Result->ModeNameForAdvertisement` | 用自己这个资产名（给匹配服务器看） |
| `Result->ExtraArgs = ExtraArgs` | 先拷用户配的额外参数 |
| ⭐ `Result->ExtraArgs.Add("Experience", ExperienceName)` | **关键一步**：把要用的 Experience 名塞进 URL 参数，游戏内靠它还原出 ExperienceId |
| `Result->MaxPlayerCount` | 人数上限 |
| `bRecordReplay` 且平台支持 | 追加 `DemoRec` 参数开启录像 |

**优先级**：只有一个函数，重点看那行 `ExtraArgs.Add(TEXT("Experience"), ...)`
