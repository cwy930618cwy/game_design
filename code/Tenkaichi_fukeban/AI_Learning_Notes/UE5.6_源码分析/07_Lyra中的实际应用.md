# 07 - Lyra 中的实际应用

> 将引擎源码知识与 Lyra 工程对照，理解实际项目如何用这些模块。

## 一、Lyra 工程结构回顾

```
LyraStarterGame/
├── Source/
│   ├── LyraGame/           ← Runtime 主模块（依赖大量引擎模块）
│   └── LyraEditor/         ← Editor 扩展模块
├── Plugins/                ← GameFeature 插件
├── Content/                ← 资产
└── Config/                 ← 配置
```

---

## 二、Lyra 用到的引擎模块

### 2.1 LyraGame.Build.cs 依赖分析

```csharp
// 核心依赖（必选）
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",                    // 基础类型
    "CoreUObject",             // UObject 系统
    "Engine",                  // 游戏引擎核心
    "InputCore"                // 输入核心
});

// 私有依赖（按需）
PrivateDependencyModuleNames.AddRange(new string[] {
    "NetCore",                 // 网络
    "GameplayAbilities",       // GAS ⭐
    "GameplayTags",            // Tag 系统 ⭐
    "GameplayTasks",           // 异步任务
    "ModularGameplay",         // 模块化玩法 ⭐
    "ModularGameplayActors",   // 模块化 Actor
    "SignificanceManager",     // 重要性管理
    "UMG",                     // UI ⭐
    "Slate",                   // UI 底层
    "SlateCore",               // UI 底层
    "CommonUI",                // CommonUI ⭐
    "CommonGame",              // 通用游戏框架
    "EnhancedInput",           // 增强输入 ⭐
    "GameFeatures",            // GameFeature 机制 ⭐
    "AsyncMixin",              // 异步混入
    "GameplayMessageRuntime",  // 消息路由
    "ControlFlows",            // 控制流
    "DeveloperSettings",       // 开发者设置
    "Niagara",                 // 粒子
    "Water",                   // 水体
    "AudioMixer",              // 音频
    "Metasound",               // MetaSound
    "NavigationSystem",        // 导航
    "AIModule",                // AI
    "StateTreeModule",         // 状态树
    "SmartObjectsModule",      // 智能对象
    "GameplayInteractionsModule", // 交互
    "ContextualAnimationModule", // 上下文动画
    "AnimationWarpingRuntime", // 动画扭曲
    "AnimationLocomotionLibrary", // 移动动画库
    "MotionWarping",           // 运动扭曲
    "PhysicsCore",             // 物理
    "ReplicationGraph",        // 网络复制优化
    "LevelSequence",           // 过场
    "MovieScene",              // 影片
    "CinematicCamera",         // 电影相机
    "GameSettings",            // 游戏设置
    "CommonUser",              // 用户身份
    "CommonConversation",      // 对话
    "PocketWorlds",            // 口袋世界
    "UIExtension",             // UI 扩展
});
```

### 2.2 对应关系表

| Lyra 功能 | 使用的引擎模块 | 源码位置 |
|-----------|---------------|---------|
| **角色组件化** | ModularGameplay + ModularGameplayActors | `Runtime/Experimental/` |
| **GAS 技能系统** | GameplayAbilities + GameplayTags + GameplayTasks | `Plugins/Runtime/GameplayAbilities/` |
| **CommonUI 界面** | CommonUI + UMG + Slate | `Plugins/Runtime/CommonUI/` |
| **Enhanced Input** | EnhancedInput + InputCore | `Plugins/EnhancedInput/` |
| **GameFeature 插件** | GameFeatures | `Plugins/Runtime/GameFeatures/` |
| **网络优化** | ReplicationGraph + NetCore | `Plugins/Runtime/ReplicationGraph/` |
| **性能优化** | SignificanceManager | `Plugins/Runtime/SignificanceManager/` |
| **消息路由** | GameplayMessageRouter | `Plugins/Runtime/GameplayMessageRouter/` |
| **Niagara 特效** | Niagara | `Plugins/FX/Niagara/` |
| **MetaSound 音频** | Metasound + AudioMixer | `Plugins/Runtime/Metasound/` |
| **AI 行为** | AIModule + NavigationSystem + StateTree | `Runtime/AIModule/` |
| **动画系统** | AnimationWarping + MotionMatching + PoseSearch | `Plugins/Animation/` |
| **智能对象** | SmartObjects | `Plugins/Experimental/SmartObjects/` |
| **交互系统** | GameplayInteractions | `Plugins/Runtime/GameplayInteractions/` |
| **口袋世界** | PocketWorlds | `Plugins/Runtime/PocketWorlds/` |

---

## 三、Lyra 关键类的继承链

### 3.1 游戏框架类

```
UObject (CoreUObject)
  └── AActor (Engine)
        ├── ALyraGameMode : AModularGameModeBase
        ├── ALyraGameState : AModularGameStateBase
        ├── ALyraPlayerController : AModularPlayerController
        ├── ALyraPlayerState : AModularPlayerState
        ├── ALyraPawn : AModularPawn
        │     └── ALyraCharacter : AModularCharacter
        ├── ALyraPlayerStart : APlayerStart
        └── ALyraCameraActor : ACameraActor
```

