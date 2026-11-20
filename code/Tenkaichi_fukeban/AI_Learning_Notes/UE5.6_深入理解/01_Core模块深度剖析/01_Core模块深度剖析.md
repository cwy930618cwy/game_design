# 01 — Core 模块深度剖析

> **路径**：`Engine/Source/Runtime/Core/`
> 
> **定位**：整个 UE5 引擎的"地基"。没有 Core，其他一切（UObject、渲染、物理、网络）都无法运行。
> 
> **核心原则**：Core 不依赖任何 UE 模块，只依赖 C++ 标准库 + 平台 API。它是唯一可以 `#include <iostream>` 的地方。

---

## 一、目录结构

```
Runtime/Core/
├── Public/                     ← 头文件（你 #include 的东西）
│   ├── ★ 核心基础
│   │   ├── Containers/         ← 容器：TArray、TMap、TSet、FString、TSharedPtr
│   │   ├── Math/               ← 数学：FVector、FRotator、FMath、FTransform
│   │   ├── Templates/          ← 模板工具：MoveTemp、类型特征、Tuple、智能指针
│   │   ├── Delegates/          ← 委托：单播/多播/动态委托
│   │   └── UObject/            ← FName 名字系统（NameTypes.h）
│   ├── ⭐ 底层系统
│   │   ├── HAL/                ← 硬件抽象层：线程、锁、文件、内存接口
│   │   ├── Serialization/      ← 序列化：FArchive、内存读写
│   │   ├── Async/              ← 异步：Future、Promise、TaskGraph
│   │   ├── Tasks/              ← 任务系统：UE 的 Task 系统
│   │   ├── IO/                 ← IO 操作：异步文件/流读取
│   │   ├── Memory/             ← 内存管理：内存池、分配器
│   │   ├── String/             ← 字符串处理：字符转换、字符串算法
│   │   ├── Hash/               ← 哈希：哈希函数、散列
│   │   ├── Traits/             ← 类型特征：编译期类型判断
│   │   ├── Concepts/           ← C++ 概念：模板约束
│   │   ├── Compression/        ← 压缩：LZ4、Oodle
│   │   ├── Stats/              ← 统计：游戏统计、性能计数
│   │   ├── ProfilingDebugging/ ← 性能分析：Profile 标记、调试工具
│   │   ├── Logging/            ← 日志：Log.h、LogCategory.h
│   │   └── Misc/               ← 杂项：断言、配置、命令行（最大目录）
│   ├── 🔧 平台相关
│   │   ├── GenericPlatform/    ← 通用平台基类（跨平台代码）
│   │   ├── Windows/ Microsoft/ MSVC/      ← Windows 平台
│   │   ├── Linux/ Unix/ Clang/            ← Linux 平台
│   │   ├── Mac/ Apple/                   ← Mac 平台
│   │   ├── IOS/ Android/                 ← 移动平台
│   │   └── IntelICX/           ← Intel 编译器
│   ├── 📦 其他
│   │   ├── Algo/               ← 算法库：Sort、BinarySearch（类似 std::algorithm）
│   │   ├── Audio/              ← 音频基础类型
│   │   ├── ColorManagement/    ← 颜色管理：色彩空间
│   │   ├── Internationalization/ ← 本地化：多语言、Text 系统
│   │   ├── Modules/            ← 模块系统：FModuleManager
│   │   ├── Features/           ← 特性开关
│   │   ├── FileCache/          ← 文件缓存
│   │   ├── Experimental/       ← 实验性：ConcurrentLinearAllocator
│   │   ├── FramePro/ MemPro/   ← 第三方性能分析工具
│   │   ├── Sanitizer/          ← 内存检查：ASan/UBSan
│   │   ├── AutoRTFM/           ← 事务内存：实验性原子内存
│   │   ├── Virtualization/     ← 资产虚拟化
│   │   └── Tests/              ← 单元测试基础
│   └── 根目录头文件
│       ├── Core.h / CoreMinimal.h / CoreFwd.h / CoreTypes.h
│       └── CoreGlobals.h / CoreSharedPCH.h / PixelFormat.h
├── Private/                     ← 实现文件（.cpp）
└── Core.Build.cs                ← 模块构建配置
```

> 完整版见文末"七、Core/Public 全目录速查"。日常开发真正高频的只有 **Containers/Math/Delegates/Templates**，平台目录和底层工具（Algo/Stats/Hash/IO...）不用主动学。

---

## 二、六大核心子系统

### 2.1 TArray — 动态数组（最常用）

**文件**：`Public/Containers/Array.h`（约 4000 行）

#### 内存布局

```cpp
template<typename InElementType, typename InAllocatorType>
class TArray {
private:
    ElementAllocatorType AllocatorInstance;  // 内存分配器（管理实际数据指针）
    SizeType             ArrayNum;           // 当前元素个数
    SizeType             ArrayMax;           // 已分配容量（>= ArrayNum）
};
```

**图解：**

