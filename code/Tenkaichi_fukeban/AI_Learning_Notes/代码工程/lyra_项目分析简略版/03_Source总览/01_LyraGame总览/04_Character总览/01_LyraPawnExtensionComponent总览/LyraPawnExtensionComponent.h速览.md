# `LyraPawnExtensionComponent.h` 速览

> ⭐ **这一层最该先看的文件**。所有 Pawn 组件初始化的"总闸"，和 `LyraGameInstance::Init()` 里注册的 4 个 InitState 是一对。

| 成员 | 干嘛的 |
|---|---|
| `: UPawnComponent` + `IGameFrameworkInitStateInterface` | 只能挂 Pawn 上；参与 InitState 体系 |
| `NAME_ActorFeatureName` | 特性名 `"PawnExtension"`，别的组件靠它来"问进度" |
| `CanChangeInitState()` | ⭐ 判断**能不能**进入下一个状态 |
| `HandleChangeInitState()` | 状态真的变了之后要做的事 |
| `OnActorInitStateChanged()` | 别的 feature 状态变了时收到通知 |
| `CheckDefaultInitialization()` | 尝试把状态链往前推进 |
| `FindPawnExtensionComponent()` | 静态查找工具，全项目到处在用 |
| `GetPawnData<T>()`（模板） | 取当前 PawnData |
| `SetPawnData()` | ⭐ 设置 PawnData（`LyraGameMode` 生成 Pawn 时调的就是它） |
| `InitializeAbilitySystem()` / `UninitializeAbilitySystem()` | 把 Pawn 变成 ASC 的 Avatar / 反过来 |
| `HandleControllerChanged()` / `HandlePlayerStateReplicated()` / `SetupPlayerInputComponent()` | ⭐ 三个"时机通知"，都只是触发一次 `CheckDefaultInitialization()` |
| `OnAbilitySystemInitialized_RegisterAndCall()` | 注册 ASC 就绪回调；**已就绪就立即调用** |
| `OnAbilitySystemUninitialized_Register()` | 注册 ASC 卸载回调 |
| `PawnData` | `ReplicatedUsing=OnRep_PawnData` |
| `AbilitySystemComponent` | 缓存的 ASC（可能不属于自己，注释说明） |
| 两个委托 | `OnAbilitySystemInitialized` / `OnAbilitySystemUninitialized` |

**优先级**：`CanChangeInitState` → `SetPawnData` → `InitializeAbilitySystem`
