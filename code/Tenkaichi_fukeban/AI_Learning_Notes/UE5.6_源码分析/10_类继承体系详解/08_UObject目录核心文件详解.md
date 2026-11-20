# UObject 目录核心文件详解

> 路径：`Engine/Source/Runtime/CoreUObject/Public/UObject/`
> 
> 这个目录是 UE5 反射系统的"心脏"，所有 UObject 相关的基础设施都在这里。

---

## 一、总览：文件分类

| 分类 | 文件 | 一句话作用 |
|------|------|-----------|
| **基石** | Object.h / Object.cpp | UObject 本体，一切对象的基类 |
| **宏定义** | ObjectMacros.h | UCLASS/UPROPERTY/UFUNCTION 等宏的展开定义 |
| **类系统** | Class.h | UClass（运行时类型信息）、UStruct、UFunction |
| **字段系统** | Field.h | FField（属性/元数据的基类，非 UObject） |
| **类型系统** | UnrealType.h | FProperty（属性的运行时描述，如 int32/float/对象引用） |
| **GC 系统** | GarbageCollection.h + .cpp | 垃圾回收：标记-清除算法 |
| **对象数组** | UObjectArray.h | GUObjectArray — 全局对象列表 |
| **对象工具** | UObjectGlobals.h | NewObject/SpawnActor 等创建函数 |
| **指针家族** | WeakObjectPtr/StrongObjectPtr/SoftObjectPtr/ObjectPtr | 各种智能指针封装 |
| **序列化** | Linker.h / LinkerLoad.h / SavePackage.h | .uasset 文件的加载和保存 |
| **脚本系统** | Script.h | UFunction 的字节码执行引擎 |
| **包管理** | Package.h | UPackage（一个 .uasset 文件 = 一个 Package） |
| **接口** | Interface.h | UInterface（蓝图可实现的 C++ 接口） |
| **重定向** | CoreRedirects.h | 类名/属性名改名后的兼容映射 |
| **元数据** | MetaData.h | 编辑器中显示的额外信息（如 ToolTip） |
| **迭代器** | UObjectIterator.h | 遍历所有 UObject |
| **引用链搜索** | ReferenceChainSearch.h | 调试用：查某个对象被谁引用着 |
| **集群** | UObjectClusters.h | GC 优化：把一组对象打包一起回收 |
| **标记** | UObjectMarks.h | 临时标记位（如"正在保存""正在加载"） |
| **哈希表** | UObjectHash.h | Name→Object 的快速查找表 |
| **类树** | ClassTree.h | 类继承关系的树形结构 |
| **属性路径** | PropertyPathFunctions.h / FieldPath.h | 通过字符串路径访问属性（如 "Health.MaxHealth"） |
| **构造助手** | ConstructorHelpers.h | 构造函数中安全地加载资源 |
| **NoExportTypes** | NoExportTypes.h | 给蓝图用的 C++ 结构体的反射镜像（FVector 等） |

---

## 二、逐文件详解

### 1. Object.h — UObject 本体

```cpp
class UObject : public UObjectBaseUtility {
    // 核心能力：
    // - 反射（StaticClass()、GetClass()）
    // - 序列化（Serialize()）
    // - GC（AddToRoot()、RemoveFromRoot()、IsPendingKill()）
    // - 名称（GetName()、SetName()、GetFullName()）
    // - 层级（GetOuter()、SetOuter()、GetWorld()）
    // - 标记（HasAnyFlags()、SetFlags()、ClearFlags()）
};
```

**关键方法：**

| 方法 | 作用 |
|------|------|
| `StaticClass()` | 返回此类的 UClass*（编译时由 UHT 自动生成） |
| `GetClass()` | 返回当前实例的实际类型 UClass* |
| `AddToRoot()` | 加入根集，GC 永远不会回收它 |
| `RemoveFromRoot()` | 移出根集，允许 GC 回收 |
| `IsPendingKill()` | 是否正在等待被 GC 销毁 |
| `IsValid()` | 是否有效（存在且未被标记销毁） |
| `Serialize(FArchive& Ar)` | 序列化入口（读写 .uasset） |
| `GetWorld()` | 获取所属的 UWorld（关卡世界） |
| `GetOuter()` | 获取外层对象（谁包含我） |
| `GetFullName()` | 完整路径名（如 "/Game/Maps/Level1.Level1:PersistentLevel.Player"） |
| `Rename()` | 重命名对象 |
| `PostInitProperties()` | 属性初始化后回调（构造函数后调用） |
| `BeginDestroy()` | 开始销毁回调（GC 回收前调用） |