```
AllocatorInstance → [Elem0][Elem1][Elem2][  ][  ][  ]
                     ↑      ↑      ↑     ↑    ↑    ↑
                   已用元素              空闲空间（Slack）
                   
ArrayNum = 3（有 3 个元素）
ArrayMax = 6（总共能装 6 个）
```

#### 关键方法源码解析

**① 构造函数**

```cpp
FORCEINLINE TArray()
    : ArrayNum(0)                              // 初始 0 个元素
    , ArrayMax(AllocatorInstance.GetInitialCapacity())  // 容量由分配器决定
{
}
```

**② operator[] — 下标访问**

```cpp
FORCEINLINE ElementType& operator[](SizeType Index) {
    RangeCheck(Index);       // 越界检查（Debug 模式下会断言）
    return GetData()[Index]; // 直接返回引用，O(1)
}
```

**③ Add — 添加元素**

```cpp
SizeType Add(const ElementType& Item) {
    CheckAddress(&Item);     // 防止自我引用导致的数据损坏
    return Emplace(Item);    // 转发到 Emplace
}

// Emplace 内部逻辑（简化）：
SizeType Emplace(ElementType&& Item) {
    if (ArrayNum == ArrayMax) {
        // 容量满了 → 重新分配更大的内存（通常 1.5x 增长）
        Grow();
    }
    // 在末尾构造新元素（placement new）
    ::new((void*)GetAllocation() + ArrayNum * sizeof(ElementType)) 
        ElementType(Forward<Item>(Item));
    ArrayNum++;
    return ArrayNum - 1;
}
```

**④ Empty — 清空**

```cpp
void Empty(SizeType Slack = 0) {
    DestructItems(GetData(), ArrayNum);  // 调用每个元素的析构函数
    ArrayNum = 0;
    // 如果 Slack > 0，保留一定容量；否则释放全部内存
}
```

#### 性能要点

| 操作 | 时间复杂度 | 说明 |
|------|-----------|------|
| `operator[]` | O(1) | 直接指针偏移 |
| `Add()` | O(1)* | *均摊，偶尔需要 realloc |
| `RemoveAt(i)` | O(n) | 后面所有元素前移 |
| `RemoveAtSwap(i)` | O(1) | 交换到末尾再删（破坏顺序） |
| `Find(Item)` | O(n) | 线性查找 |
| `Empty()` | O(n) | 逐个析构 |

#### TArray vs std::vector

| 特性 | TArray | std::vector |
|------|--------|-------------|
| 内存分配器 | 可定制（FDefaultAllocator / FInlineAllocator） | std::allocator |
| 元素类型限制 | 必须支持 `GetTypeLayout()`（反射友好） | 无限制 |
| 调试支持 | UE_LOG 集成、内存标记 | 无 |
| 序列化 | 内置 Serialize() 支持 | 无 |
| 迭代器失效 | 和 vector 一样 | 一样 |

---

### 2.2 FString — 字符串

**文件**：`Public/Containers/UnrealString.h`

#### 为什么不用 std::string？

1. **Unicode 优先**：FString 内部用 `TCHAR`（Windows 上是 `wchar_t`，即 UTF-16）
2. **反射集成**：可以作为 UPROPERTY 参与 GC 和序列化
3. **UE 生态一致**：所有 UE API 都返回 FString

#### 核心结构

```cpp
class FString {
protected:
    TArray<TCHAR> Data;  // 实际字符存储（含结尾 \0）
};
```

**是的，FString 就是 TArray<TCHAR> 的封装！**

#### 常见操作

```cpp
FString Name = TEXT("Hello World");  // TEXT() 宏确保宽字符
Name.Len();                           // 长度（不含 \0）
Name.IsEmpty();                       // 是否为空
Name.Contains(TEXT("World"));         // 包含子串
Name.StartsWith(TEXT("Hello"));       // 前缀匹配
Name.Split(TEXT(" "), &Before, &After); // 分割
Name.Replace(TEXT("World"), TEXT("UE5")); // 替换
Name.TrimStartAndEnd();               // 去首尾空白
FString Num = FString::FromInt(42);   // int → FString
int32 Val = FCString::Atoi(*Name);    // FString → int（通过 * 取原始指针）
```

#### FName vs FString vs FText

| 类型 | 用途 | 特点 |
|------|------|------|
| **FName** | 标识符（资源名、Tag、属性名） | 不可变、哈希比较、全局唯一、不能改 |
| **FString** | 通用字符串操作 | 可变、支持拼接/查找/替换 |
| **FText** | 显示给玩家的文本 | 支持本地化翻译、格式化 |

**转换关系：**
```
FName ──ToString()──→ FString ──ToText()──→ FText
FText ──ToString()──→ FString ──ToString()─→ FName
```

---

### 2.3 TMap — 键值对映射

**文件**：`Public/Containers/Map.h`

#### 底层实现

```cpp
template<typename KeyType, typename ValueType, ...>
class TMap {
private:
    TPairSortedMap<KeyType, ValueType, ...> SortedMap;
    // 内部基于 TSet<TPair<Key, Value>> 实现
    // 使用哈希表 + 有序遍历
};
```

#### 常用操作

