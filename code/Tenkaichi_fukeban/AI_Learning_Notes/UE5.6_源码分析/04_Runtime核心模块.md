# 04 - Runtime 核心模块（Core / CoreUObject / Engine）

> 这三大模块是 UE 的**地基**，所有游戏代码都建立在这之上。
> 路径：`c:\Program Files\Epic Games\UE_5.6\Engine\Source\Runtime\`

---

## 一、Core 模块（1,993 文件）

### 1.1 模块定位
**整个引擎的基础层**，提供：
- 基础数据类型
- 容器类
- 数学库
- 委托/lambda
- 内存管理
- 字符串处理
- 文件 IO
- 线程同步

### 1.2 目录结构
```
Core/
├── Public/              ← 公开头文件
│   ├── CoreMinimal.h    ← ⭐ 最常被包含的头文件
│   ├── CoreTypes.h      ← 基础类型定义
│   ├── Containers/      ← 容器类（TArray/TMap/TSet）
│   ├── Math/            ← 数学库（FVector/FQuat/FTransform）
│   ├── Delegates/       ← 委托系统
│   ├── Templates/       ← 模板工具
│   ├── Serialization/   ← 序列化接口
│   └── ...
├── Private/             ← 私有实现
└── Core.Build.cs        ← 模块依赖
```

### 1.3 必知必会

#### 基础类型
| 类型 | 说明 |
|------|------|
| `int32`, `uint32`, `int64`, `uint64` | 固定宽度整数 |
| `float` (32-bit), `double` (64-bit) | 浮点数 |
| `bool` (`bIsXXX`) | 布尔（UE 规范用小写 b 前缀） |
| `TCHAR` | 宽字符（Windows 上是 wchar_t） |
| `FString` | 字符串（推荐） |
| `FName` | 名称（哈希优化，用于标识符） |
| `FText` | 本地化文本 |

#### 容器类
| 容器 | 说明 |
|------|------|
| `TArray<T>` | 动态数组（最常用） |
| `TMap<K,V>` | 键值对 |
| `TSet<T>` | 集合 |
| `TQueue<T>` | 队列 |
| `TMultiMap<K,V>` | 多值映射 |

#### 数学库
| 类型 | 说明 |
|------|------|
| `FVector` | 3D 向量 |
| `FVector2D` | 2D 向量 |
| `FQuat` | 四元数 |
| `FRotator` | 旋转（欧拉角） |
| `FTransform` | 变换矩阵 |
| `FMath` | 数学函数集 |

#### 委托系统
| 类型 | 说明 |
|------|------|
| `DECLARE_DELEGATE` | 单播委托 |
| `DECLARE_MULTICAST_DELEGATE` | 多播委托 |
| `DECLARE_DYNAMIC_DELEGATE` | 动态委托（可序列化，蓝图可见） |
| `DECLARE_EVENT` | 事件 |

### 1.4 智能指针
| 类型 | 说明 |
|------|------|
| `TSharedPtr<T>` | 共享指针（引用计数） |
| `TWeakPtr<T>` | 弱指针 |
| `TUniquePtr<T>` | 独占指针 |
| `TSharedRef<T>` | 共享引用（非空） |

### 1.5 常用宏
```cpp
// 日志
UE_LOG(LogTemp, Log, TEXT("Message"));

// 检查
check(expr);           // 断言（Shipping 移除）
ensure(expr);          // 确保（Shipping 保留但优化）
verify(expr);          // 验证（Shipping 也执行）

// 性能追踪
TRACE_CPUPROFILER_EVENT_SCOPE(FunctionName);

// 强制内联
FORCEINLINE void Foo() {}

// 平台相关
PLATFORM_WINDOWS
WITH_EDITOR
```

---

## 二、CoreUObject 模块（856 文件）

### 2.1 模块定位
**UObject 系统**——UE 的对象模型和反射系统，这是 UE 区别于其他引擎的核心特性。

### 2.2 核心概念

#### UObject 生命周期
```
NewObject<T>() → PostInitProperties → PostLoad → BeginPlay → EndPlay → BeginDestroy → FinishDestroyed
```

#### 垃圾回收 (GC)
- UE 使用**标记-清除** GC
- 通过 `UPROPERTY()` 宏标记需要 GC 追踪的指针
- 每帧自动运行（可通过 `-nogc` 关闭调试）

### 2.3 关键宏

```cpp
// 类声明
UCLASS([specifiers])
class MYMODULE_API AMyClass : public AActor { GENERATED_BODY() };

