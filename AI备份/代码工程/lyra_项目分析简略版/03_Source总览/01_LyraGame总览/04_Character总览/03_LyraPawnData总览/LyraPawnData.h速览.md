# `LyraPawnData.h` 速览

> **只有 5 个字段的数据资产**，却决定了一个角色的绝大部分特性 —— 这就是 Lyra "改配置不改代码" 的落地处。

| 成员 | 干嘛的 |
|---|---|
| `: UPrimaryDataAsset`（`BlueprintType`, `Const`） | 蓝图可建、标记为只读 |
| `PawnClass` | 实际生成哪个 Pawn 类（应派生自 `ALyraPawn` 或 `ALyraCharacter`） |
| `AbilitySets` | 要给这个 Pawn 的 ASC 授予哪些 `ULyraAbilitySet` |
| `TagRelationshipMapping` | 技能 Tag 的互斥/依赖关系表 |
| `InputConfig` | ⭐ 玩家控制时用的 `ULyraInputConfig`（决定按键→技能） |
| `DefaultCameraMode` | 默认的相机模式类 |

## 这 5 个字段分别被谁消费

| 字段 | 消费方 |
|---|---|
| `PawnClass` | `LyraGameMode::GetDefaultPawnClassForController` |
| `AbilitySets` | `LyraPawnExtensionComponent` 初始化 ASC 时授予 |
| `TagRelationshipMapping` | `LyraPawnExtensionComponent::InitializeAbilitySystem` |
| `InputConfig` | `LyraHeroComponent::InitializePlayerInput` |
| `DefaultCameraMode` | `LyraHeroComponent::DetermineCameraMode` |

**说明**：想做一个新角色/新职业，**通常只需要新建一个 PawnData 资产**，不用写 C++。