```cpp
TMap<FName, int32> HealthMap;
HealthMap.Add(TEXT("MaxHealth"), 100);     // 插入
int32* Found = HealthMap.Find(TEXT("MaxHealth")); // 查找（返回指针，未找到返回 nullptr）
HealthMap.Remove(TEXT("MaxHealth"));       // 删除
HealthMap.Contains(TEXT("MaxHealth"));     // 是否包含
for (auto& Pair : HealthMap) {             // 遍历
    UE_LOG(LogTemp, Log, TEXT("%s = %d"), *Pair.Key.ToString(), Pair.Value);
}
```

---

### 2.4 委托系统（Delegate）

**文件**：`Public/Delegates/`（`Delegate.h` / `MulticastDelegate.h` / `DynamicDelegate.h`）

#### 什么是委托（一句话）

委托就是 UE 的 **"类型安全、可绑定多个"的函数指针**。传统 C++ 用函数指针 + `std::function` 做回调，UE 用委托做，好处是：

- **类型安全**：签名不匹配编译期直接报错
- **与 UObject 生命周期集成**：`AddUObject` 绑定时，对象销毁后委托能感知（弱引用），避免悬垂指针
- **支持绑定成员函数、lambda、裸函数、静态函数**
- **动态委托可序列化、可暴露给蓝图**

---

#### 一、目录里有什么（Delegates/）

```
Public/Delegates/
├── Delegate.h              ← 单播委托（TDelegate）
├── MulticastDelegate.h     ← 多播委托（TMulticastDelegate）
├── DynamicDelegate.h       ← 动态委托（蓝图/序列化用）
├── DelegateBase.h          ← 两种委托的公共基类（类型擦除存储）
├── DelegateSignatureImpl.inl  ← 宏展开后的具体实现（模板）
├── MulticastDelegateBase.h ← 多播委托的基类
└── ...（各种 .inl 实现文件）
```

**核心关系**：`Delegate.h` 和 `MulticastDelegate.h` 是"普通委托"（编译期、不可序列化），`DynamicDelegate.h` 是"动态委托"（运行期、可反射）。它们各自都有单播/多播两个变体。

---

#### 二、四种委托总览（先看这张表）

| 类型 | 声明宏 | 能否多绑 | 能否被蓝图用 | 能否序列化 | 使用场景 |
|------|--------|:------:|:----------:|:--------:|---------|
| **单播委托** | `DECLARE_DELEGATE[_XParam]` | ❌ 只能绑1个 | ❌ | ❌ | C++ 内部回调、事件处理 |
| **多播委托** | `DECLARE_MULTICAST_DELEGATE[_XParam]` | ✅ 可绑多个 | ❌ | ❌ | C++ 广播通知（最常用） |
| **动态单播** | `DECLARE_DYNAMIC_DELEGATE[_XParam]` | ❌ | ✅ | ✅ | 需要暴露给蓝图/保存 |
| **动态多播** | `DECLARE_DYNAMIC_MULTICAST_DELEGATE[_XParam]` | ✅ | ✅ | ✅ | 蓝图事件分发器（如 OnHit） |

> `_XParam` 表示参数个数，如 `DECLARE_DELEGATE_OneParam`（1个参）、`DECLARE_DELEGATE_TwoParams`（2个参）。

**选型口诀**：

```
需要暴露给蓝图 或 需要序列化保存？
├─ 要 → 动态委托（DECLARE_DYNAMIC_*）
└─ 不要 → 只绑一个还是多个？
          ├─ 一个 → 单播委托（DECLARE_DELEGATE，Execute）
          └─ 多个 → 多播委托（DECLARE_MULTICAST_DELEGATE，Broadcast）
```

#### 选型判断表（先问 3 个问题）

选委托类型只问自己三件事：

1. **要不要暴露给蓝图？** → 要就用动态（第 4 类）
2. **要绑多个还是一个？** → 多个用多播（第 2/4 类）
3. **要不要保存/序列化？** → 要就用动态（第 4 类）

```
事件要不要暴露给蓝图？
│
├── 不要 → 绑一个还是多个？
│          ├─ 一个 → 单播委托  例：查询冷却是否就绪
│          └─ 多个 → 多播委托  例：掉血通知血条/飘字/音效
│
└── 要 → 绑一个还是多个？
         ├─ 一个 → 动态单播  例：蓝图定制关卡进入逻辑
         └─ 多个 → 动态多播  例：死亡通知蓝图演出+C++掉落
```

**容易混淆的是「单播 vs 多播」（都 C++）和「动态单播 vs 动态多播」（都动态），区别只在「绑一个还是绑多个」。记住「查询用单播，通知用多播」就不会选错。**

---

#### 四种委托的具体场景（重要，务必吃透）

##### ① 单播委托 → 一问一答（只绑一个 C++ 回调）

**场景：技能系统 —— 「技能准备就绪后，查询冷却时间」**

技能对象想知道"这个技能还能不能用"，只有**一个**系统需要这个回调（如 HUD 冷却图标），且**不需要给蓝图**。

