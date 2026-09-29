# 07 — HAL 硬件抽象层详解

> **定位**：`Core/Public/HAL/`，HAL = **Hardware Abstraction Layer（硬件抽象层）**。它是引擎和操作系统之间的"翻译层"，让你用**同一套代码**操作 Windows/Linux/Mac/iOS/Android 所有平台。
>
> **一句话**：HAL 帮你封装了"操作系统底层的事"——线程、锁、文件、内存、时间、进程。你**不用写 `#ifdef _WIN32`**，UE 替你处理了。
>
> **文件**：`Engine/Source/Runtime/Core/Public/HAL/`（93 个文件，Core 里最大的目录之一）

---

## 一、HAL 到底解决什么问题（先搞懂动机）

你写游戏想跨平台，但操作系统不一样：

| 事情 | Windows | Linux |
|------|---------|-------|
| 创建线程 | `CreateThread()` | `pthread_create()` |
| 文件操作 | `CreateFile()` | `open()` |
| 拿当前时间 | `GetSystemTime()` | `gettimeofday()` |
| 加锁 | `EnterCriticalSection()` | `pthread_mutex_lock()` |

**如果不用 HAL**，你写一行就要 `#ifdef` 一次：

```cpp
// ❌ 不封装：到处是平台判断，代码爆炸
#ifdef _WIN32
    CreateThread(...);
#elif __linux__
    pthread_create(...);
#endif
```

**HAL 的价值**：把这些全封装成**统一的 UE API**，你只管调用，不管平台：

```cpp
// ✅ 用 HAL：一行搞定，UE 内部自动选平台实现
FRunnableThread::Create(...);   // 创建线程，Windows/Linux 都能跑
```

---

## 二、HAL/ 目录里有什么（按功能分类）

```
Core/Public/HAL/
├── 线程相关：Runnable.h、Thread.h、Event.h、PlatformProcess.h
├── 锁相关：  CriticalSection.h、PlatformAtomics.h
├── 文件相关：FileManager.h、PlatformFile.h
├── 内存相关：MemoryBase.h、UnrealMemory.h、PlatformMemory.h
├── 时间相关：PlatformTime.h、PlatformMisc.h
├── 平台接口：Platform.h（平台检测）、PlatformMisc.h、PlatformString.h
└── ...（一堆各平台的实现头文件）
```

**你日常会碰的**主要就这几个：

| 文件 | 干什么的 |
|------|---------|
| `Runnable.h` | 多线程入口（FRunnable） |
| `Thread.h` | 线程抽象（FRunnableThread） |
| `CriticalSection.h` | 临界区/互斥锁（FCriticalSection） |
| `FileManager.h` | 文件操作（IFileManager） |
| `UnrealMemory.h` | 内存分配（FMemory::Malloc） |
| `Platform.h` | 平台检测宏（PLATFORM_WINDOWS） |
| `PlatformTime.h` | 时间（FPlatformTime::Seconds） |
| `PlatformProcess.h` | 进程、线程创建 |

---

## 三、重点讲解（配具体场景）

### 3.1 平台检测宏（Platform.h）—— 判断当前平台

```cpp
#if PLATFORM_WINDOWS
    // Windows 专属逻辑
#elif PLATFORM_ANDROID
    // Android 专属逻辑
#endif
```

**具体场景**：移动端和 PC 端表现不同（比如移动端降低画质）：

```cpp
void ApplySettings() {
    if (PLATFORM_ANDROID || PLATFORM_IOS) {
        SetQuality(ELowQuality);   // 手机上用低画质
    } else {
        SetQuality(EHighQuality);  // PC 上用高画质
    }
}
```

### 3.2 文件操作（FileManager）—— 不用管平台差异

```cpp
// 不需要 #ifdef，UE 帮你处理 Windows/Linux 的文件路径差异
IFileManager& FM = IFileManager::Get();
FM.Copy(TEXT("Dest.txt"), TEXT("Src.txt"), false, true);  // 复制
FM.Delete(TEXT("Temp.txt"));                               // 删除
FM.Move(TEXT("New.txt"), TEXT("Old.txt"), false);          // 移动
```

**具体场景**：保存玩家设置到文件：

```cpp
void SaveSettings() {
    IFileManager& FM = IFileManager::Get();
    FM.WriteFile(TEXT("Save/Settings.txt"), SettingsString);
}
```

### 3.3 多线程（Runnable / Thread）—— 后台干活

```cpp
// 定义一个可运行的任务
class FLoadTask : public FRunnable {
public:
    virtual uint32 Run() override {
        // 后台加载资源
        return 0;
    }
};

// 创建线程跑它
FRunnableThread::Create(new FLoadTask(), TEXT("LoadThread"));
```

**具体场景**：加载地图/资源不能卡主线程，放到后台线程：

```cpp
// 主线程不卡，加载在线程里做
FRunnableThread::Create(new FAssetLoader(), TEXT("AssetLoader"));
```

### 3.4 互斥锁（CriticalSection）—— 保护共享数据

多线程访问同一个变量会冲突，用锁保护：

