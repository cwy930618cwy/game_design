# 01 — CoreUObject：UObject 与反射系统详解

> **定位**：`CoreUObject` 模块 —— UE 的**对象系统**核心。上一文件夹（Core）是"基础工具"，这里是"**游戏对象**"。
>
> **一句话**：`CoreUObject` 提供 **UObject 类**（所有游戏对象的老祖宗）、**反射系统**（让 C++ 能被 UE 工具识别）、**GC**（自动管理对象生命周期）。这是 UE 和普通 C++ 最大的不同。
>
> **文件**：`Engine/Source/Runtime/CoreUObject/`
>
> **对应地图**：`00_引擎全景地图.md` 里 `CoreUObject | UObject 系统：反射、GC、序列化、UClass/UProperty/UFunction`

---

## 一、先搞懂：UObject 是啥（为什么 UE 要有它）

### 1.1 一句话

**UObject 是 UE 所有"游戏对象"的基类**。你的角色、技能、存档、资产……只要想被 UE"认识和管理"，就继承 UObject（或它的子类）。

```
UObject（最老祖宗）
├── UClass（类描述）
├── AActor（场景里的对象）
│   ├── APawn → ACharacter
│   └── ...
├── UActorComponent（组件）
├── UGameInstance、UWorld 等
```

### 1.2 继承 UObject 能得到什么

普通 C++ 类（F 开头）和 UObject（U 开头）的区别：

| 能力 | 普通 C++ 类（FMyData） | UObject（UMyData） |
|------|:---:|:---:|
| 反射（被 UE 工具认识） | ❌ | ✅ |
| GC 自动管理内存 | ❌ | ✅ |
| 序列化（存盘） | ❌ | ✅ |
| 暴露给蓝图 | ❌ | ✅ |
| 网络复制 | ❌ | ✅ |

**这就是为什么**：想让对象被 UE 管理、能被蓝图用、能存档，就继承 UObject。

---

## 二、反射系统（Reflection）—— UObject 的灵魂

### 2.1 什么是反射

**反射 = "程序能描述自己"**。普通 C++ 里，类一旦编译，运行时**不知道自己有哪些属性、哪些方法**。UE 通过反射，让**运行时能查询对象的属性、调用方法**。

```cpp
// 普通 C++：运行时不知道 MyActor 有哪些属性
class MyActor { int32 Health; void Move() {} };
// 你没法在运行时问："MyActor 有哪些属性？"

// UE 反射：可以！
// 蓝图编辑器能显示你的属性、能调用你的方法
// 序列化能自动存你的属性
```

### 2.2 反射是怎么实现的？—— 宏 + UHT 工具

UE 靠**宏**（`UCLASS`/`UPROPERTY`/`UFUNCTION`）标记，再由 **UHT（Unreal Header Tool）**在编译前扫描，**自动生成反射代码**。

```cpp
UCLASS()                                    // ← 标记这是反射类
class UMyCharacter : public UObject {
    GENERATED_BODY()                        // ← 必须加，生成反射代码
public:
    UPROPERTY(BlueprintReadWrite)           // ← 标记属性，可被蓝图读写
    int32 Health;

    UFUNCTION(BlueprintCallable)            // ← 标记函数，可被蓝图调用
    void TakeDamage(float Amount);
};
```

**流程**：
```
写 UCLASS/UPROPERTY/UFUNCTION 宏
        ↓
UHT 工具编译前扫描，生成 .generated.h 反射代码
        ↓
引擎运行时能读取类的信息（属性、方法、枚举）
        ↓
支持蓝图、序列化、GC、网络
```

### 2.3 三个核心宏（必须记）

| 宏 | 作用 | 写在 |
|------|------|------|
| `UCLASS()` | 标记这是反射类 | 类前 |
| `UPROPERTY()` | 标记属性（可序列化/蓝图） | 属性前 |
| `UFUNCTION()` | 标记函数（可蓝图调用/网络） | 函数前 |
| `GENERATED_BODY()` | 生成反射代码（必须有） | 类内部 |

> **注意**：继承 UObject 的类，**必须有 `GENERATED_BODY()`**，否则编译报错。

---

## 三、反射的核心数据结构（UClass / UProperty / UFunction）

### 3.1 UClass —— "类的描述"

`UClass` 描述一个类的**元信息**：它叫什么、继承谁、有哪些属性和方法。

```cpp
// 获取某个类的 UClass
UClass* CharacterClass = UMyCharacter::StaticClass();

// 运行时判断对象是不是某类
if (Actor->IsA<UMyCharacter>()) {   // 是 UMyCharacter 或子类吗
    // ...
}
```

**具体场景：运行时判断对象类型**

```cpp
// 检测撞到的是不是玩家角色
if (OtherActor->IsA<APlayerCharacter>()) {
    // 撞到玩家了
}
```

