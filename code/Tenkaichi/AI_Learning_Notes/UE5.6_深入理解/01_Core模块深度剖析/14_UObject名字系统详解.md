# 14 — Core 的 UObject/ 名字系统详解（FName）

> **定位**：`Core/Public/UObject/` —— 注意！这是 **Core 模块里的一个子目录**，不是完整的 CoreUObject 模块。它主要放 **FName 名字系统**。
>
> **一句话**：这个目录的核心是 **`FName`** —— 全局唯一的名字标识符。FName 太基础、连 Core 自己都要用，所以被放在 Core 里，而不是 CoreUObject 模块。
>
> **文件**：`Core/Public/UObject/`（`NameTypes.h` 是核心）

---

## 一、先分清：Core 的 UObject/ 和 CoreUObject 模块（防混淆）

```
Core/Public/UObject/      ← 这篇！Core 里的子目录
   ├── NameTypes.h        ← FName 定义（核心）
   ├── UnrealNames.h      ← 全局名字表
   ├── WeakObjectPtrTemplates.h ← TWeakObjectPtr
   └── *ObjectVersion.h   ← 序列化版本号（一堆）

CoreUObject/Public/UObject/  ← 完整的 UObject 系统（下一篇再学）
   └── Object.h 等          ← UObject 类、反射、GC、序列化
```

