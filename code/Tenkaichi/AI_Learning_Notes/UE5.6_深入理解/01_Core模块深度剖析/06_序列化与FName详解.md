# 06 — 序列化（Serialization/）与 FName（UObject/）

> **定位**：Core 目录结构里剩下的两个子目录——`Serialization/`（序列化）和 `UObject/`（FName 等名字系统）。
>
> **一句话**：序列化 = **"把对象存起来 / 读出来"**（存盘、读档、网络传输、加载资源都靠它）；FName = **"全局唯一的名字标识符"**（比 FString 快得多）。
>
> **文件**：
> - 序列化：`Core/Public/Serialization/`（`Archive.h`、`MemoryReader.h`、`MemoryWriter.h`、`BufferArchive.h`）
> - FName：`Core/Public/UObject/NameTypes.h`、`Core/Private/UObject/UnrealNames.cpp`

---

## 一、Serialization/ 是什么（先搞懂"序列化"这个概念）

**序列化（Serialization）** = 把一个**内存里的对象**，转成**可以保存/传输的数据**（比如写到文件、发到网络）。反序列化 = 反过来，把数据读回内存对象。

```
内存对象 ──序列化──→ 字节/文件（保存起来、发出去）
字节/文件 ──反序列化──→ 内存对象（读回来、重建）
```

**具体场景**：游戏存档就是序列化！

```cpp
// 存档：把玩家数据"序列化"写进文件
Player->SaveToFile();   // 内部把血量、背包、位置等写到磁盘

// 读档：把文件"反序列化"读回内存
Player->LoadFromFile(); // 内部把磁盘数据重建回对象
```

**UE 里到处都是序列化**：保存关卡、加载 .uasset 资源、网络同步角色位置、存档系统……全都依赖它。

---

## 二、FArchive —— 序列化的统一抽象（核心中的核心）

`FArchive` 是序列化世界的"总接口"，**所有读写都通过它**。它像一根"管道"，数据从对象流进管道 → 存到目标（内存/文件/网络）。

```cpp
// FArchive 是所有序列化操作的总抽象
class FArchive {
public:
    virtual FArchive& operator<<(int32& Value) = 0;   // 读写 int
    virtual FArchive& operator<<(float& Value) = 0;   // 读写 float
    virtual FArchive& operator<<(FString& Str) = 0;   // 读写字符串
    // ...每个类型都能 << 进/出管道
};
```

**关键**：`operator<<` 是序列化的核心操作符。所有类型都实现"怎么把自己 `<<` 进管道"。

### 几个常用的 FArchive 子类

| 子类 | 作用 | 场景 |
|------|------|------|
| `FMemoryReader` | 从**内存**读 | 读取一段内存数据 |
| `FMemoryWriter` | 写进**内存** | 生成一段字节 |
| `FBufferArchive` | 内存缓冲区读写 | 通用内存序列化 |
| `FArchiveSave` | 存到**文件** | 存档写盘 |
| `FArchiveLoad` | 从**文件**读 | 读档 |

### 具体场景：用 FBufferArchive 序列化

```cpp
// 把玩家数据存成字节（序列化）
FBufferArchive ToBytes;
ToBytes << PlayerName;      // 名字进管道
ToBytes << Health;          // 血量进管道
ToBytes << Position;        // 位置进管道
// 现在 ToBytes 就是一段字节，可写进文件/发网络

// 从字节读回（反序列化）
FMemoryReader FromBytes(ToBytes);
FromBytes << PlayerName;    // 名字读出来
FromBytes << Health;        // 血量读出来
```

**为什么重要**：理解 FArchive，就理解了 UE 的**存档、网络复制、资源加载**三大系统的底层原理——它们全是"对象 `<<` 进管道"。

---

## 三、UObject/ 目录里是什么？—— 注意！不是完整 UObject

`01` 的目录图里，`Runtime/Core/` 下有个 `UObject/` 目录。**这里的 `UObject/` 和完整的 UObject 系统（CoreUObject 模块）不是一回事**。

```
Core/Public/UObject/   ← 只有最基础的"名字系统"
   ├── NameTypes.h     ← FName 定义
   └── UnrealNames.cpp ← FName 全局名字表

CoreUObject/Public/UObject/  ← 完整 UObject（UObject 类、反射、GC）
   └── Object.h 等
```

