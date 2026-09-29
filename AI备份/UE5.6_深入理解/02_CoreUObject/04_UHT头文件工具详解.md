# 04 — UHT 头文件工具详解（Unreal Header Tool）

> **定位**：UHT = **Unreal Header Tool（虚幻头文件工具）**。它是 UE 反射系统的"幕后功臣"——你写的 `UCLASS`/`UPROPERTY`/`UFUNCTION` 宏，全靠 UHT 扫描并生成反射代码。
>
> **一句话**：UHT 是**编译前自动运行的一个工具**，它扫描你带反射宏的 `.h` 文件，生成 `.generated.h` 反射代码，让引擎运行时能"认识"你的 C++ 类。
>
> **文件**：`Engine/Source/Programs/UnrealHeaderTool/`（UHT 本身是独立程序）

---

## 一、先搞懂：UHT 是干嘛的（一句话 + 一个流程）

**UHT = 编译前的"代码生成器"**。它在 C++ 编译之前运行，读你的反射宏，生成反射代码。

```
你写带宏的 .h 文件
  │
  ▼
UHT（编译前自动运行，读 UCLASS/UPROPERTY/UFUNCTION...）
  │
  ▼
生成 .generated.h（反射代码）
  │
  ▼
C++ 编译器把"你的代码 + 生成的代码"一起编译
  │
  ▼
引擎运行时能反射（蓝图/序列化/GC/网络）
```

**关键**：UHT **在 C++ 编译之前**运行。你改一个 `UPROPERTY`，UHT 会重新生成反射代码，然后才编译。

---

## 二、UHT 到底做什么（三个核心工作）

### 2.1 扫描宏，收集反射信息

UHT 读你的 `.h` 文件，识别所有反射宏，收集信息：

```cpp
// UHT 看到这些：
UCLASS(BlueprintType)
class UMyCharacter : public UObject {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite) float Health;      // 收集：属性
    UFUNCTION(BlueprintCallable) void TakeDamage();  // 收集：函数
};
```

UHT 记录下：类叫 UMyCharacter、有属性 Health、有函数 TakeDamage、参数是 BlueprintReadWrite 等。

### 2.2 生成反射代码（.generated.h）

UHT 根据收集的信息，**生成 `.generated.h` 文件**：

```
UMyCharacter.generated.h   ← UHT 自动生成的
  包含：
  - StaticClass() 的实现
  - 反射表（属性/函数注册表）
  - 类型转换、Super 定义
  - 蓝图虚拟机需要的代码
```

### 2.3 检查错误

UHT 会**编译前检查**宏写错没（比如 GENERATED_BODY 没写、参数不对），提前报错，比 C++ 编译器更早发现问题。

---

## 三、UHT 的底层运行机制（重点理解）

### 3.1 它是个"独立程序"

UHT 不是引擎的一部分，是**独立可执行程序**（`UnrealHeaderTool.exe`），在编译时由 **UBT（Unreal Build Tool）** 调用。

```
UBT（Unreal Build Tool，负责编译）
  └─ 编译你的模块时
      └─ 先调用 UHT
          └─ UHT 扫描 .h → 生成 .generated.h
              └─ 再真正编译 C++
```

### 3.2 它用反射宏"标记"来理解代码

UHT 会**解析 C++ 头文件**，但它只关心带反射宏的部分。普通 C++ 代码它不关心（交给 C++ 编译器）。

```
UHT 只处理：UCLASS/UPROPERTY/UFUNCTION/USTRUCT/UENUM/UINTERFACE 标记的内容
其他普通 C++：不处理
```

### 3.3 为什么要有 GENERATED_BODY()

`GENERATED_BODY()` 是 UHT 生成代码的**"接口点"**——UHT 知道该把生成的代码放哪。没有它，UHT 生成的代码无处安放，反射就没法建立。

---

## 四、UHT 和 UBT 的区别（容易混）