### 3.2 UProperty / UFunction —— 属性和方法的描述

```cpp
// 遍历一个类的所有属性（反射的力量）
for (TFieldIterator<FProperty> It(MyClass); It; ++It) {
    FProperty* Prop = *It;
    UE_LOG(LogTemp, Log, TEXT("属性名: %s"), *Prop->GetName());
}
```

**具体场景**：通用存档系统，自动序列化所有 UPROPERTY：

```cpp
// 不用手动写每个属性，遍历反射自动存
for (TFieldIterator<FProperty> It(MyClass); It; ++It) {
    // 自动把所有 UPROPERTY 属性写进存档
    Archive << *It;
}
```

---

## 四、GC（垃圾回收）—— 自动管理 UObject 生命周期

### 4.1 回顾（来自 Core 笔记）

UObject 由 **GC** 管理，你不用手动 delete。你之前学的 `TObjectPtr`、`TWeakObjectPtr` 就是配合 GC 用的。

### 4.2 GC 怎么工作

GC **从"根对象"出发**，遍历所有被引用的 UObject，**没被引用的就回收**。

```
根对象（GameInstance 等）
  ↓ 引用
World → Actor → Component → ...
  ↓ 没被引用的对象
被 GC 回收
```

**让 GC 认识你的引用，必须用 UPROPERTY 标记**：

```cpp
UCLASS()
class AMyCharacter : public AActor {
    GENERATED_BODY()
public:
    UPROPERTY()                          // ← 关键！告诉 GC 这个引用要管
    TObjectPtr<AActor> Target;           // GC 会追踪这个引用
};
```

**如果忘了 UPROPERTY**：
```cpp
// ❌ 忘了 UPROPERTY，GC 不知道这个引用，可能误删 Target
AActor* Target;
// ✅ 加 UPROPERTY，GC 才认识
UPROPERTY() TObjectPtr<AActor> Target;
```

### 4.3 NewObject —— 创建 UObject（不用 new）

创建 UObject **不能**用 C++ 的 `new`，要用 `NewObject`：

```cpp
// ❌ 错：UObject 不能用 new（要交给 GC 管）
UMyData* Data = new UMyData();

// ✅ 对：用 NewObject 创建
UMyData* Data = NewObject<UMyData>(this);
```

**具体场景：创建组件/技能对象**

```cpp
// 动态创建一个技能对象
UMySkill* Skill = NewObject<UMySkill>(this, UMySkill::StaticClass());
```

### 4.4 UObject 销毁

```cpp
// UObject 不用 delete，交给 GC
// 或主动标记销毁
Object->MarkPendingKill();   // 老 API，标记销毁
// 新版用条件引用 / 移除引用，让 GC 回收
```

---

## 五、完整示例：一个反射 + GC 的 UObject 类

**场景：一个可被蓝图用的"伤害计算"对象**

```cpp
UCLASS(BlueprintType)                        // 蓝图可创建
class UDamageCalculator : public UObject {   // 继承 UObject
    GENERATED_BODY()                         // 反射代码
public:
    UPROPERTY(BlueprintReadWrite, Category="Damage")  // 蓝图可读写
    float BaseDamage;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<UCharacter> Owner;            // 被 GC 管理

    UFUNCTION(BlueprintCallable, Category="Damage")   // 蓝图可调用
    float CalculateDamage(float Amount) {
        return BaseDamage + Amount;
    }
};

// 使用（创建用 NewObject）
UDamageCalculator* Calc = NewObject<UDamageCalculator>(this);
Calc->BaseDamage = 10.f;
float Final = Calc->CalculateDamage(5.f);   // 15
```

---

## 六、常见陷阱

**① 忘了 GENERATED_BODY()**
```cpp
// ❌ 继承 UObject 没加 GENERATED_BODY，编译错
UCLASS() class UMyData : public UObject { };   // 缺 GENERATED_BODY()
// ✅ 必须有
UCLASS() class UMyData : public UObject { GENERATED_BODY() };
```

**② UObject 用 new 创建**
```cpp
// ❌ UMyData* D = new UMyData();
// ✅ NewObject<UMyData>(this)
```

**③ UObject 成员忘了 UPROPERTY**
```cpp
// ❌ 没 UPROPERTY，GC 不管，可能悬垂
AActor* Target;
// ✅ UPROPERTY() TObjectPtr<AActor> Target;
```

**④ 用 delete 删 UObject**
```cpp
// ❌ delete UObject 会崩，归 GC 管
delete SomeUObject;
// ✅ 交给 GC / 移除引用
```

---

## 七、总结速查

