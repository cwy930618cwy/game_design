# `Character/` 目录速览

> 8 个类，17 个文件（含一个 `.code-workspace`），**没有子目录**。
> 这里是 Lyra **组件化角色体系**的核心，也是 InitState 状态机的主要舞台。

## 继承关系

```
APawn
├── ALyraPawn              ← : AModularPawn，通用 Pawn（载具等）
└── ACharacter
    └── ALyraCharacter     ← : AModularCharacter，角色本体
        └── ALyraCharacterWithAbilities   ← 自带 ASC，不依赖 PlayerState
```

## 8 个类

| 类 | 干嘛的 |
|---|---|
| `LyraPawnExtensionComponent` | ⭐⭐ 所有 Pawn 组件初始化的**总闸**（InitState 状态机） |
| `LyraHeroComponent` | ⭐ 本地玩家侧：绑定输入 + 选相机模式（也是 InitState 参与者） |
| `LyraPawnData` | ⭐ 数据资产，描述"这个角色是什么"（生成类/技能集/输入/相机） |
| `LyraPawn` | Pawn 基类，主要实现队伍归属 |
| `LyraCharacter` | 角色基类，挂 3 个组件 + 死亡流程 + 网络移动优化 |
| `LyraCharacterWithAbilities` | 自带 ASC 的角色（给不需要 PlayerState 的对象用） |
| `LyraHealthComponent` | 血量、死亡状态机、死亡广播 |
| `LyraCharacterMovementComponent` | 移动组件，加了地面信息缓存和加速度复制 |

## 补充说明

| 点 | 说明 |
|---|---|
| ⭐ 两个 InitState 参与者 | `LyraPawnExtensionComponent` 和 `LyraHeroComponent` 都实现 `IGameFrameworkInitStateInterface`，**谁先谁后由 PawnExtComponent 统一排** |
| 角色身上挂了 3 个组件 | `PawnExtComponent`、`HealthComponent`、`CameraComponent` |
| ASC 从哪来 | 普通 `LyraCharacter` 从 **PlayerState** 上取；`LyraCharacterWithAbilities` 自带一个 |
| `LyraPawnData` 的 5 个字段 | `PawnClass` / `AbilitySets` / `TagRelationshipMapping` / `InputConfig` / `DefaultCameraMode` |
| 死亡是三态 | `ELyraDeathState`：`NotDead` → `DeathStarted` → `DeathFinished`，复制给客户端 |
| 网络移动优化 | `FastSharedReplication` + `FSharedRepMovement`，**跳帧时补发一次移动更新** |
| 移动组件的两个增量 | `GetGroundInfo()` 带帧号缓存的地面信息；`SetReplicatedAcceleration()` 量化后的加速度 |

> 💡 **`LyraPawnExtensionComponent` 是这一层最该先看的** —— 它也是 `LyraGameInstance::Init()` 里那 4 个 InitState 的另一半。两端对上，Lyra 的初始化时序就通了。

**优先级**：`LyraPawnExtensionComponent` → `LyraHeroComponent` → `LyraPawnData` → `LyraCharacter`
