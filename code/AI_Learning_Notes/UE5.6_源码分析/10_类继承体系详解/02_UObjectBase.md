# 02 - 第 1 层：UObjectBase — 让对象能"活"着

> **源码位置**：`Engine\Source\Runtime\CoreUObject\Public\UObject\UObjectBase.h`
> 
> 这一层是 UE 对象系统的**地基**，负责内存管理和生命周期。

---

## 一、它做了什么？

```cpp
class UObjectBase {
public:
    // 1. 重写 new/delete（UE 自己的内存管理）
    void* operator new(size_t Size);
    void operator delete(void* Ptr);
    
    // 2. 构造/析构（对象生命周期的起点和终点）
    UObjectBase();           // 出生
    virtual ~UObjectBase();  // 死亡
    
    // 3. 状态标志
    EObjectFlags ObjectFlags;  // 这个对象现在是什么状态？
};
```

---

## 二、为什么需要这一层？

| 功能 | 说明 |
|------|------|
| **统一内存管理** | UE 不用系统的 `malloc/free`，而是用自己的内存池（更快、更可控） |
| **生命周期管理** | 每个对象出生时注册、死亡时注销，GC 才能知道谁还活着 |
| **状态追踪** | `ObjectFlags` 记录对象是否在 GC 中、是否有效等 |

---

## 三、深入理解：重写 operator new

### 普通 C++ 的 new

```cpp
// 当你写：
MyClass* p = new MyClass();

// 编译器实际做了两件事：
// 1. 调用 operator new 分配内存（从系统堆里要一块空间）
void* memory = operator new(sizeof(MyClass));

// 2. 在这块内存上构造对象
MyClass* p = new (memory) MyClass();
```

**系统的 `operator new`**：随便找一块空闲内存给你，不管别的。

### UE 重写后

```cpp
void* UObjectBase::operator new(size_t Size) {
    // 1. 从 UE 自己的内存池分配（不是系统堆）
    void* memory = GEngine->AllocateMemory(Size);
    
    // 2. 记录：这块内存属于哪个对象、多大、什么状态
    RegisterObject(memory, Size, ObjectFlags);
    
    return memory;
}
```

**好处**：
- UE 知道每块内存是谁的
- GC 可以遍历所有对象，检查谁还活着
- 内存分配更快（预分配内存池）

---

## 四、深入理解：构造函数和析构函数

### 构造函数 — 对象的"出生仪式"

```cpp
UObjectBase::UObjectBase() {
    // UE 在这里做：
    // - 注册到 GC 系统（加入全局对象列表）
    // - 设置 ObjectFlags（标记为"有效"）
    // - 记录类类型
}
```

**作用**：确保对象创建后就被 UE "登记在册"。

### 析构函数 — 对象的"葬礼"

```cpp
UObjectBase::~UObjectBase() {
    // UE 在这里做：
    // - 从 GC 系统移除（从全局对象列表删除）
    // - 通知相关系统"这个对象没了"
}
```

**作用**：确保对象销毁时"干净利落"，不会留下垃圾。

---

## 五、ObjectFlags 是什么？

```cpp
enum class EObjectFlags : uint32 {
    RF_NoFlags = 0,              // 无特殊标志
    RF_Public = 0x00000001,      // 对象是公开的
    RF_Standalone = 0x00000002,  // 对象独立存在
    RF_MirroredGarbage = 0x00000040,  // 镜像垃圾标志
    RF_BeginDestroyed = 0x00000080,   // 开始销毁
    RF_FinishDestroyed = 0x00000100,  // 完成销毁
    // ... 几十个标志
};
```

**用途**：快速判断对象状态，不用额外查询。

---

## 六、类比理解

> **UObjectBase = 房子的地基**
> 
> - 没有地基，房子立不起来
> - 但它只管"能不能立起来"，不管水电装修
> - 地基里埋了"管道接口"（ObjectFlags），为上层做准备

---

## 七、常见问题

### Q1：UObjectBase 之上还有更底层的 UE 类吗？

**没有**。`UObjectBase` 就是 UE 对象系统的根，再往下就是 C++ 标准库了。

### Q2：为什么 UE 不用系统的 malloc/free？

- 系统的内存管理"不知道"对象信息，无法配合 GC
- UE 的内存池可以预分配，减少碎片，提高性能
- 方便调试（UE 能追踪每块内存的归属）

### Q3：operator 是什么？

C++ 的关键字，表示"这是一个运算符，可以被重载"。`operator new` 就是"new 运算符对应的函数"。

---

## 八、一句话总结

> **UObjectBase = UE 对象系统的"地基"**
> 
> - 重写 new/delete → 统一内存管理
> - 构造/析构 → 生命周期管理
> - ObjectFlags → 状态追踪
> 
> 有了它，UE 才能"认识"每一个对象。
