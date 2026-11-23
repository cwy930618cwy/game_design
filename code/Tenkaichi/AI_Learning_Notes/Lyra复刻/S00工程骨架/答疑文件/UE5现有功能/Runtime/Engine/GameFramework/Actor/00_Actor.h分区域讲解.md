# Actor.h 分区域讲解（说人话版）

> 依据：`Engine\Classes\GameFramework\Actor.h`（4763 行 / 249 KB）
> 定位：**不逐行堆代码**，把 4763 行按功能切成 10 个区域，每块讲清「在干嘛 / 为什么需要 / 关键成员」。
> 想逐行照抄看 `02_Actor.h复制文件_1of7.md`（分片 7 份）。

---

## 一、这个文件要解决什么

UE 里**凡是能放进关卡、能被生成、能挂组件、能联网的东西**，都是 `AActor` 的子类。这个文件就是给所有 Actor 定一套统一规则：什么时候出生、怎么挂组件、怎么移动、怎么联网、怎么死。

🔑 **比喻**：`AActor` 是「关卡居民的共同契约」—— 所有居民都按同一套流程出生和活动，只是各自往里填不同的内容。

---

## 二、10 个区域总表

| # | 行号 | 区域 | 一句话 |
|---|---|---|---|
| 1 | 1~64 | 文件头 + 前置声明 | include 一堆头，再声明一堆「有这个类」加快编译 |
| 2 | 66~108 | 宏 + 3 个枚举 | 标签开关、重叠更新策略、生成缩放策略、已废弃的分区放置 |
| 3 | 110~140 | 日志 + 事件委托 | `LogActor` + 十几个「受伤/重叠/命中/点击/销毁」事件签名 |
| 4 | 142~154 | `TInlineComponentArray` | 栈上预留空间的组件数组，小数量时不堆分配 |
| 5 | 156~205 | ★ 官方生命周期文档 + 类声明 | 全文件最值钱的一段注释，`AActor` 正式登场 |
| 6 | 206~1391 | 属性区（上千行） | 一堆 `UPROPERTY` 位域开关：Tick / 隐藏 / 复制 / 碰撞处理 |
| 7 | 1392~2334 | 变换与位置 | Get/Set 位置旋转缩放，蓝图版 `K2_*` 和 C++ 版并存 |
| 8 | 2335~3352 | 组件 / 所有权 / 生命周期 | 挂组件、附着、BeginPlay/EndPlay/Tick、生成初始化、Owner |
| 9 | 3353~4404 | 销毁 / 查询 / 网络 / 模板工具 | Destroyed、GetWorld、GetComponents 模板族、复制相关 |
| 10 | 4405~4763 | friend 辅助结构 + 宏 | 网络状态变更用的「只有特定类能改」的小助手 + 末尾转发宏 |

---

## 三、逐区域讲解

### 区域 1：文件头 + 前置声明（1~64 行）

**在干嘛**：先 include 依赖（`UObject/Object.h`、`EngineTypes.h`、`InputCoreTypes.h` 等），末尾放 `Actor.generated.h`（UHT 反射代码，必须最后）。然后一大排 `class AActor;` / `class AController;` 这种**前置声明**。

**为什么**：include 真头文件会把整个依赖树拉进来，编译很慢；前置声明只告诉编译器「有这个类、可以当指针用」，够用了。

**记住**：`Actor.generated.h` 永远是最后一个 include；类体第一行永远是 `GENERATED_BODY()`。这是 UHT 的铁律。

### 区域 2：宏 + 枚举（66~108 行）

**在干嘛**：三个枚举各管一件事：

- `EActorUpdateOverlapsMethod`（71 行）—— 关卡加载时重不重新算重叠（触碰）。
- `ESpawnActorScaleMethod`（85 行）—— 生成 Actor 时，传入的缩放是**覆盖**根组件默认缩放，还是**相乘**。
- `EActorGridPlacement`（96 行）—— 已废弃，UE5.0 标了 `UE_DEPRECATED`，看到跳过。

### 区域 3：日志 + 事件委托（110~140 行）

**在干嘛**：声明 `LogActor` 日志类别，然后一排 `DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_*`。

**为什么需要**：这些就是你在蓝图里拖的那些事件——`OnTakeAnyDamage`（受伤）、`OnActorBeginOverlap`（开始重叠）、`OnActorHit`（被撞）、`OnClicked`（被点击）、`OnDestroyed`（被销毁）。`SPARSE` 意思是没人绑定时不占内存。

**记住**：这一区只声明「信号长什么样」，真正的发送到区域 8/9 的实现里。

### 区域 4：`TInlineComponentArray`（142~154 行）

**在干嘛**：一个继承自 `TArray` 的小模板，但用 `TInlineAllocator` 在**对象内部**预留几格空间。

**为什么**：Actor 收集组件时通常就几个，用普通 `TArray` 要堆分配（new），这个直接在栈上放，省一次内存申请。`GetComponents`（区域 9）就是拿它当输出容器。

### 区域 5：官方生命周期文档 + 类声明（156~205 行）★

**在干嘛**：156~193 行是一大段注释，**官方把 Actor 的出生顺序列全了**：`PostLoad → OnComponentCreated → PreRegisterAllComponents → RegisterComponent → PostActorCreated → OnConstruction → PreInitializeComponents → PostInitializeComponents → BeginPlay`。194 行 `UCLASS(...)` 正式声明，195 行 `class AActor : public UObject`，199~205 是两个构造函数。

