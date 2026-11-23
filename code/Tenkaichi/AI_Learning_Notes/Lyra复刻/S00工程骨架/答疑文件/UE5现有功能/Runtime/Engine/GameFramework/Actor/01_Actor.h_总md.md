# 01 — `Engine\Classes\GameFramework\Actor.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`（**4763 行 / 249 KB**）
> **⚠️ 这是引擎源码讲解，不是让你往 Tenkaichi 工程里新建这个文件** —— 引擎只读不改。
> 格式参照：`S00工程骨架\01_Tenkaichi.uproject_总md.md`

---

## 一、这个文件解决什么问题（为什么）

`AActor` 是**UE 里一切能放进关卡的东西的基类**。它解决三件事：

1. **生命周期** —— 从出生（`SpawnActor`）到 `BeginPlay` 到 `Tick` 到 `Destroyed`，有一套严格的调用顺序（156~193 行的注释就是官方清单）。
2. **组件容器** —— Actor 自己不渲染、不移动，靠挂 `UActorComponent`（`RootComponent` / `CharacterMovementComponent` 等）干活。
3. **网络复制** —— `bReplicates` 一开，属性和 RPC 就能在服务器与客户端之间同步。

🔑 **一句话比喻**：`AActor` 是「关卡里的居民身份证 + 日程表」—— 身份证决定它是谁、归谁管；日程表决定它什么时候出生、什么时候开始干活、什么时候销毁。

---

## 二、内部结构全景

