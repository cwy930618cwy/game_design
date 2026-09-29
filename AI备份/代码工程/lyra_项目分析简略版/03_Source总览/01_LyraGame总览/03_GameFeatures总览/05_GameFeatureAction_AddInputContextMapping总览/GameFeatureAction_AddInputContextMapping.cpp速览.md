# `GameFeatureAction_AddInputContextMapping.cpp` 速览

> 287 行，**一半代码在"注册/注销到用户设置"这件事上**，另一半才是加映射。

| 函数 | 干嘛的 |
|---|---|
| `OnGameFeatureRegistering()` | → `RegisterInputMappingContexts()`（**在 Registering 阶段就做**，比 Activating 更早） |
| `OnGameFeatureUnregistering()` | → `UnregisterInputMappingContexts()` |
| `RegisterInputMappingContexts()` | 挂 `OnStartGameInstance`，对已存在的 GameInstance 补一次 |
| `RegisterInputContextMappingsForGameInstance()` | 挂 LocalPlayer 增删事件，对已有 LocalPlayer 补一次 |
| `RegisterInputMappingContextsForLocalPlayer()` | ⭐ 通过 `ULyraAssetManager::GetAsset` 加载 IMC，再 `Settings->RegisterInputMappingContext(IMC)` |
| `AddToWorld()` | 对 `APlayerController::StaticClass()` 注册 `HandleControllerExtension` |
| `HandleControllerExtension()` | 移除 → 撤；`ExtensionAdded` 或 `NAME_BindInputsNow` → 加 |
| `AddInputMappingForPlayer()` | `InputSystem->AddMappingContext(IMC, Entry.Priority)` |
| `RemoveInputMapping()` | `RemoveMappingContext` |

## 为什么要"注册到 UserSettings"

```
只 AddMappingContext            → 能用，但玩家改不了键
注册到 EnhancedInputUserSettings → 改键界面能看到并能保存
```

> 💡 这是 `bRegisterWithSettings` 这个开关的意义：**勾上才能被改键系统接管**。

> ⚠️ 源码里留了个官方 TODO：`// TODO Why does this code mix and match controllers and local players? ControllersAddedTo is never modified` —— 说明这块实现有点糙，`ControllersAddedTo` 数组实际从没被写进去过。

**优先级**：`RegisterInputMappingContextsForLocalPlayer` → `AddInputMappingForPlayer`