**区别**：
- **Core 的 UObject/** = 主要放 **FName**（名字系统，很基础）
- **CoreUObject** = 完整 UObject 系统（UObject 类、反射、GC）

**为什么 FName 放 Core**：连 Core 的容器、序列化都要用 FName，所以它必须在最底层。

---

## 二、FName 是什么（核心中的核心）

### 2.1 一句话

**FName 是一个"全局唯一的名字"，内部用一个整数索引代表它。**

```
FName("Player")  → 查全局名字表 → 存进去 → 返回一个 int 索引（比如 42）
FName("Player")  → 查名字表 → 发现已有 → 返回同一个 42
FName("Player") == FName("Player")  → 比较的是 42 == 42 → true（极快）
```

### 2.2 为什么快

- 比较的是**整数索引**，不是逐字符比较 → **O(1) 极快**
- 同样的字符串在整个引擎里只存一份，省内存

```cpp
// FName 比较极快，适合频繁判断
if (Actor->GetName() == TEXT("Boss")) { ... }
```

---

## 三、FName 具体场景：给对象打 Tag / 判断类型

**场景：判断敌人是不是 Boss（频繁判断，用 FName 快）**

```cpp
// 给敌人设一个类型 Tag
Enemy->Tags.Add(FName(TEXT("Boss")));

// 频繁判断（用 FName 比较，O(1) 快）
if (Enemy->ActorHasTag(TEXT("Boss"))) {
    // 是 Boss，触发 Boss 逻辑
}
```

**场景：蓝图/资源命名，用 FName 当标识符**

```cpp
FName SkillName = TEXT("FireBall");   // 技能名，当标识符用
if (SkillName == TEXT("FireBall")) {  // 快速比较
    CastFireBall();
}
```

---

## 四、FName / FString / FText 怎么选（必背）

| 类型 | 用途 | 能翻译 | 能改 | 比较速度 |
|------|------|:---:|:---:|:---:|
| **FName** | 标识符（Tag、资源名、属性名） | ❌ | ❌ | **O(1) 极快** |
| **FString** | 通用字符串操作 | ❌ | ✅ | O(n) |
| **FText** | 显示给玩家的文本 | ✅ | ✅ | - |

**选型规则**：
- **做标识符、判断、Tag** → FName（快）
- **拼接、修改、查找子串** → FString
- **给玩家看、要翻译** → FText

**转换**：
```cpp
FName → FString：Name.ToString()
FString → FName：FName(Str) 或 FName(*Str)
```

---

## 五、FName 的几个特点（要注意）

### 5.1 大小写不敏感（默认）

```cpp
FName("Foo") == FName("foo");   // true！FName 默认忽略大小写
// 要区分大小写用 IsEqual
A.IsEqual(B, ENameCompareFlags::CaseSensitive);
```

### 5.2 不可变

```cpp
// FName 不能修改
FName Name = TEXT("Player");
// Name += TEXT("X");   // ❌ FName 不可变，没有这种操作
// 要改就新建一个 FName 或用 FString
```

### 5.3 全局名字表

所有 FName 都存在一个**全局名字表**里，所以同名 FName 是同一个索引。这也意味着：
- 创建**大量**动态 FName 会往名字表里加内容（不会销毁）
- 所以**不要**在运行时频繁创建**新的** FName（比如拼字符串），会撑大名字表

```cpp
// ❌ 高频循环里创建新 FName，撑大全局名字表
for (int i = 0; i < 100000; i++) {
    FName Name(*FString::Printf(TEXT("Item_%d"), i));  // 大量新名字
}
// ✅ 用固定的 FName 常量
```

---

## 六、目录里其他东西（了解）

| 文件 | 作用 |
|------|------|
| `NameTypes.h` | **FName 定义（核心）** |
| `UnrealNames.h` / `.inl` | 全局名字表（预定义的名字） |
| `WeakObjectPtrTemplates.h` | `TWeakObjectPtr`（你在 02 学的） |
| `StrongObjectPtrTemplates.h` | `TStrongObjectPtr`（强引用 UObject） |
| `*ObjectVersion.h` | 序列化版本号（控制存档兼容） |

> **重点**：你日常用到的主要是 **FName**（`NameTypes.h`）。`TWeakObjectPtr` 你在 02 已学。`*ObjectVersion.h` 是序列化版本控制，一般不用碰。

---

## 七、常见陷阱

**① 用 == 比较 FString 却想要快**
```cpp
// ❌ 大量比较用 FString（慢）
if (A.ToString() == B.ToString()) { ... }
// ✅ 标识符直接用 FName（快）
if (A == B) { ... }   // A、B 是 FName
```

**② 把 FName 当 FString 拼接**
```cpp
// ❌ FName 不能拼接
FName Name = ...;
// Name += TEXT("x");   // 错
// ✅ 要拼接转 FString
FString S = Name.ToString() + TEXT("x");
```

**③ 高频创建新 FName 撑大名字表**
```cpp
// ❌ 循环里拼新 FName
for (...) { FName(*Printf(...)); }
// ✅ 用常量 FName
```

**④ 忘了 FName 大小写不敏感**
```cpp
// FName("Player") == FName("player") → true
// 需要精确区分用 IsEqual(..., CaseSensitive)
```

---

## 八、总结速查

```
Core/Public/UObject/ = FName 名字系统
├── FName：全局唯一名字，int 索引，比较 O(1) 极快
├── UnrealNames.h：全局名字表
├── WeakObjectPtrTemplates.h：TWeakObjectPtr（已学）
└── *ObjectVersion.h：序列化版本号

FName 特点：不可变、大小写不敏感、全局唯一、比较快
选型：标识符用 FName，操作用 FString，给玩家看用 FText

日常重点：会用 FName 做 Tag/类型判断/标识符
```

**一句话**：`Core/Public/UObject/` 的核心是 **FName** —— 全局唯一的名字标识符，比较 O(1) 极快。日常拿它做 **Tag、类型判断、标识符**，比 FString 快得多。注意它**不可变、大小写不敏感、别高频创建新名字**。

---

## 九、什么时候深入？

- **现在**：会用 FName 做标识符/Tag/判断，知道它快、不可变
- **做资源/蓝图命名系统**时：深入全局名字表、UnrealNames
- **做存档兼容**时：看 `*ObjectVersion.h`

> 下一篇可以学 **CoreUObject 模块**（完整的 UObject 类、反射、GC），那是"真正的 UObject"，也是 UE 从基础工具迈向游戏对象的关键一步。