```cpp
class FScoreManager {
    FCriticalSection Mutex;   // 锁
    int32 Score = 0;
public:
    void AddScore(int32 N) {
        FScopeLock Lock(&Mutex);   // 加锁
        Score += N;                 // 安全修改
        // 离开作用域自动解锁
    }
};
```

**具体场景**：两个线程同时给玩家加分，不加锁会丢分数：

```cpp
// 战斗线程 + UI 线程同时改 Score → 必须加锁
FScopeLock Lock(&Mutex);
Score += Damage;
```

### 3.5 内存分配（UnrealMemory）—— 统一内存接口

```cpp
void* P = FMemory::Malloc(1024);   // 分配
FMemory::Free(P);                   // 释放
FMemory::Memzero(Buffer, Size);     // 清空
FMemory::Memcpy(Dest, Src, Size);   // 拷贝
```

**具体场景**：虽然日常用容器不用手动管内存，但读引擎/插件代码时经常见到 FMemory 调用，知道它是"统一内存接口"即可。

### 3.6 时间（PlatformTime）—— 高性能计时

```cpp
double Start = FPlatformTime::Seconds();   // 高精度时间（秒）
// ...干点活
double Elapsed = FPlatformTime::Seconds() - Start;  // 耗时
```

**具体场景**：测某段代码性能（比 UE_LOG 计时更准）：

```cpp
double T0 = FPlatformTime::Seconds();
ProcessFrame();   // 要测的代码
UE_LOG(LogTemp, Log, TEXT("帧处理耗时: %.4f 秒"), FPlatformTime::Seconds() - T0);
```

---

## 四、总结：HAL 的"抽象"思想

HAL 的核心模式是：**接口统一 + 平台各自实现**。

```
你的代码
   │ 调用统一接口（IFileManager / FRunnable / FMemory）
   ▼
HAL 抽象层
   │ 根据当前平台选实现
   ▼
Windows实现    Linux实现    Android实现    ...
   │
   ▼
操作系统
```

**好处**：
1. **跨平台**：一套代码，所有平台能跑
2. **换平台不用改游戏代码**：只要 UE 支持，你的代码不用动
3. **统一管理**：内存、文件、线程都走 UE 的体系

---

## 五、日常开发要学吗？—— 分轻重

| 主题 | 日常用得勤吗 | 建议 |
|------|:---:|------|
| `FPlatformMisc` / 平台检测 | 偶尔 | 了解 `PLATFORM_*` 宏即可 |
| `IFileManager`（文件） | 做存档/读写用 | **常用**，记住基本操作 |
| `FRunnable`（多线程） | 少见 | 做异步加载再学 |
| `FCriticalSection`（锁） | 少见 | 多线程才用 |
| `FMemory` | 很少 | 认识即可 |
| `FPlatformTime` | 计时用 | 记住 `Seconds()` |

**结论**：
1. **日常最常碰的是 `IFileManager`（文件）** 和平台检测宏
2. 多线程（FRunnable/FCriticalSection）**等真需要异步时再学**
3. 大部分 HAL 的东西，**理解"跨平台抽象"这个思想就够**，不用逐个背

---

## 六、常见陷阱

**① 多线程改共享变量不加锁 → 数据竞争/崩溃**
```cpp
// ❌ 两个线程同时写 Score，丢数据
Score += Damage;
// ✅ 用锁保护
FScopeLock Lock(&Mutex);
Score += Damage;
```

**② 用平台 API 而不是 UE 的**
```cpp
// ❌ 直接用 Windows 的，跨平台就崩
CreateThread(...);
// ✅ 用 UE 的
FRunnableThread::Create(...);
```

**③ 主线程做耗时操作 → 卡顿**
```cpp
// ❌ 主线程加载大资源，卡顿
LoadHugeAsset();
// ✅ 放后台线程
FRunnableThread::Create(new FAssetLoader(), TEXT("Loader"));
```

---

## 七、总结速查

```
HAL = 硬件抽象层（跨平台"翻译层"）
├── 平台检测：PLATFORM_WINDOWS / PLATFORM_ANDROID ...
├── 文件：    IFileManager::Get().Copy/Delete/Move
├── 线程：    FRunnable + FRunnableThread::Create
├── 锁：      FCriticalSection + FScopeLock
├── 内存：    FMemory::Malloc/Free/Memcpy
├── 时间：    FPlatformTime::Seconds()
└── 进程：    FPlatformProcess

日常重点：IFileManager（文件）、PLATFORM_*（平台判断）
进阶：多线程（FRunnable）、锁（FCriticalSection）
思想：接口统一 + 平台各自实现 = 跨平台
```

**一句话**：HAL 让"写一次代码，所有平台都能跑"。你日常用 `IFileManager`（文件）和平台检测宏最多，多线程/锁等真需要异步时再学，理解"跨平台抽象"的思想就够。

---

## 八、什么时候深入？

- **现在**：记住 `IFileManager`（文件）和 `PLATFORM_*`（平台判断），理解"跨平台"思想
- **做异步加载/多线程**时：深入 FRunnable、FCriticalSection
- **做存档**时：用 IFileManager 读写文件
- **优化性能**时：用 FPlatformTime 计时

> 和其他笔记一样：**先会用高频的（文件、平台判断），再深入冷门的（多线程、锁）**。