---

### 2. ObjectMacros.h — 宏定义大全

这是 UE5 最重要的头文件之一，定义了所有"魔法宏"：

| 宏 | 作用 |
|----|------|
| `UCLASS(...)` | 声明一个反射类（自动注册到引擎） |
| `USTRUCT(...)` | 声明一个反射结构体 |
| `UENUM(...)` | 声明一个反射枚举 |
| `UFUNCTION(...)` | 声明一个反射函数（蓝图可调用/事件） |
| `UPROPERTY(...)` | 声明一个反射属性（参与 GC、序列化、编辑器显示） |
| `GENERATED_BODY()` | 自动生成构造函数、StaticClass 等样板代码 |
| `DECLARE_DYNAMIC_MULTICAST_DELEGATE` | 动态多播委托（蓝图可绑定） |
| `RF_Public` / `RF_Standalone` / `RF_Transient` | 对象标志位 |

**你写的每一个 `UCLASS()` `UPROPERTY()` 最终都展开成这个文件里的代码。**

---

### 3. Class.h — UClass（运行时类型信息）

```cpp
class UClass : public UStruct {
    // 每个 UClass 存储：
    // - 这个类有哪些属性（ChildProperties 链表）
    // - 这个类有哪些函数（UFunction）
    // - 父类是谁（SuperClass）
    // - 默认对象 CDO（Class Default Object）
    // - 构造函数
};
```

**关键概念：**
- **CDO（Class Default Object）**：每个 UClass 都有一个"模板对象"，NewObject 时复制它来创建新实例
- **Cast<U>**：通过 UClass 做运行时类型检查
- **IsA<U>()**：判断对象是否属于某个类或其子类

---

### 4. Field.h — FField（属性节点）

```cpp
class FField {
    // 注意：FField 不是 UObject！它是轻量级的"属性描述符"
    // 每个 UPROPERTY 在内存中对应一个 FProperty 对象（继承自 FField）
    
    FName Name;           // 属性名
    FField* Next;         // 链表：下一个属性
    EFieldFlags Flags;    // 标志位
};
```

**为什么 FField 不是 UObject？**
因为属性本身不需要 GC 管理、不需要反射——它们是描述 UObject 的"元数据"，越轻越好。

---

### 5. UnrealType.h — FProperty（类型描述）

```cpp
class FProperty : public FField {
    // 描述属性的类型信息：
    // - 大小（GetSize()）
    // - 对齐（GetMinAlignment()）
    // - 默认值（SetValue_InContainer()）
    // - 比较（Identical()）
    // - 拷贝（CopySingleValue()）
};

// 常见子类：
class FBoolProperty   // bool
class FIntProperty    // int32
class FFloatProperty  // float
class FStrProperty    // FString
class FNameProperty   // FName
class FObjectProperty // UObject*（参与 GC 引用追踪）
class FArrayProperty  // TArray
class FMapProperty    // TMap
class FStructProperty // UScriptStruct（如 FVector）
```

**GAS 怎么用这个？**
`FindAttribute()` 就是通过 FName 在 UClass 的属性链表中找对应的 FProperty，然后读取/修改值。

---

### 6. GarbageCollection.h — GC 系统

```cpp
// 核心函数
void CollectGarbage(EObjectFlags KeepFlags);  // 执行一次 GC
void TryCollectGarbage(EObjectFlags KeepFlags);  // 尝试 GC（可能跳过）

// 辅助
bool IsGarbageEliminationEnabled();  // 是否启用 GC
```