**为什么重要**：这段顺序是理解 UE 的钥匙。比如你问「为什么在 `BeginPlay` 里拿组件是对的，在构造函数里拿是空的？」—— 答案就在这：构造时组件还没注册。

**记住**：这一段值得单独背下来，比后面几百行都值钱。

### 区域 6：属性区（206~1391 行，最长）

**在干嘛**：上千行的 `UPROPERTY`，绝大多数是 `uint8 bXxx:1;` 这种**只占 1 个 bit 的开关**。按用途分：

- **网络复制**：`bNetTemporary`（226）、`bOnlyRelevantToOwner`（234）、`bAlwaysRelevant`（238）、`bReplicateMovement`（252）、`bHidden`（265）、`bTearOff`（268）
- **Tick**：`PrimaryActorTick`（224）—— Tick 的总调度器，`bCanEverTick` 默认关
- **碰撞生成**：`SpawnCollisionHandlingMethod`（741）—— 生成时和别的东西重叠了怎么办

**为什么这么多位域**：Actor 实例可能在关卡里有上万个，每个开关省一点，总量很可观。

**记住**：改这些开关**不要直接写字段**，用 `SetReplicates()` / `SetActorHiddenInGame()` 这类 setter——因为改完要通知网络层或渲染层。

### 区域 7：变换与位置（1392~2334 行）

**在干嘛**：一堆 Get/Set 位置、旋转、缩放的函数。两套接口并存：

- `K2_GetActorLocation`（1415）/ `K2_SetActorLocation`（1432）—— 给蓝图用的
- `SetActorLocation`（1484）—— 给 C++ 用的，带 `bSweep`、`OutSweepHitResult`、`Teleport` 参数
- `GetActorLocation`（2332）—— `FORCEINLINE` 版，直接读 `RootComponent`，最快

**为什么分两套**：蓝图版要处理 `bool` 返回值输出引脚，C++ 版用指针参数，语义一样但签名不同。

**记住**：`SetActorLocation` 的 `bSweep=true` 才会做碰撞检测，false 就是瞬间穿过去。

### 区域 8：组件 / 所有权 / 生命周期（2335~3352 行）

**在干嘛**：Actor 的「日常行为」都在这：

- 组件：`AddComponentByClass`（1823）运行时加组件、`AttachToActor`（1877）附着到别的 Actor
- 生命周期：`EndPlay`（1980）、`ReceiveTick`（2028）
- 所有权：`SetOwner` / `GetOwner`（2806/2810）—— 网络复制时决定「谁负责同步给它」
- 生成初始化：`PostSpawnInitialize`（2932）、`FinishSpawning`（2935）、`PostInitializeComponents`（2945）

**为什么需要**：这一区是「Actor 从出生到能玩」的实际执行部分，区域 5 的生命周期顺序就是靠这些函数实现的。

### 区域 9：销毁 / 查询 / 网络 / 模板工具（3353~4404 行）

**在干嘛**：

- `Destroyed`（3353）—— 被销毁的回调
- `GetWorld`（3550）、`GetWorldTimerManager`（3553）—— 拿世界和定时器
- `GetComponents` 模板族（3756~3843）—— 5 个重载，按类型/按数组捞组件
- `GetIsReplicated`（3909）—— 查这个 Actor 开没开复制

**为什么需要**：这是「工具箱区」—— 日常查状态、找组件、拿世界都来这。

### 区域 10：friend 辅助结构 + 宏（4405~4763 行）

**在干嘛**：一堆 `struct FMarkActorIsBeingDestroyed`、`FSetActorHiddenInSceneOutliner` 之类的小结构体，用途是「只允许特定类改某个字段」。比如 `FSetActorContentBundleGuid`（4520 行）只允许 `FContentBundleEditor` 改 `ContentBundleGuid`。

**为什么需要**：C++ 的 `friend` 机制——把字段设成 private，只给信得过的类开后门，防止乱改内部状态。

**记住**：这一区**不用逐行看**，知道「这是内部helper、外部碰不到」即可。4763 行到此结束。

---

## 四、对比表：C++ 版 vs 蓝图版接口

| 用途 | C++ 版 | 蓝图版（`K2_` 前缀） |
|---|---|---|
| 取位置 | `GetActorLocation`（2332） | `K2_GetActorLocation`（1415） |
| 设位置 | `SetActorLocation`（1484） | `K2_SetActorLocation`（1432） |
| 设旋转 | `SetActorRotation` | `K2_SetActorRotation` |
| 销毁 | `Destroy()` | `K2_DestroyActor`（1782） |

规律：**蓝图版统一加 `K2_` 前缀**，参数用引用做输出引脚。

---

## 五、几个要记住的点

1. **最值钱的是区域 5 那 38 行注释**（生命周期顺序），其余 4700 行都是在实现它。
2. **Actor 自己不干活，组件干活**——`RootComponent` 定位置，其他 `UActorComponent` 定能力。
3. **改状态用 setter，别直接写 `bXxx` 字段**，尤其网络相关的。
4. **末尾 350 行（区域 10）跳过也没关系**，是内部 friend 助手。
5. 想深挖哪一块，回来看对应的复制文件分片（1of7 ~ 7of7）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`（4763 行实查，行号均来自原文）
