# 09 — UWorld 与 ULevel 详解（关卡世界）

> **定位**：理解 UE 的**世界（World）和关卡（Level）**——你的游戏场景是怎么组织的。
>
> **一句话**：`UWorld` = **一个游戏世界**（包含所有 Actor、关卡、GameMode）；`ULevel` = **世界里的一个关卡**（一个场景）。**世界是容器，关卡是其中的场景**，一个世界可以有多个关卡。
>
> **文件**：`Engine/Source/Runtime/Engine/Classes/Engine/`（`World.h`、`Level.h`）

---

## 一、先分清：UWorld vs ULevel（核心）

| | `UWorld` | `ULevel` |
|---|---|---|
| 是什么 | **世界**（游戏运行的"整个环境"） | **关卡**（一个场景/地图） |
| 包含什么 | 所有关卡、所有 Actor、GameMode | 一个关卡里的 Actor |
| 数量 | 一个游戏通常一个世界 | 一个世界可**多个**关卡 |
| 类比 | 一个"城市" | 城市里的"一栋楼" |

**一句话**：
- **UWorld = 城市**（整个环境）
- **ULevel = 城市里的一栋楼**（一个场景）

---

## 二、UWorld —— 关卡世界

### 2.1 是什么

**UWorld 是游戏运行的"世界"**，它是最高层的容器，包含：
- 所有的关卡（Level）
- 所有的 Actor
- GameMode（游戏规则）
- 物理、碰撞、光照等环境

```
UWorld（世界）
├── ULevel 1（关卡1）
├── ULevel 2（关卡2）  ← 一个世界可多个关卡
├── GameMode（规则）
├── 所有 Actor
└── 物理/碰撞/光照
```

### 2.2 具体场景：获取世界，生成 Actor

```cpp
// 生成一个 Actor 到世界里（你天天用这个）
FVector Loc(100.f, 0.f, 0.f);
FRotator Rot(0.f, 0.f, 0.f);
AEnemy* Enemy = GetWorld()->SpawnActor<AEnemy>(AEnemy::StaticClass(), Loc, Rot);
//                       ↑ 世界负责生成 Actor
```

### 2.3 常用操作

```cpp
// 获取世界（几乎所有 Actor/组件都有）
UWorld* World = GetWorld();

// 获取当前关卡
ULevel* Level = World->GetCurrentLevel();

// 获取 GameMode
AGameModeBase* GM = World->GetAuthGameMode();

// 获取所有 Actor
for (TActorIterator<AEnemy> It(World); It; ++It) {
    AEnemy* Enemy = *It;
}
```

**关键**：`GetWorld()` 是 UE 里**最常用的函数之一**——你在任何 Actor/组件里都能拿到世界，然后生成物体、查询关卡。

---

## 三、ULevel —— 关卡

### 3.1 是什么

**ULevel 是一个"关卡"**（一个场景/地图），它包含这一关里的所有 Actor。

```
ULevel（一个关卡）
├── 关卡里所有 Actor（地形、敌人、触发器）
├── 关卡设置
└── 关卡里的世界
```

### 3.2 关卡 vs 世界的关系（关键）

```
UWorld（世界，容器）
  └─ ULevel（关卡，场景）
       └─ AActor（关卡里的东西）
```

**多层关卡（大世界）**：
```
UWorld
├── ULevel 1（基础关卡：地形）
├── ULevel 2（战斗关卡）
└── ULevel 3（可流送关卡）
    ← UE5 大世界用"多关卡"动态加载，用到才加载
```

### 3.3 具体场景：切换关卡 / 加载关卡

```cpp
// 加载一个新关卡（Open Level）
UGameplayStatics::OpenLevel(World, TEXT("Level_2"));
//                        ↑ 切换到 Level_2

// 流送加载一个关卡（不切换，叠加）
UGameplayStatics::LoadStreamLevel(this, TEXT("Level_3"), true, true);
```

---

## 四、World / Level / Actor / GameMode 的完整关系图

```
        UWorld（世界：容器）
        ├── ULevel（关卡：场景）
        │    └── AActor（关卡里的东西）
        │         └── UActorComponent（零件）
        ├── AGameMode（规则）
        ├── UGameInstance（全局，跨关卡）
        └── 物理/碰撞/光照
```

**协作流程**：
```
游戏启动
  → GameInstance 创建（全局数据）
  → UWorld 创建（世界）
  → 加载 ULevel（关卡）
      → 生成关卡里的 AActor
      → GameMode 定规则，PlayerController 控制玩家
```

---

## 五、你写代码最常用的（GetWorld 全家桶）

```cpp
// 生成 Actor
GetWorld()->SpawnActor<AActor>(Class, Loc, Rot);

// 获取当前关卡
GetWorld()->GetCurrentLevel();

// 获取 GameMode
GetWorld()->GetAuthGameMode();

// 遍历 Actor
for (TActorIterator<AActor> It(GetWorld()); It; ++It) { ... }

// 切换关卡
UGameplayStatics::OpenLevel(this, TEXT("Level_2"));

// 获取游戏实例
GetWorld()->GetGameInstance();
```

---

## 六、常见陷阱

**① 混淆 World 和 Level**
```cpp
// ❌ 以为关卡就是世界
// ✅ World 是容器（世界），Level 是场景（关卡），一个世界多个关卡
```

**② 在没 World 的地方调 GetWorld()**
```cpp
// ❌ 构造函数里可能没 World（对象还没进世界）
AActor() { GetWorld()->SpawnActor(...); }   // 可能空指针
// ✅ 在 BeginPlay 里（这时已在世界）
void BeginPlay() { GetWorld()->SpawnActor(...); }
```

**③ 生成 Actor 忘了传参数**
```cpp
// ❌ SpawnActor 需要类 + 位置 + 旋转
GetWorld()->SpawnActor<AEnemy>(AEnemy::StaticClass());
// ✅
GetWorld()->SpawnActor<AEnemy>(AEnemy::StaticClass(), Loc, Rot);
```

---

## 七、总结速查

```
UWorld（世界，容器）
├── ULevel（关卡，场景）
│    └── AActor（东西）
├── GameMode（规则）
└── 物理/光照

关系：世界(World) 包含 关卡(Level) 包含 Actor
常用：
  GetWorld() → 生成 Actor、查关卡、拿 GameMode
  OpenLevel() → 切换关卡
  GetCurrentLevel() → 当前关卡
```

**一句话**：`UWorld` 是**游戏世界**（包含所有关卡和 Actor），`ULevel` 是**一个关卡**（一个场景）。**世界是容器，关卡是场景**。你写代码用 `GetWorld()` 生成 Actor、切换关卡、拿 GameMode，是 UE 最常用的入口之一。

---

## 八、下一步

理解了世界和关卡，你已经把 UE 的对象体系、游戏框架、场景组织都过了一遍。接下来可以深入渲染（Renderer）、UI（Slate/UMG）、AI，或先沉淀已学内容。