**GC 流程：**
1. **标记阶段**：从根集（Root Set）出发，通过 UPROPERTY 引用链遍历所有可达对象，标记为"存活"
2. **清除阶段**：遍历 GUObjectArray，删除未标记的对象

**KeepFlags 参数**：指定哪些标志的对象要保留（如 `RF_NoFlags` = 全部可回收，`RF_RootSet` = 根集保留）

---

### 7. UObjectArray.h — 全局对象列表

```cpp
// 每个 UObject 在这个数组里占一个槽位
struct FUObjectItem {
    UObjectBase* Object;      // 对象指针
    int32 Flags;              // GC 标记位
    int32 RefCount;           // 引用计数
    int32 SerialNumber;       // 唯一序列号
};

// 全局唯一的对象数组
extern FUObjectArray GUObjectArray;

// 关键方法
FUObjectItem* AllocateUObject(int32 Size, bool bAddToHash);  // 分配槽位
void FreeUObject(UObjectBase* Object);                        // 释放槽位
int32 GetObjectIndex(const UObjectBase* Object);              // 查索引
UObjectBase* GetObjectPtr(int32 Index);                       // 按索引取指针
```

**每个 UObject 创建时自动在这里注册，所以你可以按索引快速找到任何对象。**

---

### 8. UObjectGlobals.h — 对象创建函数

```cpp
// 最常用的几个：
template<class T> T* NewObject(UObject* Outer, ...);        // 创建 UObject
template<class T> T* NewObject<U>(UClass* Class, ...);      // 指定类创建
template<class T> T* StaticAllocateObject(...);             // 底层内存分配
```

**NewObject vs SpawnActor：**
- `NewObject<T>()` → 创建普通 UObject（如 AttributeSet、AbilitySet）
- `SpawnActor<T>()` → 创建 AActor 并放入关卡（在 Engine/Engine.h 里）

---

### 9. 指针家族

| 类型 | 作用 | GC 追踪 | 适用场景 |
|------|------|---------|---------|
| `TObjectPtr<T>` | 强引用指针（UE5 推荐替代裸指针） | ✅ 是 | UPROPERTY 成员变量 |
| `TWeakObjectPtr<T>` | 弱引用（不阻止 GC） | ❌ 否 | 不想让对象活着的引用 |
| `TStrongObjectPtr<T>` | 强引用（RAII 风格，析构时自动 RemoveFromRoot） | ✅ 是 | 非 UObject 类持有 UObject |
| `TSoftObjectPtr<T>` | 软引用（存路径，不强制加载） | ❌ 否 | 延迟加载资产 |
| `FLazyObjectPtr<T>` | 懒加载指针（通过 GUID 追踪） | ❌ 否 | 跨关卡引用 Actor |
| `FSoftObjectPath` | 资产路径字符串（如 "/Game/Chars/BP_Hero"） | ❌ 否 | 配置表中引用资产 |

**简单记忆：**
- 要强引用防 GC → `TObjectPtr` 或 `UPROPERTY UObject*`
- 要不阻止 GC → `TWeakObjectPtr`
- 要延迟加载 → `TSoftObjectPtr`

---

### 10. Package.h — UPackage（包）

```cpp
class UPackage : public UObject {
    // 一个 .uasset 文件 = 一个 UPackage
    // 比如 Content/Characters/BP_Hero.uasset → Package "BP_Hero"
    
    // 包含该文件中的所有 UObject（类、默认对象、组件模板等）
};
```

**关键规则：**
- 每个 UObject 都属于某个 Package（通过 GetOutermost() 获取）
- SavePackage() 把一个 Package 及其所有对象写入磁盘
- LoadPackage() 从磁盘加载整个 Package

---

### 11. Linker.h / LinkerLoad.h — 链接器（加载器）

```cpp
class FLinkerLoad : public FLinker {
    // 负责从 .uasset 文件反序列化 UObject
    // 解析文件头、导入表、导出表、依赖关系
};
```

**加载流程：**
1. 读文件头（Magic Number、版本号、CustomVersion）
2. 解析 Import Table（引用了哪些外部资产）
3. 解析 Export Table（本文件导出了哪些对象）
4. 逐个反序列化 Export 中的 UObject

