# `GameFeatureAction_AddInputBinding.h` 速览

> 给 **Pawn** 挂上 Lyra 自己的输入配置 `ULyraInputConfig`。菜单里显示为 "Add Input Binds"。

| 成员 | 干嘛的 |
|---|---|
| `: UGameFeatureAction_WorldActionBase`（`final`） | 走世界感知流程 |
| `InputConfigs` | `TArray<TSoftObjectPtr<const ULyraInputConfig>>`，标了 `AssetBundles="Client,Server"` |
| `IsDataValid()`（编辑器） | 检查列表里有没有空项 |
| `AddToWorld()` | 对 `APawn::StaticClass()` 注册扩展回调 |
| `HandlePawnExtension()` | 移除事件 → 撤；`ExtensionAdded` 或 **`NAME_BindInputsNow`** → 加 |
| `AddInputMappingForPlayer()` | 通过 `LyraHeroComponent::AddAdditionalInputConfig` 加进去 |
| `RemoveInputMapping()` | 反向移除 |
| `FPerContextData` | `ExtensionRequestHandles` + `PawnsAddedTo`（弱引用） |
| `ContextData` | 按 Context 分开记账 |

> 💡 它**不直接操作 EnhancedInput**，而是转交给 `ULyraHeroComponent` —— 因为"什么时候可以绑输入"由 HeroComponent 的 InitState 决定。

**优先级**：`AddInputMappingForPlayer` → `HandlePawnExtension`