**Core 里的 `UObject/` 只放 `FName`**，因为 FName 太基础，连 Core 都要用，所以放在 Core 里。完整的 UObject 类在 CoreUObject 模块。

---

## 四、FName —— 全局唯一的名字标识符

### 什么是 FName

`FName` 是一个**全局唯一的名字**。同样内容的字符串，在整个引擎里只存一份，用一个**整数索引**代表它。

```
FName("Player")  → 存进全局名字表，返回一个 int 索引
FName("Player")  → 查找名字表，发现已有，返回同一个 int
```

**关键**：`FName("Player") == FName("Player")` 比较的是**那个 int 索引**，不是字符串内容 → **O(1) 极快**。

### FName vs FString（回忆 01 的内容）

| | FName | FString |
|---|---|
| 本质 | 全局唯一，int 索引 | 可变的字符数组 |
| 比较速度 | **O(1) 极快** | O(n) 逐字符 |
| 可改吗 | ❌ 不可变 | ✅ 可拼接修改 |
| 用途 | 标识符（Tag、资源名、属性名） | 通用字符串操作 |

### 具体场景：给敌人打 Tag、判断类型

```cpp
// 用 FName 做标识符，比较极快
FName EnemyType = TEXT("Boss");

// 判断敌人是不是 Boss（FName 比较快，适合频繁判断）
if (Enemy->GetType() == TEXT("Boss")) {
    // 是 Boss，触发 Boss 逻辑
}
```

### 注意 FName 的一个坑：大小写不敏感

```cpp
FName("Foo") == FName("foo");   // true！FName 默认忽略大小写
```

如果必须区分大小写，用 `IsEqual(B, ENameCompareFlags::CaseSensitive)`。

---

## 五、序列化 + FName 的关系（它们怎么配合）

存档时，对象里的 **FName 也要序列化**：

```cpp
// FName 也能被 FArchive 序列化（因为它太基础，连 Core 的序列化都支持它）
FBufferArchive Bytes;
Bytes << SomeFName;   // FName 存进管道
```

**为什么 FName 放在 Core 的 UObject/ 里**：因为序列化、容器等 Core 基础功能都要用 FName，所以它必须放在最底层的 Core，不能放在依赖 Core 的 CoreUObject 里。

---

## 六、日常开发要学吗？—— 分轻重

| 主题 | 日常用得勤吗 | 建议 |
|------|:---:|------|
| `FName` | **非常常用** | **必学**（标识符、Tag、比较快） |
| `FString` | 常用 | 已学 |
| `FArchive` | 偶尔（做存档） | **了解**原理，做存档时深入 |
| 自定义序列化 | 少 | 遇到再做 |

**结论**：
1. **FName 是必学重点**——它是 UE 的标识符，到处都用
2. **FArchive/序列化**——理解"对象 `<<` 进管道"这个概念即可，做存档系统时再深入
3. `Serialization/` 和 `UObject/` 目录：**知道它们是"序列化工具"和"FName"就行**

---

## 七、总结速查

```
Serialization/  = 序列化工具（把对象存起来/读出来）
  核心：FArchive（operator<< 管道抽象）
  子类：FMemoryReader/Writer、FBufferArchive、文件读写
  用途：存档、读档、网络传输、加载资源

UObject/（Core 里的）= FName 名字系统
  FName = 全局唯一名字标识符，int 索引，比较 O(1) 极快
  特点：不可变、大小写不敏感
  用途：Tag、资源名、属性名、类型判断

日常重点：FName 必学（到处用）
        FArchive 了解原理，做存档再深入
```

**一句话**：`Serialization/` = "把对象存进文件/读出来的工具（FArchive）"；`UObject/`（Core 里的）= "FName 名字系统"。日常你重点学 **FName**，序列化理解概念、做存档时再深入。

---

## 八、什么时候深入？

- **现在**：掌握 FName（必学），了解 FArchive 是"序列化抽象"即可
- **做存档/读档系统**时：深入研究 FArchive、序列化操作符
- **做网络同步**时：理解对象如何 `<<` 进网络管道

> 和其他笔记一样：**先会用 FName（日常高频），再深入 FArchive（用到再学）**。
