# `GameFeatures/` 目录速览

> 8 个类，16 个文件，**没有子目录**。这里定义的是「玩法该怎么挂进游戏」的**手段**。

## 继承关系

```
UGameFeatureAction
├── GameFeatureAction_AddGameplayCuePath     ← 不需要世界，直接继承
└── GameFeatureAction_WorldActionBase        ← 「要给某个世界加东西」的基类
    ├── GameFeatureAction_AddAbilities
    ├── GameFeatureAction_AddWidgets
    ├── GameFeatureAction_AddInputBinding
    ├── GameFeatureAction_AddInputContextMapping
    └── GameFeatureAction_SplitscreenConfig

ULyraGameFeaturePolicy（不是 Action，是策略类）
```

## 8 个类

| 类 | 干嘛的 |
|---|---|
| `GameFeatureAction_WorldActionBase` | ⭐ 所有"要往世界里加东西"的 Action 的基类，子类实现纯虚 `AddToWorld()` |
| `GameFeatureAction_AddAbilities` | 给**指定类的 Actor** 授予技能 / 属性集 / `ULyraAbilitySet` |
| `GameFeatureAction_AddWidgets` | 给 HUD 加 Layout 和小部件（按 GameplayTag 找 UIExtension 插槽） |
| `GameFeatureAction_AddInputBinding` | 给 Pawn 挂 `ULyraInputConfig`（Lyra 自己的输入配置） |
| `GameFeatureAction_AddInputContextMapping` | 给本地玩家加 EnhancedInput 的 `InputMappingContext`，可带优先级 |
| `GameFeatureAction_AddGameplayCuePath` | 注册 GameplayCue 的搜索目录（默认 `/GameplayCues`） |
| `GameFeatureAction_SplitscreenConfig` | 用**投票制**开关分屏（多个 Action 投同一个 Viewport 才算生效） |
| `LyraGameFeaturePolicy` | 决定 GameFeature 怎么加载；还挂了两个状态观察者（热更新、Cue 路径） |

## 补充说明

| 点 | 说明 |
|---|---|
| `WorldActionBase` 的巧妙处 | 它在 `OnStartGameInstance` 上挂了回调，**世界的 GameInstance 一启动就把 Action 应用上去**；已经存在的世界则当场补一遍 |
| 为什么 `AddGameplayCuePath` 不继承它 | 它只改 GameplayCueManager 的目录列表，跟具体世界无关 |
| 真正"应用"这些 Action 的地方 | `LyraExperienceManagerComponent::OnExperienceFullLoadCompleted` 里逐个调 `Registering` / `Loading` / `Activating` |
| `LyraGameFeaturePolicy` 的加载模式 | `bLoadClientData = !IsRunningDedicatedServer()`，`bLoadServerData = !IsRunningClientOnly()` |

**优先级**：`GameFeatureAction_WorldActionBase` → `AddAbilities` → `AddWidgets` → `AddInputContextMapping`