---

### 12. SavePackage.h — 保存器

```cpp
// 核心函数
bool UPackage::Save(SavePackageArgs& SaveArgs);

// 参数包括：
// - 目标文件名
// - 要保存的对象
// - 平台（Windows/Mobile/Console）
// - 是否 Cook（烘焙为平台专用格式）
```

---

### 13. Script.h — 脚本虚拟机

```cpp
// UFunction 的字节码解释器
// 蓝图编译后的指令在这里执行

EX_BytecodeInterpreter {
    // 逐条执行 EX_xxx 操作码
    // 如 EX_LocalVariable、EX_VirtualFunction、EX_Return
}
```

**蓝图 → 编译 → 字节码 → Script.h 解释执行**

---

### 14. Interface.h — UInterface

```cpp
class UInterface : public UStruct {
    // 定义 C++ 接口（蓝图可实现）
    // 用法：
    //   UINTERFACE() class UMyInterface : public UInterface {};
    //   class IMyInterface { virtual void DoSomething() = 0; };
};
```

---

### 15. CoreRedirects.h — 重定向

```cpp
// 当你重命名类/属性时，旧名字 → 新名字的映射
// 存在 DefaultEngine.ini 中：
// [CoreRedirects]
// +ClassRedirects=(OldName="/Script/MyGame.OldClass",NewName="/Script/MyGame.NewClass")
// +PropertyRedirects=(OldName="OldClass.OldProp",NewName="NewClass.NewProp")
```

**作用：** 改名后，旧的 .uasset 文件仍能正确加载（不会丢数据）。

---

### 16. ConstructorHelpers.h — 构造助手

```cpp
// 在构造函数中安全地加载资源（避免加载顺序问题）
ConstructorHelpers::FObjectFinder<UTexture2D> TextureFinder(TEXT("/Game/Textures/T_MyTexture"));
ConstructorHelpers::FClassFinder<AActor> ActorClassFinder(TEXT("/Game/Blueprints/BP_MyActor"));

// 用法：
AMyActor::AMyActor() {
    if (TextureFinder.Succeeded()) {
        StaticMeshComponent->SetMaterial(0, TextureFinder.Object);
    }
}
```

**为什么不用 LoadObject？** 因为构造函数执行时机很早，直接 LoadObject 可能导致其他系统还没初始化完。

---

### 17. UObjectIterator.h — 对象迭代器

```cpp
// 遍历所有 UObject
for (TObjectIterator<UObject> It; It; ++It) {
    UObject* Obj = *It;
    // 处理每个对象
}

// 可以加过滤条件
for (TObjectIterator<ACharacter> It; It; ++It) {
    ACharacter* Char = *It;
    // 只遍历 ACharacter 及其子类
}
```

**用途：** 调试、统计、批量操作（如"找到所有材质并替换"）

---

### 18. ReferenceChainSearch.h — 引用链搜索

```cpp
// 调试工具：找出某个对象被谁引用着（为什么没被 GC？）
FReferenceChainSearch Search(Object, EReferenceChainSearchMode::Shortest);
if (Search.PrintResults()) {
    // 打印从根集到该对象的最短引用路径
}
```

**典型场景：** 内存泄漏排查——"这个对象应该被回收了，为什么还在？"

---

### 19. UObjectClusters.h — GC 集群

```cpp
// 优化 GC 性能：把一组总是同时存活的对象打包
// GC 时整组一起标记/清除，减少遍历时间

bool CanCreateObjectClusters();  // 是否支持集群
```

**原理：** 如果对象 A、B、C 总是同时被引用或同时被释放，就把它们捆成一个 Cluster，GC 只需检查 Cluster 的根引用。

---

### 20. UObjectMarks.h — 对象标记

```cpp
// 临时标记位（不同于 ObjectFlags，marks 不持久化）
enum class EObjectMark : uint8 {
    None = 0,
    // 例如：正在保存、正在加载、客户端不加载、服务器不加载
};
```

