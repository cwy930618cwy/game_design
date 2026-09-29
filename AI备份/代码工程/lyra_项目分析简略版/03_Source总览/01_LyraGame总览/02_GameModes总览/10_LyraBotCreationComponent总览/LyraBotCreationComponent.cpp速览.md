# `LyraBotCreationComponent.cpp` 速览

> 183 行，主体包在 `#if WITH_SERVER_CODE` 里；文件末尾是非服务器版本的三个空壳。

| 函数 | 干嘛的 |
|---|---|
| `BeginPlay()` | 注册 **`_LowPriority`** 档的 Experience 加载回调 —— 机器人最后才生成 |
| `OnExperienceLoaded()` | 有 authority 才调 `ServerCreateBots()` |
| `ServerCreateBots_Implementation()` | 决定数量后循环 `SpawnOneBot()` |
| `CreateBotName()` | ⭐ 从 `RemainingBotNames` 里随机抽一个并移除；**抽完之后退化成 `Tinplate <随机数>`** |
| `SpawnOneBot()` | 生成 AIController → 设名字 → ⭐ **`GameMode->GenericPlayerInitialization` + `RestartPlayer`** → 最后手补一次 `CheckDefaultInitialization()` |
| `RemoveOneBot()` | 随机挑一个；有 HealthComponent 就 `DamageSelfDestruct()`，没有才直接 `Destroy()` |

## 机器人数量由谁决定（优先级高 → 低）

```
URL 参数 ?NumBots=
Developer Settings 里的 OverrideNumPlayerBotsToSpawn（需勾 bOverrideBotCount，且仅在编辑器）
NumBotsToCreate（默认 5）
```

## 非服务器版本

```
ServerCreateBots / SpawnOneBot / RemoveOneBot
  → ensureMsgf(0, "Bot functions do not exist in LyraClient!")
```

> 💡 **`LyraClient` 目标里机器人的存在感都被砍掉了** —— 这是个明确的瘦身策略：客户端包不需要 AI 相关逻辑。

**优先级**：`ServerCreateBots_Implementation` → `SpawnOneBot`