| | UBT（Unreal Build Tool） | UHT（Unreal Header Tool） |
|---|---|---|
| 全名 | Unreal Build Tool | Unreal Header Tool |
| 干嘛 | **编译** C++、管理模块依赖 | **生成**反射代码 |
| 管什么 | 编译流程、.Build.cs | .h 文件、反射宏 |
| 产物 | .dll / .exe | .generated.h |

```
编译流程：
UBT（管编译）
  ├─ 调用 UHT → 生成 .generated.h（管反射）
  └─ 调用 C++ 编译器 → 生成 .dll（管代码）
```

**记忆**：**UBT 编译，UHT 生成反射代码**。UBT 是"包工头"（管整个编译），UHT 是"砌墙的"（专门生成反射那部分）。

---

## 五、具体场景：改代码后发生了什么

**场景：你在类里加了一个 UPROPERTY**

```cpp
// 你改代码：新增一个属性
UPROPERTY(BlueprintReadWrite) int32 Score;   // 新增
```

**保存 → 编译时 UHT 会**：
1. 扫描头文件，发现新增的 `Score` 属性
2. 重新生成 `UMyCharacter.generated.h`，把 Score 加进反射表
3. C++ 编译器编译新代码
4. 引擎启动后，蓝图/序列化就能看到 `Score`

> **如果你改了 UPROPERTY 却觉得蓝图没反应**，往往是因为没重新编译（UHT 没重跑）。

---

## 六、常见问题

**① "Missing type specifier / unknown override" 编译错**
通常是 `GENERATED_BODY()` 放错位置或漏写，UHT 生成的代码无法正确插入。

**② 改了宏参数但蓝图没变化**
可能没重新编译（UHT 没重跑）。重新编译让 UHT 重新生成。

**③ 忘 include .generated.h**
`GENERATED_BODY()` 会自动 include，但如果你手写反射相关代码可能需要显式 include `UMyClass.generated.h`。

**④ UHT 报 "not allowed to declare"**
反射宏用在不支持的地方（比如局部变量、普通类），UHT 会拒绝。

---

## 七、总结速查

```
UHT = Unreal Header Tool（头文件工具）
作用：编译前扫描反射宏 → 生成 .generated.h → 让引擎能反射
位置：独立程序，由 UBT 在编译时调用

UHT 处理：UCLASS/UPROPERTY/UFUNCTION/USTRUCT/UENUM/UINTERFACE
产物：UMyClass.generated.h

区别：
  UBT = 编译（管整个编译流程）
  UHT = 生成反射代码（管 .generated.h）

运行时机：每次编译前（改宏就要重跑）
```

**一句话**：UHT 是**编译前的反射代码生成器**。你写 `UCLASS`/`UPROPERTY` 等宏，UHT 扫描后生成 `.generated.h`，让引擎能反射你的 C++ 类。**它和 UBT 的区别是：UBT 管编译，UHT 管生成反射代码。**

---

## 八、底层源码实拍（真实宏定义长这样）

> 直接看 UE5.6 引擎真实源码 `CoreUObject/Public/UObject/ObjectMacros.h`，看看 `UCLASS` / `GENERATED_BODY` 到底是怎么定义的。

### 8.1 UCLASS() 的真实定义（空宏）

```cpp
// ObjectMacros.h 第 754 行（真实源码）
#define UCLASS(...) BODY_MACRO_COMBINE(CURRENT_FILE_ID,_,__LINE__,_PROLOG)
```

**展开过程**：

```
UCLASS(...)
→ BODY_MACRO_COMBINE(CURRENT_FILE_ID, _, __LINE__, _PROLOG)
→ 拼接成一个名字：MyFile_1234_PROLOG
```

**结论**：`UCLASS()` 本身是**空壳宏**，只拼出一个"名字/标记"，几乎没实质代码。它的作用 = **在类前面放一个"PROLOG"标记**，让 UHT 生成的代码能定位到这个类的开头。

