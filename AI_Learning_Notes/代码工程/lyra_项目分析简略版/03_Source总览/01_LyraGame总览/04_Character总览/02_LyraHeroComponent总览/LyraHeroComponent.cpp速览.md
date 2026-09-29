# `LyraHeroComponent.cpp` 速览

> 513 行，**是整个 Character 目录最大的文件**。

| 函数 | 干嘛的 |
|---|---|
| `OnRegister()` | 挂在非 Pawn 上会 Error + 编辑器弹 MessageLog（注释：PIE 会崩）；正常则 `RegisterInitStateFeature()` |
| `CanChangeInitState()` | ⭐ 四道关，见下 |
| `HandleChangeInitState()` | ⭐ 到 `DataInitialized` 时做三件事，见下 |
| `OnActorInitStateChanged()` | 只在乎 `PawnExtension` 这个 feature；它到 `DataInitialized` 就再推一次 |
| `InitializePlayerInput()` | ⭐ 见下 |
| `AddAdditionalInputConfig()` | 给 GameFeature 追加绑定 |
| `RemoveAdditionalInputConfig()` | ⚠️ **`//@TODO: Implement me!` —— 空实现** |
| `Input_AbilityInputTagPressed/Released` | 转成 ASC 的 `AbilityInputTagPressed/Released` |
| `Input_Move` / `Input_LookMouse` / `Input_LookStick` / `Input_Crouch` / `Input_AutoRun` | 具体输入响应 |
| `DetermineCameraMode()` | 优先返回技能设的 `AbilityCameraMode`，否则取 `PawnData->DefaultCameraMode` |

## `CanChangeInitState()` 的四道关（比 PawnExt 更严）

```
无 → Spawned          有 Pawn 就行
Spawned → DataAvailable
    · 必须有 ALyraPlayerState
    · 非模拟端：Controller 必须已"认领"自己的 PlayerState
    · 本地控制且非机器人：必须有 InputComponent + LyraPC + LocalPlayer
DataAvailable → DataInitialized
    · 有 PlayerState 且 ★ PawnExtension 已达 DataInitialized
DataInitialized → GameplayReady    直接通过（TODO 说这里该加技能初始化检查）
```

## `HandleChangeInitState()` 到 DataInitialized 时做的三件事

```
1. PawnExtComp->InitializeAbilitySystem(PS 上的 ASC, PS)   ← ASC 从这里接上
2. InitializePlayerInput(Pawn->InputComponent)             ← 绑输入
3. CameraComponent->DetermineCameraModeDelegate 绑到本组件  ← 相机模式交给它决定
```

## `InitializePlayerInput()` 的关键

```
ClearAllMappings()  ← 先清空所有映射
→ 注册 DefaultInputMappings（bRegisterWithSettings 的才加）
→ LyraIC->AddInputMappings / BindAbilityActions   ← 输入 → GameplayTag
→ BindNativeAction 逐个绑：Move / Look_Mouse / Look_Stick / Crouch / AutoRun
→ bReadyToBindInputs = true
★ 最后对 Controller 和 Pawn 各发一次 NAME_BindInputsNow 事件
```

> 💡 **最后那两行是串起整个体系的关键**：`GameFeatureAction_AddInputBinding` 和 `AddInputContextMapping` 都在这个事件之后才开始加自己的输入。这也解释了为什么"改键/加输入"要放在 GameFeature 里 —— 它们必须在本地输入就绪之后才生效。

> ⚠️ `RemoveAdditionalInputConfig` 是空的，意味着 **GameFeature 卸载时输入配置实际上没被移除**。

**优先级**：`HandleChangeInitState` → `InitializePlayerInput` → `CanChangeInitState`
