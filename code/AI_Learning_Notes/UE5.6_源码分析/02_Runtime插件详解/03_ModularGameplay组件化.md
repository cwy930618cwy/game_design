# 03 - ModularGameplay 组件化（角色架构）

> 涉及插件：`ModularGameplay` (46 文件) + `ModularGameplayActors`
> Lyra 使用度：⭐⭐⭐ **核心**（整个角色系统的架构基础）

---

## 一、这是什么？

**Modular Gameplay** 是 Lyra 角色组件化的**架构基础**。它把传统的"继承式"角色设计改成了"组合式"：

### 传统做法（继承）
```cpp
// 每个角色都要继承 ACharacter，然后重写各种函数
class AMyHero : public ACharacter { /* 几百行代码 */ };
class AMyEnemy : public ACharacter { /* 又几百行重复代码 */ };
class AMyNPC : public ACharacter { /* 还是几百行 */ };
```

### Lyra 做法（组合）
```cpp
// AModularCharacter 是个空壳，功能全靠组件拼装
ALyraCharacter (几乎空的)
  ├── ULyraPawnExtensionComponent  ← 协调初始化
  ├── ULyraHeroComponent           ← 输入 + 相机
  ├── ULyraHealthComponent         ← 生命值
  ├── ULyraCameraComponent         ← 相机模式
  └── ...其他业务组件
```

**好处**：
- ✅ 职责分离（每个组件只做一件事）
- ✅ 灵活组合（不同角色拼不同组件）
- ✅ 易于测试（组件可单独测试）
- ✅ 避免 God Class（不会有个几千行的角色类）

---

## 二、核心类

### 2.1 AModularPawn / AModularCharacter
继承自 `APawn` / `ACharacter`，但**几乎不添加任何功能**，只提供组件挂载点。

```cpp
// 关键：允许动态添加组件
class MODULARGAMEPLAY_API AModularPawn : public APawn
{
    // 支持通过接口添加/移除组件
    virtual void AddComponent(UActorComponent* Component);
};
```

### 2.2 IModularPawnInterface
组件实现这个接口，就能被 Modular Pawn 识别和管理。

```cpp
class IModularPawnInterface
{
    // 组件注册/注销回调
    virtual void OnModularPawnRegistered(APawn* Pawn);
    virtual void OnModularPawnUnregistered(APawn* Pawn);
};
```

### 2.3 UPawnDataAsset（DataAsset）
**数据驱动**的角色配置：

```cpp
// Lyra 的 BP_LyraPawnData
UPROPERTY()
TArray<FModularPawnComponentEntry> Components;  // 要添加哪些组件

UPROPERTY()
TArray<UAbilitySet*> AbilitySets;               // 授予哪些技能

UPROPERTY()
ULyraInputConfig* InputConfig;                  // 用什么输入配置
```

---

## 三、工作流程

### 3.1 角色初始化流程

```
1. 关卡 Spawn 一个 AModularCharacter
2. 读取 PawnDataAsset（从 ExperienceDefinition 来）
3. 遍历 Components 数组
4. 动态创建并注册每个组件
5. 每个组件的 OnModularPawnRegistered 被调用
6. 组件自行初始化（绑定输入、订阅消息等）
```

### 3.2 组件通信

组件之间**不直接引用**，而是通过**消息/事件**通信：

```cpp
// HeroComponent 需要生命值信息？
// 不直接找 HealthComponent，而是发个消息
MessageSystem->BroadcastMessage(MSGKEY("Pawn.Ready"), FPawnReadyMessage{this});

// HealthComponent 监听后自行处理
void ULyraHealthComponent::OnPawnReady(const FPawnReadyMessage& Msg)
{
    // 初始化生命值...
}
```

---

## 四、Lyra 的关键组件

### 4.1 ULyraPawnExtensionComponent ⭐
**组件化的中枢神经**，负责：
- 协调所有组件的初始化顺序
- 管理 PawnData 加载
- 处理网络同步的初始化状态

```cpp
// 初始化状态机
enum class ELyraPawnInitState
{
    Spawned,           // 刚生成
    DataAvailable,     // PawnData 已加载
    DataInitialized,   // 数据初始化完成
    GameplayReady      // 游戏就绪
};
```

### 4.2 ULyraHeroComponent
**玩家角色的专属组件**：
- 绑定 Enhanced Input
- 管理相机组件
- 处理复活/重生

### 4.3 ULyraHealthComponent
**生命值管理**：
- 监听 GAS 属性变化
- 处理死亡/复活
- 广播生命值事件

### 4.4 ULyraCameraComponent
**相机管理**：
- 管理相机模式（第三人称/第一人称）
- 处理相机穿透（避免穿模）

---

## 五、目录结构

```
ModularGameplay/
├── Source/
│   └── ModularGameplay/
│       ├── Public/
│       │   ├── ModularPawn.h           ← 模块化 Pawn 基类
│       │   ├── ModularCharacter.h      ← 模块化角色基类
│       │   ├── ModularPawnInterface.h  ← 接口
│       │   └── ModularGameplayModule.h
│       └── Private/
└── ModularGameplay.uplugin
```

---

## 六、与传统做法的对比

| 方面 | 传统继承 | Lyra 组件化 |
|------|---------|------------|
| 代码复用 | 难（重复代码多） | 易（组件复用） |
| 添加新功能 | 改角色类 | 加新组件 |
| 测试 | 难（耦合重） | 易（组件独立） |
| 网络同步 | 复杂 | 组件各自处理 |
| 学习曲线 | 低（直观） | 中高（需理解架构） |

---

## 七、学习建议

1. **先看 AModularCharacter** — 理解"空壳"设计
2. **再看 PawnExtensionComponent** — 理解初始化协调
3. **跟踪一个完整角色** — 如 Lyra 的 Cat 角色组装过程
4. **动手实践** — 创建一个自定义组件并挂载到角色

## 八、下一步

- [01_GameplayAbilities_GAS](./01_GameplayAbilities_GAS技能系统.md) — GAS 技能系统
- [02_CommonUI与UMG](./02_CommonUI与UMG.md) — UI 框架
- [04_GameFeatures与Experience](./04_GameFeatures与Experience.md) — GameFeature 机制
- [00_插件体系总览](./00_插件体系总览.md) — 回到总览