// 结构体声明
USTRUCT([specifiers])
struct MYMODULE_API FMyStruct { GENERATED_BODY() };

// 枚举声明
UENUM([specifiers])
enum class EMyEnum : uint8 { Value1, Value2 };

// 属性（支持反射/GC/蓝图访问）
UPROPERTY([specifiers])
float Health;

// 函数（支持蓝图调用/事件）
UFUNCTION([specifiers])
void TakeDamage(float Amount);

// 网络复制
UPROPERTY(Replicated)
int32 Score;

// 编辑器可见
UPROPERTY(EditAnywhere, BlueprintReadWrite)
int32 MaxHealth;
```

### 2.4 UPROPERTY 常用修饰符

| 修饰符 | 说明 |
|--------|------|
| `EditAnywhere` | 任何地方可编辑 |
| `EditDefaultsOnly` | 仅默认值可编辑 |
| `VisibleAnywhere` | 可见但不可编辑 |
| `BlueprintReadOnly` | 蓝图只读 |
| `BlueprintReadWrite` | 蓝图读写 |
| `Category="Name"` | 分类 |
| `meta=(...)` | 元数据 |
| `Replicated` | 网络复制 |
| `SaveGame` | 存档 |
| `Transient` | 不序列化 |

### 2.5 关键类

| 类 | 作用 |
|----|------|
| `UObject` | 所有对象的基类 |
| `AActor` | 场景中的 Actor |
| `UActorComponent` | Actor 组件基类 |
| `UWorld` | 世界 |
| `ULevel` | 关卡 |
| `AGameModeBase` | 游戏模式基类 |
| `AGameStateBase` | 游戏状态基类 |
| `APlayerController` | 玩家控制器 |
| `APawn` | Pawn 基类 |
| `UClass` | 类的元信息 |
| `UFunction` | 函数的元信息 |
| `FProperty` | 属性的元信息 |

### 2.6 反射系统

```cpp
// 运行时获取类信息
UClass* Class = MyObject->GetClass();
FProperty* Prop = Class->FindPropertyByName(TEXT("Health"));
UFunction* Func = Class->FindFunctionByName(TEXT("TakeDamage"));

// 调用函数
MyObject->ProcessEvent(Func, &Params);

// 创建对象
UMyClass* NewObj = NewObject<UMyClass>();
```

### 2.7 序列化

```cpp
// 重写 Serialize 函数
virtual void Serialize(FArchive& Ar) override;

// 使用 << 运算符
Ar << Health << MaxHealth;