### 3.2 组件类

```
UActorComponent (Engine)
  └── USceneComponent
        ├── UStaticMeshComponent
        ├── USkeletalMeshComponent
        └── UCapsuleComponent
  
  └── ULyraPawnExtensionComponent (Lyra)
  └── ULyraHeroComponent (Lyra)
  └── ULyraHealthComponent (Lyra)
  └── ULyraCameraComponent (Lyra)
  └── ULyraAbilitySystemComponent (Lyra)
        └── 继承自 UAbilitySystemComponent (GameplayAbilities 插件)
```

### 3.3 AbilitySystem 类

```
UObject
  └── UGameplayAbility (GameplayAbilities)
        └── ULyraGameplayAbility (Lyra)
              ├── ULyraGameplayAbility_Jump
              ├── ULyraGameplayAbility_Dash
              ├── ULyraGameplayAbility_Melee
              └── ULyraGameplayAbility_RangedWeapon

  └── UAttributeSet (GameplayAbilities)
        └── ULyraAttributeSet (Lyra)
              ├── ULyraHealthSet
              └── ULyraCombatSet

  └── UGameplayEffect (GameplayAbilities)
        └── GE_LyraDamage
        └── GE_LyraHeal
```

---

## 四、Lyra 中的设计模式

### 4.1 组件化模式 (Component Pattern)

```cpp
// Lyra 不用继承，用组件组合
ALyraCharacter (空壳)
  ├── ULyraPawnExtensionComponent  ← 协调初始化顺序
  ├── ULyraHeroComponent           ← 输入 + 相机
  ├── ULyraHealthComponent         ← 生命值
  ├── ULyraCameraComponent         ← 相机模式
  └── ULyraEquipmentManagerComponent ← 装备管理
```

**好处**：
- 职责分离
- 易于测试
- 灵活组合
- 避免 God Class

### 4.2 数据驱动模式 (Data-Driven)

```cpp
// 不用硬编码，用 DataAsset
ULyraExperienceDefinition* Experience;  // 决定加载什么 GameFeature
ULyraPawnData* PawnData;                // 决定角色的能力/输入/外观
ULyraAbilitySet* AbilitySet;            // 决定 GA/GE/AttributeSet
ULyraInputConfig* InputConfig;          // 决定按键映射
```

### 4.3 模块化模式 (Module Pattern)

```cpp
// GameFeature 运行时热插拔
ExperienceDefinition
  └── GameFeatureAction_AddAbilities
  └── GameFeatureAction_AddWidgets
  └── GameFeatureAction_AddInputBinding
  └── GameFeatureAction_SpawnActor
```

### 4.4 观察者模式 (Observer Pattern)

```cpp
// GameplayMessage 消息路由
FGenericMulticastMessageListenerHandle Handle = 
    MessageSystem->RegisterListener<FMyMessage>(
        MSGKEY("My.Message"),
        this,
        &MyClass::OnMessage
    );
```

---

## 五、常见开发场景与源码对照

### 5.1 新增一个角色

| 步骤 | 涉及源码 |
|------|---------|
| 创建 PawnData | `Source/LyraGame/GameModes/LyraPawnData.h` |
| 添加组件 | `Source/LyraGame/Character/LyraPawnExtensionComponent.cpp` |
| 配置 AbilitySet | `Source/LyraGame/AbilitySystem/LyraAbilitySet.h` |
| 绑定输入 | `Source/LyraGame/Input/LyraInputConfig.h` |
| 设置外观 | `Source/LyraGame/Character/LyraCosmeticComponent.cpp` |

### 5.2 新增一个技能 (GA)

| 步骤 | 涉及源码 |
|------|---------|
| 创建 GA 类 | `Source/LyraGame/AbilitySystem/Abilities/LyraGameplayAbility.h` |
| 定义属性 | `Source/LyraGame/AbilitySystem/Attributes/LyraHealthSet.h` |
| 创建 GE | `Source/LyraGame/AbilitySystem/Effects/LyraGameplayEffect.h` |
| 添加 Cue | `Source/LyraGame/AbilitySystem/Executions/LyraDamageExecution.h` |

### 5.3 新增一个 UI 界面

| 步骤 | 涉及源码 |
|------|---------|
| 创建 Widget | `Source/LyraGame/UI/LyraHUDLayout.h` |
| 推送到屏幕 | `Plugins/Runtime/CommonUI/` |
| 绑定数据 | `Source/LyraGame/UI/Foundation/LyraActivatableWidget.h` |
| 消息触发 | `Plugins/Runtime/GameplayMessageRouter/` |

---

## 六、学习建议

1. **先看继承链** — 理解 Lyra 类在引擎中的位置
2. **再看依赖** — 理解每个功能用了哪些引擎模块
3. **最后看设计模式** — 理解为什么这样组织代码
4. **动手实践** — 跟着教程做一个新功能

## 七、下一步

- [04_Runtime核心模块](./04_Runtime核心模块.md) — 深入 Core/CoreUObject/Engine
- [05_Editor模块详解](./05_Editor模块详解.md) — 编辑器架构
- [08_文件类型与宏速查](./08_文件类型与宏速查.md) — 速查手册