```cpp
// 声明
DECLARE_DELEGATE_RetVal(bool, FQuerySkillReady);

class USkill {
public:
    FQuerySkillReady OnQueryReady;   // 单播：只绑一个

    bool IsReady() {
        // 有绑定就执行（返回是否就绪），没绑定默认不可用
        return OnQueryReady.IsBound() ? OnQueryReady.Execute() : false;
    }
};

// 某处绑定（只绑了 HUD 这一个）
Skill->OnQueryReady.BindUObject(HudComp, &UCooldownWidget::CheckCooldown);
```

**为什么用单播**：查询类回调本质是"**一问一答**"，只需要一个回答者，多个回答者会造成混乱。

---

##### ② 多播委托 → 广播给所有人（绑多个 C++ 回调，最常用）

**场景：伤害系统 —— 「角色掉血，血条/飘字/音效都更新」**

角色受伤，血条、伤害飘字、连击判定、音效都要**同时响应**，且都不给蓝图。

```cpp
// 声明
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDamaged, float, AActor*);

class ACharacter {
public:
    FOnDamaged OnDamaged;      // 多播：可以绑很多个

    void TakeDamage(float Amount, AActor* Causer) {
        OnDamaged.Broadcast(Amount, Causer);  // 一次性通知所有绑定者
    }
};

// 多个系统各自绑定
Char->OnDamaged.AddUObject(HpBar,    &UHPBar::UpdateValue);    // 血条
Char->OnDamaged.AddUObject(DamageUI, &UDamageNum::ShowNumber); // 伤害飘字
Char->OnDamaged.AddUObject(Combo,    &UComboMgr::BreakCombo);  // 连击中断
```

**为什么用多播**：这是"**通知**"而非"查询"，一个事件有多个听众，多播让它们**全部收到**。

---

##### ③ 动态单播 → 蓝图专属单发（暴露给蓝图 + 可保存）

**场景：关卡系统 —— 「玩家进入关卡时，蓝图定制一次初始化」**

关卡蓝图需要**自己决定**进入关卡后执行什么逻辑，且这个绑定要能**保存进关卡资产**。

```cpp
// 声明
DECLARE_DYNAMIC_DELEGATE(FOnLevelEntered);

UCLASS()
class ALevelDirector : public AActor {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable)
    FOnLevelEntered OnLevelEntered;   // 动态单播：蓝图可绑，可保存

    void EnterLevel() {
        OnLevelEntered.ExecuteIfBound();  // 蓝图绑定的逻辑会执行
    }
};
```

**为什么用动态单播**：蓝图只能绑动态委托（普通委托蓝图不认识），且 `UPROPERTY` 能把它**保存进关卡**资产，普通单播做不到。

---

##### ④ 动态多播 → 蓝图+C++ 都收（引擎 OnHit 同款，最典型）

**场景：角色死亡 —— 「死亡时，蓝图和 C++ 都要收到通知」**

C++ 要处理掉落，蓝图要做死亡演出，**都绑到同一个事件**上。引擎自带的 `OnHit`、`OnLanded`、`OnComponentBeginOverlap` 全是这种。

```cpp
// 声明（引擎自带的就是这种）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Victim);

UCLASS(Blueprintable)
class ACharacter : public ACharacter {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable, Category="Event")  // ← 关键：蓝图可绑
    FOnDeath OnDeath;

    void Die() {
        OnDeath.Broadcast(this);  // C++ 和蓝图绑定的全都会收到
    }
};
```

**为什么用动态多播**：**既要给蓝图用、又要绑多个**，引擎所有"事件"都用它。

---

#### 三、单播委托（DECLARE_DELEGATE）

只能绑**一个**函数，用 `Execute()` 调用。

```cpp
// ① 声明（在头文件里）
DECLARE_DELEGATE_OneParam(FOnHealthChanged, float);

// ② 使用
class AMyCharacter {
public:
    FOnHealthChanged OnHealthChanged;      // 委托成员

    void SetHealth(float NewHealth) {
        OnHealthChanged.ExecuteIfBound(NewHealth); // 触发，没绑就跳过
    }
};

// ③ 绑定
Character->OnHealthChanged.BindUObject(this, &AMyUI::UpdateHealthBar);
// Character->OnHealthChanged.BindRaw(...);   // 绑定裸 C++ 对象
// Character->OnHealthChanged.BindLambda(...); // 绑定 lambda
```

| 方法 | 作用 |
|------|------|
| `Bind*` | 绑定一个函数（只能绑一个，再绑会覆盖） |
| `Execute(参数)` | 调用绑定的函数（**未绑定时调用会崩溃**） |
| `ExecuteIfBound(参数)` | 安全调用，未绑定就跳过 |
| `Unbind()` | 解除绑定 |

**注意**：单播委托用 `Execute` 前**一定要 `IsBound()` 检查或直接用 `ExecuteIfBound`**，否则崩溃。

---

#### 四、多播委托（DECLARE_MULTICAST_DELEGATE）★ 最常用

能绑**多个**函数，用 `Broadcast()` 依次全部调用。**UI 事件、伤害通知、技能触发**基本都是它。