// 条件序列化
if (Ar.IsSaving()) { ... }
if (Ar.IsLoading()) { ... }
if (Ar.IsCooking()) { ... }
```

---

## 三、Engine 模块（3,832 文件）

### 3.1 模块定位
**游戏引擎的核心实现**，包含 Actor、Component、World、GameInstance 等所有游戏框架类。

### 3.2 目录结构
```
Engine/
├── Classes/             ← 头文件（按功能分类）
│   ├── GameFramework/   ← 游戏框架类
│   ├── Components/      ← 组件类
│   ├── Engine/          ← 引擎核心类
│   ├── AI/              ← AI 相关
│   ├── Animation/       ← 动画相关
│   ├── Camera/          ← 相机相关
│   ├── Particles/       ← 粒子相关
│   └── ...
├── Private/             ← 实现文件
└── Public/              ← 公开头文件
```

### 3.3 GameFramework 核心类

| 类 | 作用 | 生命周期 |
|----|------|---------|
| `UGameInstance` | 游戏实例（全局单例） | 引擎启动→关闭 |
| `UWorld` | 世界 | 关卡加载→卸载 |
| `ULevel` | 关卡 | World 的子集 |
| `AGameModeBase` | 游戏模式（规则） | 仅在服务器 |
| `AGameStateBase` | 游戏状态（全局数据） | 所有客户端 |
| `APlayerController` | 玩家控制器（输入） | 每个玩家 |
| `APlayerState` | 玩家状态（个人数据） | 每个玩家 |
| `APawn` | Pawn（可被控制的 Actor） | 场景中 |
| `ACharacter` | 角色（带移动组件） | 场景中 |
| `ALevelScriptActor` | 关卡脚本 | 每个关卡 |
| `AInfo` | Info 类（无物理存在） | 场景中 |

### 3.4 组件体系

| 组件 | 作用 |
|------|------|
| `UActorComponent` | 组件基类 |
| `USceneComponent` | 场景组件（有 Transform） |
| `UPrimitiveComponent` | 图元组件（有碰撞/渲染） |
| `UStaticMeshComponent` | 静态网格 |
| `USkeletalMeshComponent` | 骨骼网格 |
| `UCapsuleComponent` | 胶囊碰撞 |
| `UBoxComponent` | 盒碰撞 |
| `USphereComponent` | 球碰撞 |
| `UCharacterMovementComponent` | 角色移动 |
| `UProjectileMovementComponent` | 抛射物移动 |
| `UCameraComponent` | 相机 |
| `ULightComponent` | 灯光 |
| `UAudioComponent` | 音频 |
| `UParticleSystemComponent` | 粒子 |
| `UNiagaraComponent` | Niagara 粒子 |

### 3.5 关键子系统

| 系统 | 位置 | 作用 |
|------|------|------|
| **Tick 系统** | `FTickTaskManager` | 管理所有 Tick |
| **定时器** | `FTimerManager` | 延迟/循环调用 |
| **碰撞检测** | `FCollisionQuery` | 射线/扫掠检测 |
| **物理** | `FPhysicsInterface` | PhysX/Chaos 抽象 |
| **导航** | `ARecastNavMesh` | AI 寻路 |
| **渲染** | `FScene` | 场景渲染管理 |
| **音频** | `FAudioDevice` | 音频设备管理 |
| **输入** | `UInputComponent` | 输入绑定 |

### 3.6 Actor 生命周期

```cpp
// 完整生命周期顺序
Spawned → PostInitializeComponents → BeginPlay → Tick → EndPlay → BeginDestroy → FinishDestroyed

// 关键函数
virtual void PostInitializeComponents();  // 组件初始化后
virtual void BeginPlay();                 // 游戏开始
virtual void Tick(float DeltaTime);       // 每帧
virtual void EndPlay(const EEndPlayReason::Type);  // 游戏结束
virtual void BeginDestroy();              // 开始销毁
virtual void FinishDestroyed();           // 完成销毁
```

### 3.7 Component 生命周期

```cpp
// 注册/注销
virtual void OnRegister();                // 注册到场景
virtual void OnUnregister();              // 从场景注销

// 激活/停用
virtual void OnActivate();                // 激活
virtual void OnDeactivate();              // 停用

// Tick
virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);
```

---

## 四、三者关系图

```
┌─────────────────────────────────────────┐
│              Your Game Code             │
│         (继承 AActor / UActorComponent)  │
└─────────────────┬───────────────────────┘
                  │ 依赖
┌─────────────────▼───────────────────────┐
│            Engine 模块                   │
│  AActor / UWorld / AGameMode / UCamera  │
│  UCharacterMovement / UInputComponent   │
└─────────────────┬───────────────────────┘
                  │ 依赖
┌─────────────────▼───────────────────────┐
│          CoreUObject 模块                │
│  UObject / UPROPERTY / UFUNCTION / GC   │
│  反射 / 序列化 / CDO / Subobject        │
└─────────────────┬───────────────────────┘
                  │ 依赖
┌─────────────────▼───────────────────────┐
│             Core 模块                    │
│  TArray / FString / FVector / FMath     │
│  Delegate / TSharedPtr / UE_LOG / check │
└─────────────────────────────────────────┘
```

---

## 五、学习建议

1. **先看 Core** — 理解基础类型、容器、委托
2. **再看 CoreUObject** — 理解 UObject、UPROPERTY、GC
3. **最后看 Engine** — 理解 Actor、Component、World
4. **结合 Lyra** — 找 Lyra 里的具体用法对照

## 六、下一步

- [02_插件体系](./02_插件体系.md) — 回到插件总览
- [05_Editor模块详解](./05_Editor模块详解.md) — 编辑器架构
- [07_Lyra中的实际应用](./07_Lyra中的实际应用.md) — Lyra 如何用这些模块