**与 ObjectFlags 的区别：**
- **ObjectFlags**（RF_xxx）：持久化，会保存到磁盘
- **Marks**：临时，只在当前函数调用期间有效

---

### 21. UObjectHash.h — 对象哈希表

```cpp
// Name → Object 的快速查找
UObject* FindObject<U>(UObject* Outer, FName Name);     // 按名字找对象
UObject* StaticFindObject(UClass* Class, UObject* Outer, FName Name);
UObject* StaticLoadObject(UClass* Class, UObject* Outer, const TCHAR* Name);  // 加载资产
```

**内部原理：** 哈希表，Key = (Outer, Name)，Value = UObject*

---

### 22. ClassTree.h — 类树

```cpp
// 记录所有 UClass 的继承关系（树形结构）
// 用于：
// - Cast<> 的类型检查
// - 遍历某个类的所有子类
// - 编辑器中的类选择器
```

---

### 23. PropertyPathFunctions.h / FieldPath.h — 属性路径

```cpp
// 通过字符串路径访问嵌套属性
// 如 "HealthComponent.MaxHealth" → 找到 HealthComponent 对象的 MaxHealth 属性

FProperty* FindPropertyByNameAndTypeName(const UStruct* Struct, FName Name, FPropertyTypeName TypeName);
```

**用途：** GAS 中通过 Tag 查找属性、编辑器中显示属性绑定、数据绑定的实现基础。

---

### 24. MetaData.h — 元数据

```cpp
// 存储在对象上的额外信息（不在对象本身中）
// 比如：
// - 编辑器 ToolTip 文字
// - DisplayName（中文显示名）
// - Category（分组）
// - ClampMin/ClampMax（数值范围）

UMetaData* GetMetaData();
FString GetMetaDataTag(UObject* Object, FName Key);
```

**和 UPROPERTY 的关系：** `UPROPERTY(meta=(ToolTip="xxx"))` 中的 meta 就存在这里。

---

### 25. NoExportTypes.h — 蓝图结构体镜像

```cpp
// 给 UHT 解析用的 C++ 结构体反射声明
// 因为 Core 模块不能被 UHT 扫描，所以在这里手动声明

// 包含：FVector、FRotator、FTransform、FColor、FLinearColor 等
```

**为什么叫 NoExport？** 这些类型不导出到蓝图原生代码（Nativization），但蓝图能识别和使用它们。

---

## 三、核心调用关系图

```
你的代码
    │
    ├─ UCLASS() ──────────→ ObjectMacros.h（宏展开）
    │                           ↓
    │                      UHT 生成代码 → GENERATED_BODY()
    │
    ├─ NewObject<T>() ────→ UObjectGlobals.h
    │                           ↓
    │                      StaticAllocateObject() → 分配内存
    │                           ↓
    │                      UObjectBase 构造 → GUObjectArray.AddObject()
    │                           ↓
    │                      注册到全局对象列表（UObjectArray.h）
    │
    ├─ UPROPERTY() ───────→ GC 追踪（GarbageCollection.h）
    │                           ↓
    │                      CollectGarbage() → 标记-清除
    │
    ├─ Serialize() ───────→ LinkerLoad.h（加载）/ SavePackage.h（保存）
    │                           ↓
    │                      FArchive → 二进制读写
    │
    └─ StaticClass() ─────→ Class.h（UClass）
                                ↓
                           包含 FProperty 链表（Field.h → UnrealType.h）
                                ↓
                           反射系统可查询类型信息
```

---

## 四、新手必须记住的 5 个文件

| 优先级 | 文件 | 原因 |
|--------|------|------|
| ⭐⭐⭐ | Object.h | UObject 本体，一切之源 |
| ⭐⭐⭐ | ObjectMacros.h | 所有宏的定义，看懂它就看懂 UE 的"魔法" |
| ⭐⭐⭐ | Class.h | UClass/UStruct/UFunction，反射的核心数据结构 |
| ⭐⭐ | UnrealType.h | FProperty，理解 GAS 如何操作属性 |
| ⭐⭐ | GarbageCollection.h | 理解 GC 如何管理对象生死 |