```cpp
// ① 声明
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, FVector);

// ② 类中持有并广播
class AMyCharacter : public ACharacter {
public:
    FOnDamageTaken OnDamageTaken;

    void TakeDamage(float Amount, FVector Location) {
        OnDamageTaken.Broadcast(Amount, Location); // 广播给所有绑定者
    }
};

// ③ 外部多路绑定
Character->OnDamageTaken.AddUObject(this, &AMyUI::ShowDamageNumber);
Character->OnDamageTaken.AddUObject(this, &ASoundManager::PlayHitSound);
Character->OnDamageTaken.AddLambda([](float Amount, FVector Loc) {
    UE_LOG(LogTemp, Warning, TEXT("受伤 %.1f 位置 %s"), Amount, *Loc.ToString());
});
```

| 方法 | 作用 |
|------|------|
| `AddUObject` | 绑定 UObject 成员函数（弱引用，对象销毁自动移除） |
| `AddRaw` | 绑定裸 C++ 对象（**不管理生命周期**，对象销毁前必须手动 Remove，否则悬垂崩溃） |
| `AddLambda` | 绑定 lambda（捕获的对象生命周期由你自己负责） |
| `Broadcast(参数)` | 依次调用所有绑定者，**无绑定时安全不崩** |
| `Clear()` | 清空所有绑定 |
| `IsBound()` | 是否有绑定 |

> ⚠️ **安全红线**：`AddRaw` 和捕获裸指针的 `AddLambda` 是悬垂崩溃两大来源。如果被绑对象可能先于委托销毁，必须用 `Remove*` 主动解绑，或改用 `AddUObject`。

---

#### 五、动态委托（DECLARE_DYNAMIC_*）

动态委托的核心差异：**基于 UObject 反射系统**，可以被 UPROPERTY 标记、序列化保存、暴露给蓝图。

```cpp
// 动态单播（可保存、可蓝图绑定，但只能绑1个）
DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FCanUseSkill);

// 动态多播（蓝图事件分发器，最典型）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeath, AActor*, AController*);

UCLASS()
class AMyCharacter : public ACharacter {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable)      // ← 关键：允许蓝图绑定
    FOnDeath OnDeath;
};
```

**动态委托的限制**：

- **性能更差**：走反射，比普通委托慢
- **只能绑定蓝图函数 或 UObject 上的 UFUNCTION**，不能绑 lambda / 裸 C++ 函数
- **参数类型有限制**：必须是反射系统能处理的类型（不能是 `TArray<int>` 这种组合类型，但支持 `TArray<FString>` 等部分组合）

---

#### 六、委托的执行模型（原理）

```
单播委托：
  Delegate.Bind(this, &AMyClass::OnEvent);
  Delegate.Execute(Param);  → 调用 AMyClass::OnEvent(Param)

多播委托：
  Delegate.AddUObject(this, &AMyClass::OnEvent);
  Delegate.AddLambda(...);
  Delegate.Broadcast(Param); → 依次调用所有绑定的函数
```

**内部存储**：委托本质上存一个 **函数指针 + 可选对象指针**（`TBaseDelegate` 用类型擦除把各种签名统一存储，`DelegateBase.h` 负责）。多播委托内部维护一个 **TArray<TDelegate>**，`Broadcast` 就是遍历这个数组逐个调用。

**绑定列表的合法性**：多播委托内部在调用时会检查每个绑定项是否仍有效（`IsCompactValid` / `CompactInvalidate`），所以对象销毁后 `AddUObject` 绑的项会被自动清理——这就是它比裸函数指针安全的原因。

---

#### 七、常见陷阱

**① 单播委托忘了判断就 Execute → 崩溃**
```cpp
// ❌ 没绑定就 Execute 直接崩
OnEvent.Execute(1);

// ✅ 安全写法
if (OnEvent.IsBound()) OnEvent.Execute(1);
// 或
OnEvent.ExecuteIfBound(1);
```

**② 被绑对象销毁，委托还在 → 悬垂崩溃**
```cpp
// ❌ AddRaw 绑了裸对象，对象 delete 后委托还留着 → 调用就崩
SomeDelegate.AddRaw(&obj, &MyObj::OnEvent);
delete &obj;

// ✅ 用 AddUObject 自动清理，或对象销毁前手动 RemoveRaw
SomeDelegate.AddUObject(&obj, &MyObj::OnEvent);
```

**③ 动态委托在 C++ 里绑定 lambda → 编译失败**
```cpp
// ❌ 动态委托只能用 UObject/蓝图函数，不能绑 lambda
FOnDeath.OnDeath.AddLambda([this](){ ... });   // 编译不过

// ✅ 动态委托只能这样绑
Character->OnDeath.AddDynamic(this, &AMyUI::HandleDeath);
```

**④ 在 Tick / 高频循环里调用动态委托 → 性能浪费**
动态委托走反射，高频触发用普通多播委托即可。

---

#### 八、与其他机制的对比（什么时候不用委托）