```
CoreUObject = UObject 系统
├── UObject：所有游戏对象的基类
├── 反射：UCLASS/UPROPERTY/UFUNCTION 宏 + UHT 生成代码
├── UClass/UProperty/UFunction：运行时描述类/属性/方法
├── GC：自动管理 UObject 生命周期
└── NewObject：创建 UObject（不能用 new）

四个必背：
  UCLASS() 标记反射类
  UPROPERTY() 标记属性
  UFUNCTION() 标记函数
  GENERATED_BODY() 必须有

核心认识：
  想让对象被 UE 管 → 继承 UObject
  继承 UObject → 用 NewObject 创建，交给 GC，成员加 UPROPERTY
```

**一句话**：`CoreUObject` 是 UE 的对象系统。**继承 UObject 的对象能被反射（被工具认识）、被 GC 管、被序列化、被蓝图用**。写 UObject 类必须记：`UCLASS` + `GENERATED_BODY()`，属性加 `UPROPERTY()`，创建用 `NewObject()`，别用 new/delete。

---

## 八、CoreUObject/Public 全目录速查（实际清单）

> 这是 `Engine/Source/Runtime/CoreUObject/Public/` 下**真实存在的全部子目录**，按"干什么的"分类。

```
CoreUObject/Public/
├── UObject/               ← ★ 核心（最大的目录，UObject 系统主体）
├── AssetRegistry/         ← 资产注册表（扫描/索引所有资产）
├── Blueprint/             ← 蓝图支持（蓝图相关的 UObject 类型）
├── Serialization/         ← 序列化（UObject 的存盘/加载）
├── Misc/                  ← 杂项工具
├── Templates/             ← 模板工具（智能对象指针等）
├── StructUtils/           ← 结构体工具
├── Concepts/              ← 模板约束概念
├── Cooker/                ← 烘焙（打包工具）
├── Internationalization/  ← 本地化
└── VerseVM/               ← Verse 语言虚拟机（实验性）
```

### 8.1 UObject/（核心，最重要，170+ 文件）

这是 CoreUObject 的**主体**，UObject 系统全在这。文件很多（170+），按功能分类：

**① 类 / 反射系统（对象和类的描述）**

| 文件 | 作用 |
|------|------|
| `Object.h` | **UObject 基类定义**（所有游戏对象的老祖宗） |
| `UObjectBase.h` | UObject 最底层基类（对象名、类指针） |
| `Class.h` | **UClass**（类的描述） |
| `Field.h` | **UField 字段系统**（UClass/UFunction/UProperty 基类） |
| `Interface.h` | UInterface（接口基类） |
| `UnrealType.h` | **FProperty**（属性类型系统，最重要的反射类型） |
| `Script.h` | UFunction（蓝图虚拟机） |

**② 宏定义（写代码必用）**

| 文件 | 作用 |
|------|------|
| `ObjectMacros.h` | **UCLASS/UPROPERTY/UFUNCTION/GENERATED_BODY 宏** |
| `ScriptMacros.h` | 脚本相关宏 |
| `GeneratedCppIncludes.h` | 生成的 .generated.h 包含 |

**③ GC 垃圾回收**

| 文件 | 作用 |
|------|------|
| `GarbageCollection.h` | **GC 实现**（标记清除算法） |
| `GCObject.h` | 非 UObject 也能被 GC 追踪（FRootObject） |
| `GCObjectScopeGuard.h` | GC 作用域保护 |
| `FastReferenceCollector.h` | 快速引用收集器（GC 加速） |

**④ UObject 的各种指针（衔接 Core 学的智能指针）**

| 文件 | 作用 |
|------|------|
| `WeakObjectPtr.h` | **TWeakObjectPtr**（弱引用，已学） |
| `WeakObjectPtrFwd.h` | TWeakObjectPtr 前向声明 |
| `SoftObjectPtr.h` | **TSoftObjectPtr**（软引用资产，不加载） |
| `LazyObjectPtr.h` | TLazyObjectPtr（延迟加载对象） |
| `StrongObjectPtr.h` | TStrongObjectPtr（强引用，脱离 GC 追踪） |
| `ObjectPtr.h` | TObjectPtr（UE5 的对象指针） |

**⑤ 创建 / 全局函数**

| 文件 | 作用 |
|------|------|
| `UObjectGlobals.h` | **NewObject / CreateDefaultSubobject 等** |
| `UObjectAllocator.h` | UObject 内存分配 |
| `UObjectHash.h` | 对象哈希查找 |
| `ConstructorHelpers.h` | 构造函数里查找资产（FObjectFinder） |

**⑥ Package / 资产（存盘加载）**

| 文件 | 作用 |
|------|------|
| `Package.h` | UPackage（资产的容器） |
| `SavePackage.h` | 保存资产 |
| `LinkerLoad.h` / `LinkerSave.h` | 加载/保存链路 |
| `TopLevelAssetPath.h` | 资产顶层路径 |

**⑦ 其他工具**

