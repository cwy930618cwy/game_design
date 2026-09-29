# `GameModes/` 目录速览

> 10 个类，20 个文件，**没有子目录**。
> 这里是 **Experience（体验）机制**的全部实现 —— Lyra 用"数据描述一局游戏"取代了传统 GameMode 子类化。

## 分成四组看

```
【数据配置】纯资产，不改代码就能出新模式
  LyraExperienceDefinition              一局游戏的完整配置
  LyraExperienceActionSet               可复用的 Action 包
  LyraUserFacingExperienceDefinition    菜单里显示的那一项

【执行者】把配置变成现实
  LyraExperienceManagerComponent   ★★★ 加载状态机（灵魂）
  LyraGameMode                     ★★  决定用哪份 Experience、怎么生成 Pawn
  LyraGameState                    ★   承载上面那个组件 + 全局 ASC
  LyraBotCreationComponent              离线练习的机器人

【入口】
  AsyncAction_ExperienceReady           蓝图异步节点：等 Experience 就绪
  LyraWorldSettings                     让每张地图自带默认 Experience

【全局协调】
  LyraExperienceManager                 仅编辑器：多 PIE 间插件引用计数
```

## 10 个类一句话

| 类 | 干嘛的 |
|---|---|
| `LyraExperienceManagerComponent` | ⭐⭐⭐ 挂 GameState 上，把一份 Experience 逐步加载成真正跑起来的游戏 |
| `LyraGameMode` | ⭐⭐ 挑选 Experience（7 层优先级）+ 生成 Pawn + 把出生/重生转交组件 |
| `LyraExperienceDefinition` | ⭐ 数据资产：启用哪些插件、用哪套角色、执行哪些 Action |
| `LyraGameState` | 承载 ExperienceManagerComponent 和一个全局 ASC |
| `LyraExperienceActionSet` | 可跨 Experience 复用的 Action 包（`NotBlueprintable`） |
| `LyraUserFacingExperienceDefinition` | 菜单卡片：地图 + 玩法 + 标题图标 + 怎么开服 |
| `LyraWorldSettings` | 每地图的默认 Experience；编辑器下还能强制 Standalone |
| `AsyncAction_ExperienceReady` | 蓝图用的四步异步节点 |
| `LyraBotCreationComponent` | 离线机器人生成（抽象类，实际用蓝图子类） |
| `LyraExperienceManager` | 只管插件引用计数，**不是**加载管理器（别被名字骗了） |

## Experience 加载流水线

```
LyraGameMode::HandleMatchAssignmentIfNotExpectingOne()   挑出用哪份 Experience
        ↓ SetCurrentExperience(ExperienceId)
LyraExperienceManagerComponent
        ├─ StartExperienceLoad()             加载资源 Bundle（区分 Client/Server）
        ├─ OnExperienceLoadComplete()        找齐 GameFeature 插件并激活
        └─ OnExperienceFullLoadCompleted()   执行所有 Action → Loaded
                ↓ 按 高 / 中 / 低 三档广播
        其它系统开始初始化、机器人最后生成
```

## 补充说明

| 点 | 说明 |
|---|---|
| 加载状态机 | `ELyraExperienceLoadState`：`Unloaded → Loading → LoadingGameFeatures → LoadingChaosTestingDelay → ExecutingActions → Loaded → Deactivating` |
| 三档回调 | `CallOrRegister_OnExperienceLoaded` 的 HighPriority / 普通 / LowPriority，用来**控制初始化顺序** |
| 加载界面为什么不消失 | `ShouldShowLoadingScreen()` 在没到 Loaded 时一直返回 true |
| 两个 ID 的区别 | `UserFacingExperience`（地图+界面）通过 URL 参数 `?Experience=` 把 `ExperienceDefinition`（玩法）传进游戏内 |
| 官方自曝的 TODO | `LyraExperienceManagerComponent.cpp` 顶部 7 条：加载失败直接 check、GameFeature 卸载会泄漏等 |

**优先级**：`LyraExperienceManagerComponent` → `LyraGameMode` → `LyraExperienceDefinition` → `LyraGameState`
