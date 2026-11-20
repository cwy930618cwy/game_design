# `LyraBotCreationComponent.h` 速览

> 离线练习用的机器人生成器。它是个**抽象类**（`Abstract`），实际用的版本是蓝图子类。

| 成员 | 干嘛的 |
|---|---|
| `: UGameStateComponent`, `Blueprintable`, `Abstract` | 挂 GameState 上，期待被蓝图继承 |
| `BeginPlay()` | 注册 Experience 加载完成回调 |
| `OnExperienceLoaded()`（private） | 有权限时调 `ServerCreateBots()` |
| `NumBotsToCreate` | 生成几个（默认 5） |
| `BotControllerClass` | 用哪个 AIController 类 |
| `RandomBotNames` | 机器人名字池 |
| `RemainingBotNames` | ⭐ 还没被取用过的名字（抽签用） |
| `SpawnedBotList` | 已生成的机器人，`Transient` |
| `SpawnOneBot()` | 生成一个，蓝图可调 + `BlueprintAuthorityOnly` |
| `RemoveOneBot()` | 删一个，同上 |
| `ServerCreateBots()` | `BlueprintNativeEvent` —— 可以在蓝图里覆写 |
| `Cheat_AddBot` / `Cheat_RemoveBot` | 作弊命令（仅 `WITH_SERVER_CODE`） |
| `CreateBotName()` | 取随机名字，用完就没有 |