### 8.2 GENERATED_BODY() 的真实定义（拼接宏）

```cpp
// ObjectMacros.h 第 743 行（真实源码）
#define GENERATED_BODY(...) BODY_MACRO_COMBINE(CURRENT_FILE_ID,_,__LINE__,_GENERATED_BODY);
```

**展开过程**：

```
GENERATED_BODY()
→ MyFile_1234_GENERATED_BODY;   ← 一个"名字 + 分号"
```

**结论**：`GENERATED_BODY()` 也只是**拼出一个名字**（`文件名_行号_GENERATED_BODY`）。这个**名字的定义**在 UHT 生成的 `.generated.h` 里。

### 8.3 关键真相：宏是"门牌号"，.generated.h 是"家具"

```
UCLASS()        → MyFile_1234_PROLOG          （门牌号，你写的）
GENERATED_BODY()→ MyFile_1234_GENERATED_BODY; （门牌号，你写的）

这两个名字的"真正定义"（反射代码）→ 在 UHT 生成的 .generated.h 里
```

**编译时**：宏展开的名字 **include 到 UHT 生成的代码**，合体成一个完整的反射类。

### 8.4 .generated.h 里有什么（UHT 生成的真实代码）

假设你写了：

```cpp
UCLASS()
class UMyActor : public UObject {
    GENERATED_BODY()
public:
    UPROPERTY() int32 Health;
};
```

UHT 生成的 `UMyActor.generated.h` 里**大致包含**（简化）：

```cpp
// ① 定义 StaticClass()：返回这个类的 UClass 描述
static UClass* UMyActor::StaticClass();

// ② 定义 GENERATED_BODY() 展开的那个名字（真正的反射代码）
#define UMyActor_1234_GENERATED_BODY \
    static UClass* StaticClass(); \
    static void StaticRegisterNativesUMyActor(); \
    /* 反射表、属性注册、蓝图代码... */

// ③ 注册 Health 属性（描述这个属性的 FProperty 元数据）
// 这样蓝图/序列化才能认识 Health
```

**真实生成的代码包括**：
1. `StaticClass()` —— 返回类的 UClass
2. 反射表 —— 注册所有 UPROPERTY/UFUNCTION
3. `StaticRegisterNatives` —— 注册 native 函数
4. `Cast<>` / 类型转换支持

### 8.5 完整流程（看代码版）

```cpp
// ① 你写的
UCLASS()                                  // → MyFile_10_PROLOG
class UMyActor : public UObject {
    GENERATED_BODY()                      // → MyFile_12_GENERATED_BODY;
public:
    UPROPERTY() int32 Health;             // UHT 注意到这个属性
};

// ② UHT 生成 UMyActor.generated.h，定义那两个"名字"的真实反射代码

// ③ 编译时：宏展开 + include 生成代码 = 完整反射类
class UMyActor : public UObject {
    MyFile_12_GENERATED_BODY;   // 展开成 UHT 生成的反射代码
public:
    int32 Health;
};
```

### 8.6 结论（看代码后）

> **`UCLASS()` / `GENERATED_BODY()` 底层就是"拼接宏"，展开成 `文件名_行号_标记` 这种名字。真正干活的反射代码在 UHT 生成的 `.generated.h` 里。** 你写的宏是"门牌号"，UHT 生成的代码是"家具"，编译时合体。

**所以**：
- 宏本身**很简单**（就是拼个名字的空壳）
- 复杂的是 **UHT 生成的 `.generated.h`**（几千行反射代码）
- 你不需要会写那些生成代码，只要知道：**宏 = 门牌号，UHT 生成 = 家具**

---

## 九、下一步

理解了 UHT（反射代码怎么来），你就完整掌握了 UObject 反射系统的"怎么用"（宏）+"怎么实现"（UHT）。接下来可以进入 **Engine 模块的 AActor**（生命周期、组件、GameMode），开始真正写游戏逻辑了。