```
Actor.h（4763 行）
├─ 1~33      文件头：pragma once + 25 个 include + Actor.generated.h
├─ 35~64     前置声明：AActor / AController / APawn / UActorChannel …
├─ 66~68     ACTOR_HAS_LABELS 宏（Test/Shipping 是否保留标签）
├─ 70~108    3 个枚举
│  ├─ EActorUpdateOverlapsMethod : 71~82   ← 初始化时重叠更新策略
│  ├─ ESpawnActorScaleMethod      : 85~93   ← 生成时缩放怎么处理
│  └─ EActorGridPlacement         : 96~108  ← 已废弃（UE5.0）
├─ 110       LogActor 日志类别
├─ 112~140   委托声明：伤害 / 重叠 / 命中 / 鼠标 / 触摸 / 销毁 / EndPlay / MakeNoise
├─ 142~154   TInlineComponentArray<T> 模板类（栈上预留的组件数组）
├─ 156~193   ★ AActor 类文档：完整生命周期调用顺序（最重要的一段注释）
├─ 194~197   UCLASS + class AActor : public UObject + GENERATED_BODY
├─ 199~4404  ★ 类体（占全文 88%）
│  ├─ 199~205    两个构造函数
│  ├─ 206~1391   属性区
│  │  ├─ 224  PrimaryActorTick     ← Tick 调度器
│  │  ├─ 266  bHidden              ← 是否隐藏
│  │  ├─ 388  bAllowTickBeforeBeginPlay
│  │  ├─ 465  bReplicates          ← ★ 网络复制总开关
│  │  └─ 741  SpawnCollisionHandlingMethod
│  ├─ 1392~2334  变换与位置
│  │  ├─ 1415 K2_GetActorLocation / 1432 K2_SetActorLocation（蓝图版）
│  │  ├─ 1484 SetActorLocation（C++ 版）
│  │  └─ 2332 GetActorLocation（FORCEINLINE 版）
│  ├─ 2335~3352  组件 / 所有权 / 生命周期
│  │  ├─ 1770 SetActorHiddenInGame / 1782 K2_DestroyActor
│  │  ├─ 1823 AddComponentByClass
│  │  ├─ 1877 AttachToActor
│  │  ├─ 1977 ReceiveEndPlay / 1980 EndPlay / 2028 ReceiveTick
│  │  ├─ 2806 SetOwner / 2810 GetOwner
│  │  └─ 2932 PostSpawnInitialize / 2935 FinishSpawning / 2945 PostInitializeComponents
│  ├─ 3353~4404  销毁 / 查询 / 网络 / 模板工具
│  │  ├─ 3353 Destroyed
│  │  ├─ 3550 GetWorld / 3553 GetWorldTimerManager
│  │  ├─ 3756~3843 GetComponents 模板族（5 个重载）
│  │  └─ 3909 GetIsReplicated
│  └─ 4066~4404  friend struct 声明
├─ 4405~4519  friend struct 定义（网络状态变更用的辅助结构）
│  ├─ 4405 FMarkActorIsBeingDestroyed
│  ├─ 4417 FSetActorWantsDestroyDuringBeginPlay
│  └─ 4430~4519 其他 Set* 辅助结构
└─ 4520~4763  辅助宏（含 4721~4728 的 GetActorLocation / SetActorLocation 转发）
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|---|---|---|---|
| `Actor.h` | ❌ 无（纯引擎头） | ✅ 属于 Core/Engine 模块 | ❌ 不需要 |

**关键依赖表**

| 依赖 | 作用 |
|---|---|
| `UObject/Object.h` | `AActor : UObject` 的基类 |
| `Engine/EngineTypes.h` | `FTransform` / `EEndPlayReason` 等枚举 |
| `InputCoreTypes.h` | `FKey` / `ETouchIndex`（鼠标触摸事件委托） |
| `Components/ChildActorComponent.h` | 子 Actor 组件 |
| `WorldPartition/WorldPartitionActorDescType.h` | 世界分区相关 |
| `Actor.generated.h` | UHT 生成的反射代码（编译期生成） |

**反向依赖（谁继承它）**

| 类 | 位置 |
|---|---|
| `APawn` → `ACharacter` → `ALyraCharacter` | Lyra 角色链 |
| `AController` → `APlayerController` → `ALyraPlayerController` | Lyra 控制链 |
| `AGameModeBase` → `ALyraGameMode` | Lyra 规则链 |

---

## 四、怎么教（推进节奏）

4763 行一比一注释后约 9000 行，**一份装不下**，按下列 7 片推进（**一轮只发一片**）：

| 片 | 行号 | 内容 | 产物 |
|---|---|---|---|
| 1 | 1~205 | 文件头 + 枚举 + 委托 + 类文档 + 构造函数 | `02_Actor.h复制文件_1of7.md` |
| 2 | 206~1100 | 属性区上半（Tick / 隐藏 / 复制开关） | `03_..._2of7.md` |
| 3 | 1101~2000 | 属性区下半 + 组件相关 | `04_..._3of7.md` |
| 4 | 2001~2900 | 变换 / 位置 / 所有权 | `05_..._4of7.md` |
| 5 | 2901~3800 | 生成初始化 / Tick / GetComponents | `06_..._5of7.md` |
| 6 | 3801~4404 | 网络 / 模板工具 / friend 声明 | `07_..._6of7.md` |
| 7 | 4405~4763 | friend struct 定义 + 辅助宏 | `08_..._7of7.md` |

> ⚠️ 每份复制文件 md 里的 `//` 注释**全部是你自己写的中文讲解**，原源码的英文注释已译中文。
> ⚠️ 注释独占一行、写在代码正上方；**零行尾注释**。

---

## 五、本节最该记住的 4 件事

1. **生命周期顺序**（156~193 行）：`PostLoad` → `OnComponentCreated` → `PreRegisterAllComponents` → `RegisterComponent` → `PostRegisterAllComponents` → `PostActorCreated` → `OnConstruction` → `PreInitializeComponents` → `PostInitializeComponents` → `BeginPlay`。
2. **`bReplicates` 是网络总开关**（465 行）：不开这个，属性复制和 RPC 全部失效。
3. **`PrimaryActorTick`**（224 行）：Tick 默认关闭，要手动 `PrimaryActorTick.bCanEverTick = true`。
4. **Actor 不干活，组件干活**：`RootComponent` 决定位置，`UActorComponent` 决定能力。

---

**下一步**：第 1 片 `02_Actor.h复制文件_1of7.md`（1~205 行，含最关键的 Actor 生命周期官方注释）。
