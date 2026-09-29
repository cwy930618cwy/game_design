# 08 — GameInstance 与 GameMode 详解（游戏框架核心）

> **定位**：理解 UE 的**游戏框架**——`UGameInstance`（全局）、`AGameModeBase`（规则）、`APlayerController`（玩家）怎么协作。
>
> **一句话**：`UGameInstance` = **整个游戏的"全局实例"**（跨关卡不消失）；`AGameMode` = **当前关卡的"游戏规则"**（谁生成、怎么赢）。一个管"全局"，一个管"这一关"。
>
> **文件**：`Engine/Source/Runtime/Engine/Classes/`

---

## 一、先分清：UGameInstance vs GameMode（最容易混）

这两个名字像，但**生命周期和作用完全不同**：

| | `UGameInstance` | `AGameMode` |
|---|---|---|
| 是什么 | 整个游戏的**全局实例** | 当前关卡的**游戏规则** |
| 存活多久 | **整个游戏**（换关不消失） | 一个**关卡**（换关重建） |
| 管什么 | 跨关卡的全局数据、在线功能 | 这一关的规则（生成谁、胜负） |
| 谁能访问 | 所有场景都能拿 | 当前关卡内 |
| 例子 | 玩家总金币、语言设置、成就 | 死亡次数限制、得分规则 |

**一句话**：
- **GameInstance = 永久**（从游戏启动到退出都在）
- **GameMode = 临时**（只在当前关卡，换关就换）

---

## 二、UGameInstance —— 整个游戏的全局实例

### 2.1 是什么

**一个游戏进程只有一个 GameInstance**，游戏启动创建，退出销毁。**换关卡不销毁**，用来存"跨关卡"的数据。

```
游戏启动 → 创建 UGameInstance → 进入关卡1 → 关卡2 → ... → 退出销毁
              ↑ 一直活着，跨关卡保存数据
```

### 2.2 具体场景：存"跨关卡不消失"的数据

```cpp
UCLASS()
class UMyGameInstance : public UGameInstance {
    GENERATED_BODY()
public:
    // 跨关卡保存的数据
    int32 TotalCoins = 0;        // 总金币
    FString PlayerName;          // 玩家名
    TArray<FItem> GlobalInventory;  // 跨关卡背包

    UFUNCTION(BlueprintCallable)
    void AddCoins(int32 N) {
        TotalCoins += N;   // 换关卡金币还在
    }
};
```

**为什么用 GameInstance**：如果存在 Actor 里，换关就没了。GameInstance 跨关卡活着，所以全局数据放这里。

### 2.3 怎么访问

```cpp
// 获取 GameInstance
UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
// 或者
UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
GI->TotalCoins = 100;
```

---

## 三、GameMode —— 当前关卡的"游戏规则"

### 3.1 是什么

**每个关卡有一个 GameMode**，定义这一关的规则：
- 用什么 Pawn（玩家角色）
- 用什么 PlayerController（玩家控制器）
- 谁赢谁输（胜负规则）

### 3.2 具体场景：一个"谁先杀 10 个谁赢"的规则

```cpp
UCLASS()
class AMyGameMode : public AGameModeBase {
    GENERATED_BODY()
public:
    AMyGameMode() {
        // 规则 1：玩家默认用这个角色
        DefaultPawnClass = AMyCharacter::StaticClass();
        // 规则 2：玩家用这个控制器
        PlayerControllerClass = AMyPlayerController::StaticClass();
    }

    // 规则 3：记击杀数，够 10 就赢
    void KilledEnemy(AController* Killer) {
        KillCount++;
        if (KillCount >= 10) {
            // 宣布胜利
            UE_LOG(LogTemp, Log, TEXT("玩家获胜！"));
        }
    }
};
```

### 3.3 GameMode 的关键属性

| 属性 | 作用 |
|------|------|
| `DefaultPawnClass` | 玩家默认生成的角色 |
| `PlayerControllerClass` | 玩家控制器 |
| `HUDClass` | HUD 类 |
| `GameStateClass` | 游戏状态（多人同步） |

---

## 四、完整游戏框架：它们怎么协作

```
UGameInstance（全局，永久）
  └─ 跨关卡数据、在线功能

AGameMode（规则，当前关卡）
  └─ 决定：DefaultPawnClass / PlayerControllerClass / 胜负
       ├─ APlayerController（玩家大脑）
       │    └─ Possess → ACharacter（玩家身体）
       └─ 控制游戏流程

UWorld（世界）
  └─ 包含所有 Actor、关卡
```

**协作流程**：
```
游戏启动 → GameInstance 创建（存全局数据）
   ↓ 进入关卡
GameMode 创建（定规则）→ 生成 PlayerController → 附身到 ACharacter
   ↓ 玩家操作
Controller 指挥 Character，GameMode 判胜负
```

---

## 五、GameInstance vs GameMode vs 其他（完整对比）

| 类 | 存活 | 管什么 | 场景 |
|------|------|--------|------|
| `UGameInstance` | 永久 | 全局数据 | 总金币、语言、成就 |
| `AGameMode` | 当前关卡 | 规则、胜负 | 得分规则、谁生成 |
| `APlayerController` | 玩家 | 输入、视角 | WASD 控制 |
| `APlayerState` | 玩家 | 玩家状态 | 分数、名字（多人） |
| `AGameState` | 关卡 | 同步状态 | 计时、全局得分 |

**记忆**：
- 想**永久存数据** → GameInstance
- 想**定这一关规则** → GameMode
- 想**管玩家输入** → PlayerController
- 想**同步玩家状态**（多人）→ PlayerState

---

## 六、常见陷阱

**① 把跨关卡数据存 Actor（换关就没了）**
```cpp
// ❌ 存 Actor 里，换关卡就销毁
UCLASS() class AMyActor { int32 TotalCoins; };
// ✅ 存 GameInstance（跨关卡活着）
UCLASS() class UMyGameInstance : public UGameInstance { int32 TotalCoins; };
```

**② 搞混 GameInstance 和 GameMode**
```cpp
// ❌ 想存全局数据却放 GameMode（换关就没了）
// ✅ 全局数据放 GameInstance，关卡规则放 GameMode
```

**③ 忘设 GameMode 的 DefaultPawnClass**
```cpp
// ❌ 没设，玩家没角色生成
// ✅ AMyGameMode() { DefaultPawnClass = AMyCharacter::StaticClass(); }
```

---

## 七、总结速查

```
UGameInstance（全局，永久）
  - 跨关卡存数据：金币、语言、成就
  - 获取：GetGameInstance()

AGameMode（规则，当前关卡）
  - 定规则：DefaultPawnClass / PlayerControllerClass / 胜负
  - 换关卡就重建

分工：
  永久数据 → GameInstance
  关卡规则 → GameMode
  玩家输入 → PlayerController
  玩家状态（多人）→ PlayerState
```

**一句话**：`UGameInstance` 是**整个游戏的全局实例**（永久，存跨关卡数据），`AGameMode` 是**当前关卡的规则**（临时，定生成和胜负）。**永久数据放 GameInstance，关卡规则放 GameMode。**

---

## 八、下一步

理解了游戏框架，接下来可以深入 **GameMode 与 PlayerController 协作**（完整的一局游戏流程），或进入其他模块（渲染/UI/AI）。