| 文件 | 作用 |
|------|------|
| `UObjectIterator.h` | 遍历所有 UObject |
| `UObjectMarks.h` | 对象标记 |
| `ObjectKey.h` / `ObjectRef.h` | 对象引用工具 |
| `ScriptInterface.h` | 接口智能指针 |
| `PropertyTag.h` / `PropertyText.h` | 属性序列化 |

> **重点掌握**：`Object.h`（UObject 基类）、`Class.h`（UClass）、`UnrealType.h`（FProperty）、`ObjectMacros.h`（宏）、`GarbageCollection.h`（GC）、`UObjectGlobals.h`（NewObject）。**这些是你学 CoreUObject 的核心。**

### 8.2 AssetRegistry/ —— 资产注册表

**扫描并索引所有资产**，让你能按名字/类型快速查资产。

```cpp
// 查找某类所有资产（反射 + 资产注册表）
TArray<FAssetData> Assets;
UAssetRegistryHelpers::GetAssetRegistry().GetAssetsByClass(TEXT("StaticMesh"), Assets);
```

**场景**：运行时按类型找资产、内容浏览器显示资产列表。**进阶功能。**

### 8.3 Blueprint/ —— 蓝图支持

蓝图运行时相关的 UObject 类型。**一般写 C++ 不直接碰**，做蓝图系统才深入。

### 8.4 Serialization/ —— 序列化

UObject 的存盘/加载（配合你 Core 学的 FArchive）。做存档系统用。

### 8.5 Misc/ —— 杂项

各种工具，如断言、错误处理。**遇到再看。**

### 8.6 Templates/ —— 模板工具

如 `TStrongObjectPtr`（强引用智能指针）、`TWeakObjectPtr` 实现。**进阶。**

### 8.7 其他（了解即可）

| 目录 | 作用 |
|------|------|
| `StructUtils/` | 结构体工具 |
| `Concepts/` | C++ 模板约束概念 |
| `Cooker/` | 烘焙/打包工具 |
| `Internationalization/` | 本地化 |
| `VerseVM/` | Verse 语言虚拟机（实验性，不用管） |

### 8.7.5 全目录完整清单（11 个逐个说明）

> 把 11 个目录**全部**列出来，逐个说明干嘛的、要不要学。

| 目录 | 干嘛的 | 要学吗 |
|------|--------|:---:|
| **UObject/** | UObject 系统主体（反射/GC/指针/宏） | ✅ **重点** |
| **Serialization/** | UObject 存盘/加载（ArchiveUObject/BulkData） | ⭐ 做存档再学 |
| **Templates/** | 模板工具（TSubclassOf/Cast） | ⭐ 进阶 |
| **AssetRegistry/** | 资产注册表（扫描/索引所有资产） | ⭐ 进阶 |
| **Misc/** | 杂项工具 | ❌ 遇再看 |
| **Blueprint/** | 蓝图支持（BlueprintSupport.h） | ❌ 一般不碰 |
| **StructUtils/** | 结构体工具 | ❌ 少用 |
| **Concepts/** | C++ 概念（模板约束） | ❌ 不用学 |
| **Cooker/** | 烘焙/打包工具 | ❌ 不用学 |
| **Internationalization/** | 本地化 | ❌ 做多语言再学 |
| **VerseVM/** | Verse 虚拟机（实验性） | ❌ 不用管 |

**重点总结**：
- **重点只学 `UObject/`**（对象系统的家：反射/GC/指针/宏）
- 进阶可学：**Serialization**（存档）、**Templates**（TSubclassOf）、**AssetRegistry**（资产管理）
- 其余（Blueprint/Cooker/VerseVM/Concepts/StructUtils/Misc/Internationalization）**都不用学**

### 8.8 学习建议

| 主题 | 要学吗 | 建议 |
|------|:---:|------|
| UObject/ 的类/反射/GC | ✅ 重点 | 已学，继续深入 |
| UObject/ 的指针（Weak/Soft/Lazy） | ⭐ 常碰 | 你之前学的 TWeakObjectPtr 在这 |
| AssetRegistry | ⭐ 进阶 | 做资产管理再学 |
| Serialization | ⭐ 做存档 | 用到再学 |
| 其他（Blueprint/Cooker/VerseVM） | ❌ 不用管 | 用不到 |

**一句话**：CoreUObject 的核心在 **`UObject/`**（UObject 类、UClass、反射、GC、各种指针）。AssetRegistry（资产注册表）和 Serialization（序列化）是进阶。其余（Blueprint/Cooker/VerseVM）一般不用碰。

---

## 九、下一步

UObject 学完，下一个自然是 **Engine 模块**（`AActor`、`ACharacter`、组件、GameMode）——那是真正的"游戏世界"对象。但先把这个 CoreUObject 的 UObject/反射/GC 吃透，它是理解一切的基础。