| 场景 | 用委托还是别的 |
|------|--------------|
| C++ 对象间解耦回调 | ✅ 单播/多播委托 |
| 暴露事件给蓝图 | ✅ 动态多播委托 |
| 想持久化保存事件配置 | ✅ 动态委托（可序列化） |
| 需要知道"某对象销毁" | ❌ 用 `FOnDestroyed` / `TWeakObjectPtr` + 委托 |
| 想监听输入/碰撞等引擎事件 | ❌ 引擎已内置，直接重写回调/用 EventDispatcher |
| 纯 C++ 高性能事件总线 | ✅ 普通多播委托（不用动态） |

---

### 2.5 FMath — 数学工具箱

**文件**：`Public/Math/UnrealMathUtility.h`

#### 常用常量

```cpp
PI                          // 3.14159265358979323846
UE_SMALL_NUMBER             // 1.e-8（浮点比较的容差）
UE_KINDA_SMALL_NUMBER       // 1.e-4
UE_BIG_NUMBER               // 3.4e+98
UE_DOUBLE_PI                // PI * 2
UE_HALF_PI                  // PI / 2
UE_INV_PI                   // 1 / PI
```

#### 常用函数

```cpp
FMath::Abs(x)               // 绝对值
FMath::Clamp(x, Min, Max)   // 限制范围
FMath::Lerp(A, B, Alpha)    // 线性插值：A + (B-A)*Alpha
FMath::Sqrt(x)              // 平方根
FMath::Square(x)            // x²
FMath::Sin/Cos/Tan(x)       // 三角函数（弧度）
FMath::DegreesToRadians(d)  // 角度 → 弧度
FMath::RandRange(Min, Max)  // 随机浮点数
FMath::RandStream()         // 伪随机数流（可复现）
```

#### 向量运算（FVector）

```cpp
FVector A(1.f, 2.f, 3.f);
FVector B(4.f, 5.f, 6.f);

A + B                       // 加法
A - B                       // 减法
FVector::DotProduct(A, B)   // 点积
FVector::CrossProduct(A, B) // 叉积
A.Size()                    // 长度
A.SizeSquared()             // 长度的平方（更快，不用开方）
A.GetSafeNormal()           // 归一化（安全，零向量返回 ZeroVector）
A.Distance(B)               // 两点距离
```

---

### 2.6 HAL — 硬件抽象层

**文件**：`Public/HAL/`

这是引擎和操作系统之间的"翻译层"，让你用同一套代码操作不同平台。

| 文件 | 作用 |
|------|------|
| `Platform.h` | 平台检测宏（PLATFORM_WINDOWS / PLATFORM_LINUX / PLATFORM_MAC） |
| `Thread.h` | 线程抽象（FRunnable、FRunnableThread） |
| `CriticalSection.h` | 临界区/互斥锁（FCriticalSection） |
| `FileManager.h` | 文件操作（IFileManager::Get().Copy/Move/Delete） |
| `PlatformFile.h` | 平台文件系统抽象 |
| `MemoryBase.h` | 内存分配接口 |
| `UnrealMemory.h` | FMemory::Malloc/Free/Memcpy/Memzero |
| `PlatformProcess.h` | 进程操作（创建子进程、获取 PID） |
| `Event.h` | 事件信号（线程间同步） |
| `Runnable.h` | 可运行对象基类（多线程入口） |

#### 示例：跨平台文件操作

```cpp
// 不需要 #ifdef _WIN32，UE 帮你处理了
IFileManager& FM = IFileManager::Get();
FM.Copy(TEXT("Dest.txt"), TEXT("Src.txt"), false, true);
FM.Delete(TEXT("Temp.txt"));
```

---

## 三、其他重要子系统速查

### 3.1 模板工具（Templates/）

| 文件 | 作用 |
|------|------|
| `TypeTraits.h` | 编译期类型判断（`TIsPointer_V<T>`、`TIsSame_V<A,B>`） |
| `UnrealTemplate.h` | UE 特有模板（`TUniquePtr`、`MakeShared`） |
| `Tuple.h` | 元组实现 |
| `Function.h` | 函数包装器（类似 std::function） |
| `Sorting.h` | 排序算法（`Algo::Sort`、`Algo::BinarySearch`） |
| `Invoke.h` | 调用转发（`Invoke`、`Forward`） |

### 3.2 序列化（Serialization/）

| 文件 | 作用 |
|------|------|
| `Archive.h` | FArchive 基类（所有读写的抽象） |
| `MemoryReader.h` | 从内存读取 |
| `MemoryWriter.h` | 写入内存 |
| `BufferArchive.h` | 缓冲区读写 |
| `StructuredArchive.h` | 结构化序列化（JSON-like） |

### 3.3 异步（Async/）

| 文件 | 作用 |
|------|------|
| `Future.h` | Future/Promise 模式 |
| `TaskGraphInterfaces.h` | 任务图（UE 的线程池调度系统） |
| `ParallelFor.h` | 并行 for 循环 |

### 3.4 日志系统（Misc/）

