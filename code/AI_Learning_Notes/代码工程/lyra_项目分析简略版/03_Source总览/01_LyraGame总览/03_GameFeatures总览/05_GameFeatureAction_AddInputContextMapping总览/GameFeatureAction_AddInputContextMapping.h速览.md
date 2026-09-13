# `GameFeatureAction_AddInputContextMapping.h` 速览

> 把 EnhancedInput 的 `InputMappingContext` 加给**本地玩家**。菜单里显示为 "Add Input Mapping"。
> ⚠️ 别和 `AddInputBinding` 搞混，见文末对比。

| 成员 | 干嘛的 |
|---|---|
| `FInputMappingContextAndPriority` | `InputMapping` + `Priority`（数字大的优先）+ `bRegisterWithSettings` |
| `: UGameFeatureAction_WorldActionBase`（`final`） | 走世界感知流程 |
| ⭐ 4 个生命周期全 override | `Registering` / `Activating` / `Deactivating` / `Unregistering` —— 比其它 Action 多两档 |
| `InputMappings` | 配置数组 |
| `RegisterInputMappingContexts()` | 注册到 **EnhancedInputUserSettings**（改键功能的基础） |
| `RegisterInputContextMappingsForGameInstance()` | 监听 LocalPlayer 增删，并对已有的补一次 |
| `RegisterInputMappingContextsForLocalPlayer()` | 实际调用 `Settings->RegisterInputMappingContext` |
| 三个 `Unregister*` | 反向 |
| `AddToWorld()` | 对 `APlayerController::StaticClass()` 注册回调 |
| `AddInputMappingForPlayer()` | `InputSystem->AddMappingContext(IMC, Priority)` |
| `RemoveInputMapping()` | `RemoveMappingContext` |
| `RegisterInputContextMappingsForGameInstanceHandle` | 记住 `OnStartGameInstance` 的委托句柄 |

## 与 `AddInputBinding` 的区别

| | AddInputContextMapping | AddInputBinding |
|---|---|---|
| 加的东西 | `UInputMappingContext`（EnhancedInput 原生物） | `ULyraInputConfig`（Lyra 自己的封装） |
| 加给谁 | `APlayerController` / LocalPlayer | `APawn` |
| 加给谁管 | EnhancedInput 直接管 | 转交 `LyraHeroComponent` |
| 支持改键 | ✅（注册进 UserSettings） | 由 HeroComponent 决定 |

**优先级**：`RegisterInputMappingContextsForLocalPlayer` → `AddInputMappingForPlayer`
