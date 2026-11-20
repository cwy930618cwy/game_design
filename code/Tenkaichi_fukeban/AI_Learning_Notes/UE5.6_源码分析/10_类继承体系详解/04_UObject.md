# 04 - 第 3 层：UObject — 加上 UE 的魔法

> **源码位置**：`Engine\Source\Runtime\CoreUObject\Public\UObject\Object.h`
> 
> 这一层给对象加上**反射系统**，让 UE 能真正"认识"和管理对象。

---

## 一、它做了什么？

```cpp
UCLASS()   // ← 这个宏让 UE 认识这个类
class UObject : public UObjectBaseUtility {
    GENERATED_BODY()   // ← 自动生成构造函数、StaticClass() 等
    
public:
    // 反射系统相关
    static FORCEINLINE const UClass* StaticClass();   // 返回这个类的"身份证"
    virtual UClass* GetClass() const override;
    
    // 序列化
    virtual void Serialize(FArchive& Ar);
    
    // 网络复制
    virtual bool IsSupportedForNetworking() const;
    
    // GC 相关
    virtual void PostInitProperties();
    virtual void BeginDestroy();
};
```

---

## 二、关键概念解释

### 1. `StaticClass()` 是什么？

```cpp
// 每个 UE 类都有一个 StaticClass() 函数
// 调用它就能拿到这个类的"身份证"（UClass 指针）

UClass* IDCard = UObject::StaticClass();
//              ↑ 里面存着：类名、父类是谁、对象大小、有哪些属性...
```

**用途**：
- `NewObject<T>()` — 动态创建对象（UE 靠 StaticClass 知道要创建什么）
- `IsA(T::StaticClass())` — 判断类型（这个对象是不是 T 类型？）
- 序列化 — 存档时靠它知道怎么读写

### 2. 为什么构造函数要传 `FObjectInitializer`？

```cpp
// 普通 C++：无参构造
MyClass obj;

// UE：必须带参数
UObject(const FObjectInitializer& ObjectInitializer);

// 因为 UE 需要在构造时注入信息：
// - 这个对象是哪个类的子对象？
// - 初始化标志是什么？
```

### 3. UCLASS() 和 GENERATED_BODY() 的关系

| 宏 | 作用 | 生成的东西 |
|----|------|-----------|
| `UCLASS()` | 让 UE 认识这个类 | `StaticClass()` 声明、反射注册代码 |
| `GENERATED_BODY()` | 自动生成样板代码 | 构造函数、析构函数、拷贝构造、`StaticClass()` 实现 |

```cpp
UCLASS()                    // 先说"这个类我要管"
class UMyClass : public UObject {
    GENERATED_BODY()        // 再说"帮我生成构造/析构/StaticClass"
};
```

---

## 三、UClass 里存了什么？

```cpp
class UClass : public UStruct {
    FName ClassName;           // "UMyClass"
    UClass* SuperClass;        // 父类指针 → UObject
    int32 InstanceSize;        // sizeof(UMyClass)
    TArray<FProperty*> Properties;   // 所有属性列表
    TArray<UFunction*> Functions;    // 所有函数列表
    // ... 还有几十个字段的元数据
};
```

**每个类只有一个 UClass 实例**（全局单例），这就是"反射信息只有一份"的原因。

---

## 四、完整流程示例

```cpp
// 你写的代码
UCLASS()
class UMyClass : public UObject {
    GENERATED_BODY()
    
    UPROPERTY()
    int Health;
};

// UE 编译时自动生成（简化版）
const UClass* UMyClass::StaticClass() {
    static UClass* SingletonClass = nullptr;
    if (SingletonClass == nullptr) {
        // 构造 UClass，填入类名、父类、大小等信息
        SingletonClass = new UClass(
            "UMyClass",                    // 类名
            UObject::StaticClass(),        // 父类是谁
            sizeof(UMyClass),              // 对象大小
            ...                            // 其他元数据
        );
    }
    return SingletonClass;
}
```

---

## 五、类比理解

> **UObject = 房子的装修和家具**
> 
> - 有了它，对象才能被 UE 管理（序列化、GC、网络复制、蓝图访问……）
> - 它是从"能用"到"被引擎完整管理"的关键一步

---

## 六、一句话总结

> **UObject = UE 对象系统的"核心"**
> 
> - 反射系统 → 运行时认识对象
> - 序列化 → 存档/读档
> - GC → 自动内存管理
> - 网络复制 → 多人游戏同步
