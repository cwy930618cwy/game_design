# `LyraHeroComponent.h` 速览

> 本地玩家侧的组件：**负责绑输入和选相机**。和 `LyraPawnExtensionComponent` 是搭档 —— 它跟着 PawnExt 的进度走。

| 成员 | 干嘛的 |
|---|---|
| `: UPawnComponent` + `IGameFrameworkInitStateInterface` | 也参与 InitState |
| `NAME_ActorFeatureName` | `"Hero"` |
| `NAME_BindInputsNow` | ⭐ `"BindInputsNow"` 事件名 —— `GameFeatureAction_AddInputBinding` 等的就是它 |
| `FindHeroComponent()` | 静态查找 |
| `SetAbilityCameraMode()` / `ClearAbilityCameraMode()` | 让技能临时接管相机（带 SpecHandle 校验归属） |
| `AddAdditionalInputConfig()` / `RemoveAdditionalInputConfig()` | 给 `GameFeatureAction_AddInputBinding` 用 |
| `IsReadyToBindInputs()` | ⭐ 上面的 Action 会先问这个 |
| `CanChangeInitState()` / `HandleChangeInitState()` / `OnActorInitStateChanged()` / `CheckDefaultInitialization()` | InitState 四件套 |
| `InitializePlayerInput()` | 真正绑输入的地方 |
| `Input_*` 系列 | 移动 / 鼠标看 / 手柄看 / 蹲 / 自动奔跑 |
| `DetermineCameraMode()` | 决定用哪个相机模式 |
| `DefaultInputMappings` | 组件上直接配的 IMC 列表 |
| `AbilityCameraMode` / `AbilityCameraModeOwningSpecHandle` | 技能设置的相机模式 + 归属 |
| `bReadyToBindInputs` | 输入是否已绑好 |

**优先级**：`InitializePlayerInput` → `CanChangeInitState` → `DetermineCameraMode`