```cpp
// 日志级别
UE_LOG(LogTemp, Log, TEXT("普通信息"));
UE_LOG(LogTemp, Warning, TEXT("警告"));
UE_LOG(LogTemp, Error, TEXT("错误"));
UE_LOG(LogTemp, Fatal, TEXT("致命错误"));  // 会崩溃

// 自定义日志类别
DECLARE_LOG_CATEGORY_EXTERN(LogMyGame, Log, All);
DEFINE_LOG_CATEGORY(LogMyGame);
```

---

## 四、Core 在整个引擎中的位置

```
┌─────────────────────────────────────────────┐
│              你的游戏代码                     │
├─────────────────────────────────────────────┤
│  Engine (AActor/UWorld/GameMode...)          │
├─────────────────────────────────────────────┤
│  CoreUObject (UObject/GC/反射/序列化)         │
├─────────────────────────────────────────────┤
│  Renderer/Slate/AIModule/Network...          │
├─────────────────────────────────────────────┤
│  ★ Core ★                                    │
│  ┌─────────────────────────────────────────┐│
│  │ TArray/FString/TMap/Delegate/FMath/HAL  ││
│  └─────────────────────────────────────────┘│
├─────────────────────────────────────────────┤
│  C++ 标准库 (<memory>/<vector>/<string>)     │
├─────────────────────────────────────────────┤
│  操作系统 (Windows/Linux/macOS/iOS/Android)  │
└─────────────────────────────────────────────┘
```

**Core 是唯一被所有其他模块依赖的模块。** 它不依赖任何 UE 模块，只依赖 C++ 标准库和平台 API。

---

## 五、新手必须掌握的 10 个 API

| 优先级 | API | 所属 | 原因 |
|--------|-----|------|------|
| ⭐⭐⭐ | `TArray<T>` | Containers | 到处都在用，替代 std::vector |
| ⭐⭐⭐ | `FString` | Containers | 字符串操作，替代 std::string |
| ⭐⭐⭐ | `FName` | UObject/NameTypes | 标识符，比 FString 快得多 |
| ⭐⭐⭐ | `UE_LOG` | Misc/Logging | 日志输出 |
| ⭐⭐⭐ | `FMath::Clamp/Lerp` | Math | 游戏数学必备 |
| ⭐⭐ | `TMap<K,V>` | Containers | 键值对查找 |
| ⭐⭐ | `TSharedPtr<T>` | Templates | 引用计数智能指针 |
| ⭐⭐ | `FVector` | Math | 3D 向量 |
| ⭐⭐ | `TEXT()` 宏 | CoreTypes | 宽字符字符串字面量 |
| ⭐ | `FCriticalSection` | HAL | 多线程互斥锁 |

---

## 六、常见陷阱

### 6.1 TArray 迭代器失效

```cpp
// ❌ 错误：遍历时删除元素
for (int32 i = 0; i < Arr.Num(); i++) {
    if (ShouldRemove(Arr[i])) {
        Arr.RemoveAt(i);  // 后面的元素前移，i 跳过了下一个！
    }
}

// ✅ 正确：倒序遍历删除
for (int32 i = Arr.Num() - 1; i >= 0; i--) {
    if (ShouldRemove(Arr[i])) {
        Arr.RemoveAt(i);
    }
}

// ✅ 更好：使用 RemoveAllSwap（如果不关心顺序）
Arr.RemoveAllSwap([&](const auto& Item) { return ShouldRemove(Item); });
```

### 6.2 FString 转 const char*

```cpp
FString Str = TEXT("Hello");

// ❌ 错误：直接转可能乱码
const char* Bad = TCHAR_TO_ANSI(*Str);  // 非 ASCII 字符会丢失

// ✅ 正确：保持宽字符
const TCHAR* Wide = *Str;  // 或 Str.GetData()

// ✅ 需要 ANSI 时用
FTCHARToANSI AnsiConverter(*Str);
const char* Ansi = AnsiConverter.Get();
```

### 6.3 FName 不要用 == 频繁比较

```cpp
// FName 的 == 比较的是哈希值，不是字符串内容
// 对于大量比较场景，先转 FString 再比较
FName A(TEXT("LongName123"));
FName B(TEXT("LongName123"));
// A == B  → 比较哈希（快但理论上有碰撞风险）
// 如果需要绝对精确，用 A.IsEqual(B, ENameCompareFlags::CaseSensitive)
```

---

## 七、Core/Public 全目录速查（UE5.6 实际清单）

> 这是 `Engine/Source/Runtime/Core/Public/` 下**真实存在的全部子目录**，按"干什么的"分类说明。比文档开头的简化图更全。

### 7.1 核心基础（★ 常打交道）

| 目录 | 干什么的 |
|------|---------|
| `Containers/` | **容器**：TArray、TMap、TSet、FString、TSharedPtr（最常用） |
| `Math/` | **数学**：FVector、FRotator、FTransform、FMath（99 个文件，很大） |
| `Templates/` | **模板工具**：MoveTemp、类型推断、Function、Tuple、智能指针源码 |
| `Delegates/` | **委托**：单播/多播/动态委托（Delegate.h 等） |
| `UObject/` | **FName 名字系统**（NameTypes.h）+ 部分 UObject 基础 |

