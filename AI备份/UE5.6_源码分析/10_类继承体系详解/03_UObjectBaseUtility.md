# 03 - 第 2 层：UObjectBaseUtility — 给对象装工具

> **源码位置**：`Engine\Source\Runtime\CoreUObject\Public\UObject\UObjectBaseUtility.h`
> 
> 这一层给 UObject 装上**常用工具函数**，让对象"用起来方便"。

---

## 一、它做了什么？

```cpp
class UObjectBaseUtility : public UObjectBase {
public:
    // 这些是你天天用的函数，都定义在这一层：
    
    FName GetName() const;           // 获取对象名字
    UClass* GetClass() const;        // 获取对象的类（运行时类型识别）
    UObject* GetOuter() const;       // 获取外层对象（UE 的对象层级）
    bool IsPendingKill() const;      // 是否正在被 GC 回收？
    void MarkPendingKill();          // 标记为待回收
    // ... 几十个基础工具函数
};
```

---

## 二、为什么需要这一层？

**把常用工具函数抽离出来**，好处：

1. `UObject` 不用重复写这些代码
2. 有些内部类只需要 `UObjectBase`（不需要这些工具），可以省内存
3. 职责清晰，好维护

---

## 三、常用方法清单

### 身份识别类（最常用）⭐

| 方法 | 作用 | 使用频率 |
|------|------|---------|
| `GetName()` | 获取对象名字（字符串） | ⭐⭐⭐ |
| `GetClass()` | 获取对象的 UClass（运行时类型） | ⭐⭐⭐ |
| `GetFullName()` | 获取完整路径名（含外层） | ⭐⭐ |
| `GetFName()` | 获取 FName 类型名字（更快） | ⭐⭐ |
| `IsA<T>()` | 判断对象是不是 T 类型 | ⭐⭐⭐ |

```cpp
// 例子：判断一个对象是不是 ACharacter 类型
if (SomeActor->IsA<ACharacter>()) {
    // 是角色，可以安全地转型
    ACharacter* Char = Cast<ACharacter>(SomeActor);
}
```

### 生命周期/状态类 ⭐

| 方法 | 作用 | 使用频率 |
|------|------|---------|
| `IsPendingKill()` | 是否正在被 GC 回收？ | ⭐⭐⭐ |
| `MarkPendingKill()` | 标记为待回收（旧版） | ⭐⭐ |
| `SetFlags()` / `ClearFlags()` | 设置/清除对象标志 | ⭐ |
| `HasAnyFlags()` | 检查是否有某个标志 | ⭐ |

```cpp
// 例子：检查对象是否还活着
if (!IsValid(MyObject)) {   // IsValid 内部就是检查 IsPendingKill
    // 对象已经被回收了，不能用
}
```

### 层级关系类 ⭐

| 方法 | 作用 | 使用频率 |
|------|------|---------|
| `GetOuter()` | 获取外层对象（谁包含我？） | ⭐⭐ |
| `GetOutermost()` | 获取最外层（通常是 UPackage） | ⭐ |
| `Rename()` | 重命名对象 | ⭐ |

```cpp
// 例子：获取 Actor 所在的关卡
UWorld* World = MyActor->GetOuter()->GetOuter();  // Actor → Level → World
```

### 模板函数（配合 StaticClass 用）⭐⭐⭐

| 函数 | 作用 | 使用频率 |
|------|------|---------|
| `Cast<T>()` | 安全类型转换 | ⭐⭐⭐ |
| `NewObject<T>()` | 创建新对象 | ⭐⭐⭐ |
| `LoadObject<T>()` | 加载资源 | ⭐⭐ |
| `FindObject<T>()` | 查找已存在的对象 | ⭐⭐ |

```cpp
// 例子：创建和转换
UMyData* Data = NewObject<UMyData>();                    // 创建
ACharacter* Char = Cast<ACharacter>(SomeActor);          // 转换
UTexture2D* Tex = LoadObject<UTexture2D>(...);           // 加载资源
```

---

## 四、最常用的 5 个（记住这些就够）

1. **`GetName()`** — 拿名字
2. **`GetClass()`** — 拿类型信息
3. **`IsA<T>()`** — 判断是不是某类型
4. **`Cast<T>()`** — 安全转型
5. **`IsPendingKill()`** — 检查是否还活着

---

## 五、类比理解

> **UObjectBaseUtility = 房子的水电管道**
> 
> - 有了它，对象才能"用起来方便"（能查名字、能查类型）
> - 它是从"能存在"到"能用"的关键一步

---

## 六、一句话总结

> **UObjectBaseUtility = UE 对象的"工具箱"**
> 
> - 身份识别：我是谁？我是什么类型？
> - 生命周期：我还活着吗？
> - 层级关系：谁包含我？
> - 模板函数：创建、转换、加载