### 7.2 底层系统（理解原理用）

| 目录 | 干什么的 |
|------|---------|
| `HAL/` | **硬件抽象层**：线程、锁、文件、内存接口（跨平台，93 个文件，最大之一） |
| `Serialization/` | **序列化**：FArchive、内存读写（存盘/读档/网络传输底层） |
| `Async/` | **异步**：Future/Promise、TaskGraph、ParallelFor（多线程任务） |
| `Tasks/` | **任务系统**：UE 的 Task 系统（比 TaskGraph 更现代） |
| `IO/` | **IO 操作**：异步文件/流读取 |
| `Memory/` | **内存管理**：内存池、分配器细节 |
| `String/` | **字符串处理**：字符转换、字符串算法（补充 FString 外的工具） |
| `Hash/` | **哈希**：哈希函数、散列工具 |
| `Traits/` | **类型特征**：编译期类型判断（TypeTraits 拆分出来的） |
| `Concepts/` | **C++ 概念**：模板约束（C++20 Concepts 封装） |
| `Compression/` | **压缩**：LZ4、Oodle 等压缩算法 |
| `Stats/` | **统计**：游戏统计、性能计数（STAT 宏） |
| `ProfilingDebugging/` | **性能分析**：Profile 标记、调试工具（35 个文件） |
| `Logging/` | **日志**：Log.h、LogCategory.h（UE_LOG 定义在这） |
| `Misc/` | **杂项**：断言、配置文件、命令行、各种工具（181 个文件，最大目录） |

### 7.3 平台相关（跨平台，一般不用管）

| 目录 | 干什么的 |
|------|---------|
| `GenericPlatform/` | **通用平台层**：所有平台的基类（跨平台代码都在这） |
| `Windows/` `Microsoft/` `MSVC/` | Windows 平台实现 |
| `Linux/` `Unix/` `Clang/` | Linux 平台实现 |
| `Mac/` `Apple/` | Mac 平台实现 |
| `IOS/` `Android/` | 移动平台实现 |
| `IntelICX/` | Intel 编译器实现 |

> 这些是"同一套接口，不同平台的实现"。你用 `IFileManager`、`FPlatformMisc` 时，底层会跳到对应平台目录。**正常开发不用碰。**

### 7.4 其他（了解即可）

| 目录 | 干什么的 |
|------|---------|
| `Algo/` | **算法库**：Sort、BinarySearch、Transform（类似 std::algorithm，44 个文件） |
| `Audio/` | **音频**：基础音频类型 |
| `ColorManagement/` | **颜色管理**：色彩空间转换 |
| `Internationalization/` | **本地化**：多语言翻译、Text 系统 |
| `Modules/` | **模块系统**：FModuleManager（加载/卸载模块） |
| `Features/` | **特性开关**：游戏特性开关 |
| `FileCache/` | **文件缓存**：DDC 相关 |
| `Experimental/` | **实验性**：ConcurrentLinearAllocator 等实验代码 |
| `FramePro/` `MemPro/` | **第三方性能分析**工具接入 |
| `Sanitizer/` | **内存检查**：ASan/UBSan 等消毒器接口 |
| `AutoRTFM/` | **事务内存**：实验性原子内存系统 |
| `Virtualization/` | **虚拟化**：资产虚拟化（大世界资产流送） |
| `Tests/` | **测试**：单元测试基础 |

### 7.5 根目录的 .h 文件（一进 Core 就有的）

| 文件 | 干什么的 |
|------|---------|
| `Core.h` | Core 模块总入口头文件 |
| `CoreMinimal.h` | **最小依赖头**（最常用，include 它够用大多数场景） |
| `CoreFwd.h` | **前向声明**（只声明类名，不定义） |
| `CoreTypes.h` | 基础类型定义（int32、float、TCHAR、FString 声明） |
| `CoreGlobals.h` | 全局变量、全局函数声明（GEngine、GWarn 等） |
| `CoreSharedPCH.h` | 共享预编译头 |
| `PixelFormat.h` | 像素格式定义（渲染相关） |

> **新手最常 include 的是 `CoreMinimal.h`**——它包含了大部分常用类型，一个头够用。

### 7.6 学习建议（哪些值得看，哪些不用管）

| 类别 | 要不要学 | 建议 |
|------|:---:|------|
| Containers / Math / Delegates / Templates | ✅ 重点 | 日常开发天天用 |
| UObject(FName) / Logging / String | ✅ 常碰 | 用得多 |
| HAL / Serialization / Async / Misc | ⭐ 理解 | 懂概念即可，用到再深入 |
| 平台目录（Windows/Linux...） | ❌ 不用管 | 正常开发不碰 |
| Algo / Stats / Hash / Experimental | ❌ 遇再看 | 需要时查 |

**一句话**：Core/Public 里，**开发时真正高频的是 Containers/Math/Delegates/Templates** 这几个；平台目录和一堆底层工具（Algo/Stats/Hash/IO...）**不用主动学**，遇到再查。知道"每个目录大概是干嘛的"，就能在需要时准确找到地方。
