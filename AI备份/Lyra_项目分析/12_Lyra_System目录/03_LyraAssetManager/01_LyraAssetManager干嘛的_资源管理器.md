# `ULyraAssetManager` 干嘛的？—— 资源管理器

> **定位**：只讲 `System/LyraAssetManager.h`（`ULyraAssetManager` 类的头文件，含内联的模板实现）。
>
> **原代码**（`LyraAssetManager.h` 核心）：
> ```cpp
> UCLASS(MinimalAPI, Config = Game)
> class ULyraAssetManager : public UAssetManager
> {
>     GENERATED_BODY()
> public:
>     static UE_API ULyraAssetManager& Get();          // 单例
>
>     // 拿软引用指向的资源（没加载就同步加载）
>     template<typename AssetType>
>     static AssetType* GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);
>
>     // 拿软引用指向的类（没加载就同步加载）
>     template<typename AssetType>
>     static TSubclassOf<AssetType> GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);
>
>     static UE_API void DumpLoadedAssets();           // 打印已加载资源
>
>     UE_API const ULyraGameData& GetGameData();       // 拿全局游戏数据
>     UE_API const ULyraPawnData* GetDefaultPawnData() const;  // 拿默认 Pawn 数据
>     ...
> };
> ```
>
> **一句话**：`ULyraAssetManager` 是 Lyra 的**资源管理器**，继承引擎的 `UAssetManager`，是"资源加载的总管"——专门把软引用（`TSoftObjectPtr`/`TSoftClassPtr`）解析成真正加载好的资源，并统一管理全局数据资产（GameData / PawnData）的加载与缓存。

---

## 一、它是什么

| 问题 | 答案 |
|------|------|
| 它是什么？ | 一个**资源管理器**（继承 `UAssetManager`） |
| 核心职责 | 把**软引用**变成真正加载好的资源 + 管理全局数据资产 |
| 谁用它？ | 全项目任何需要加载资源的地方，通过 `ULyraAssetManager::Get()` |
| 几个实例？ | **全局唯一**（单例，`Get()` 拿到同一个） |
| 怎么被启用？ | 在 `DefaultEngine.ini` 里设 `AssetManagerClassName` 指向它 |

> **核心**：它是 Lyra 所有"资源加载"的总管。软引用只存路径不自动加载，真正要用时，靠它来"解析 + 加载 + 缓存"。

---

## 二、类定义逐行拆解

### 2.1 继承 `UAssetManager` —— 引擎的资源总管

```cpp
class ULyraAssetManager : public UAssetManager
```

- `UAssetManager` = UE 引擎内置的**资源管理器基类**（`#include "Engine/AssetManager.h"`）。
- 它负责管理所有 `UPrimaryDataAsset`、按 ID 加载资源、异步/同步加载调度等。
- Lyra 继承它，加上游戏专属逻辑（管理 LyraGameData、LyraPawnData 等）。

> 注释里明确说了：**大多数游戏都应该重写 AssetManager**，因为它是放"游戏专属加载逻辑"的好地方。

### 2.2 `UCLASS(MinimalAPI, Config = Game)`

| 说明符 | 含义 |
|--------|------|
| `MinimalAPI` | 只导出标 `UE_API` 的接口，减小模块耦合 |
| `Config = Game` | 能用 `DefaultGame.ini` 里的配置（如 `LyraGameDataPath` 路径） |

### 2.3 怎么被启用？（注释里的关键信息）

```cpp
// This class is used by setting 'AssetManagerClassName' in DefaultEngine.ini.
```

- 在 `DefaultEngine.ini` 里有一行 `AssetManagerClassName=/Script/LyraGame.LyraAssetManager`。
- 引擎启动时读这个配置，知道"用 Lyra 的这个资产管理器"，而不是默认的。
- 这就是"接管引擎资源系统"的方式——**配置注入**。

---

## 三、核心接口（头文件里的 public 函数）

### 3.1 `static Get()` —— 全局单例

```cpp
// Returns the AssetManager singleton object.
static UE_API ULyraAssetManager& Get();
```

- 返回单例引用，全项目用 `ULyraAssetManager::Get()` 调。

### 3.2 `GetAsset` —— 软引用 → 加载好的对象

```cpp
template<typename AssetType>
static AssetType* GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);
```

- **作用**：传入一个**软对象引用**（`TSoftObjectPtr`，只存路径），返回**真正加载好的对象指针**。
- **没加载就同步加载**（注释：This will synchronously load the asset if it's not already loaded）。
- `bKeepInMemory = true`：加载后**保持在内存**（加进 `LoadedAssets` 集合，防止被 GC）。

> **为什么需要它**：软引用（`TSoftObjectPtr`）省内存但"用时才加载"。这个函数就是"把软引用兑现成硬资源"的那一步。

### 3.3 `GetSubclass` —— 软类引用 → 加载好的类

```cpp
template<typename AssetType>
static TSubclassOf<AssetType> GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);
```

- **作用**：和 `GetAsset` 一样，但针对**类**（`TSoftClassPtr`）。
- 返回 `TSubclassOf<AssetType>`（一个已加载的 `UClass`，可直接用来 `SpawnActor` 或施加 GE）。

> `GetAsset` 拿"对象实例"，`GetSubclass` 拿"类"。上一篇文章里 `LyraGameData` 存的就是 `TSoftClassPtr<UGameplayEffect>`，用的就是 `GetSubclass` 来加载那个 GE 类。

### 3.4 `DumpLoadedAssets` —— 调试用

```cpp
// Logs all assets currently loaded and tracked by the asset manager.
static UE_API void DumpLoadedAssets();
```

- 打印当前所有已加载、被管理的资源（调试/排查内存用）。

### 3.5 `GetGameData` / `GetDefaultPawnData` —— 拿全局数据资产

```cpp
UE_API const ULyraGameData& GetGameData();              // 拿全局游戏数据（只读）
UE_API const ULyraPawnData* GetDefaultPawnData() const; // 拿默认 Pawn 数据
```

- `GetGameData()`：返回 `ULyraGameData`（上一篇讲的那个全局数据资产），内部走"取或加载"逻辑。
- `GetDefaultPawnData()`：返回默认 Pawn 配置（生成玩家 Pawn 时若 PlayerState 没指定就用它）。

> 这两个函数让"全局数据资产"的加载也归资产管理器统一管——不用各处自己加载。

---

## 四、头文件内联的模板实现（重点）★

头文件里**直接实现了两个模板函数**（因为 C++ 模板必须把实现放头文件，否则编译期看不到）。这是理解它怎么工作的关键。

### 4.1 `GetAsset` 的实现逻辑

```cpp
template<typename AssetType>
AssetType* ULyraAssetManager::GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
    AssetType* LoadedAsset = nullptr;
    const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

    if (AssetPath.IsValid())
    {
        LoadedAsset = AssetPointer.Get();              // ① 先试直接取（可能已加载）
        if (!LoadedAsset)
        {
            LoadedAsset = Cast<AssetType>(SynchronousLoadAsset(AssetPath));  // ② 没加载就同步加载
            ensureAlwaysMsgf(LoadedAsset, TEXT("Failed to load asset [%s]"), ...);
        }

        if (LoadedAsset && bKeepInMemory)
        {
            Get().AddLoadedAsset(Cast<UObject>(LoadedAsset));  // ③ 记进"已加载"集合，防 GC
        }
    }
    return LoadedAsset;
}
```

**三步走**：
1. **先试取**：`AssetPointer.Get()`——如果资源已经在内存，直接返回（不重复加载）。
2. **没加载就同步加载**：`SynchronousLoadAsset(AssetPath)`——阻塞式加载，直到加载完。
3. **保持内存**：`AddLoadedAsset()`——加进 `LoadedAssets` 集合（用 `bKeepInMemory` 控制），防止被垃圾回收。

> **这就是"软引用 → 硬资源"的完整过程**：取 → 没有就加载 → 记起来别被回收。

### 4.2 `GetSubclass` 的实现逻辑

```cpp
template<typename AssetType>
TSubclassOf<AssetType> ULyraAssetManager::GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
    ...
    LoadedSubclass = AssetPointer.Get();              // ① 先试取
    if (!LoadedSubclass)
    {
        LoadedSubclass = Cast<UClass>(SynchronousLoadAsset(AssetPath));  // ② 没加载就同步加载
        ...
    }
    if (LoadedSubclass && bKeepInMemory)
    {
        Get().AddLoadedAsset(Cast<UObject>(LoadedSubclass));  // ③ 保持内存
    }
    return LoadedSubclass;
}
```

- 和 `GetAsset` 逻辑完全一样，只是类型从"对象"换成"类"（`TSubclassOf` / `UClass`）。

### 4.3 两个模板的共同套路

```
软引用（只存路径）
   ↓ Get() 先试取
已加载？→ 直接返回
   ↓ 没有
同步加载 SynchronousLoadAsset
   ↓
加进 LoadedAssets 防 GC（bKeepInMemory）
   ↓
返回硬资源
```

> **一句话**：这两个模板就是"软引用的兑现器"——把延迟加载的软引用，在真正需要时同步加载成硬资源，并可选地常驻内存。

---

## 五、protected 成员（头文件能看出的设计）

### 5.1 全局数据资产的路径与缓存

```cpp
// Global game data asset to use.
UPROPERTY(Config)
TSoftObjectPtr<ULyraGameData> LyraGameDataPath;      // 路径（从 ini 配置读）

// Loaded version of the game data
UPROPERTY(Transient)
TMap<TObjectPtr<UClass>, TObjectPtr<UPrimaryDataAsset>> GameDataMap;  // 已加载缓存
```

- `LyraGameDataPath`：`Config` = 从 `DefaultGame.ini` 读路径（软引用，不强制加载）。
- `GameDataMap`：`Transient` = 运行时缓存（不序列化），按类型存已加载的数据资产，避免重复加载。

> **套路**：路径（软引用）+ 缓存（Map）——先查缓存，没有再按路径加载，加载后存缓存。这就是 `GetOrLoadTypedGameData` 的基础。

### 5.2 默认 Pawn 数据

```cpp
// Pawn data used when spawning player pawns if there isn't one set on the player state.
UPROPERTY(Config)
TSoftObjectPtr<ULyraPawnData> DefaultPawnData;
```

- 同样从 ini 配路径，供 `GetDefaultPawnData()` 用。

### 5.3 `GetOrLoadTypedGameData` —— 取或加载（模板）

```cpp
template <typename GameDataClass>
const GameDataClass& GetOrLoadTypedGameData(const TSoftObjectPtr<GameDataClass>& DataPath)
{
    if (TObjectPtr<UPrimaryDataAsset> const * pResult = GameDataMap.Find(GameDataClass::StaticClass()))
    {
        return *CastChecked<GameDataClass>(*pResult);   // 缓存里有，直接返回
    }
    // 没有就阻塞加载
    return *CastChecked<const GameDataClass>(LoadGameDataOfClass(...));
}
```

- **取或加载**：先查 `GameDataMap` 缓存，有就返回；没有才 `LoadGameDataOfClass` 加载。
- 这是"全局数据资产"高效加载的核心——**加载一次，缓存复用**。

---

## 六、启动流程相关（头文件能看出的骨架）

头文件里还有一批 `protected`/`private` 函数，勾勒出"启动时加载"的流程：

```cpp
UE_API virtual void StartInitialLoading() override;     // 引擎回调：开始初始加载
#if WITH_EDITOR
UE_API virtual void PreBeginPIE(bool bStartSimulate) override;  // 编辑器 PIE 前
#endif

private:
    UE_API void DoAllStartupJobs();                     // 执行所有启动任务
    UE_API void InitializeGameplayCueManager();         // 初始化 GAS 的 Cue 管理器
    UE_API void UpdateInitialGameContentLoadPercent(float GameContentPercent);  // 更新加载进度
    TArray<FLyraAssetManagerStartupJob> StartupJobs;    // 启动任务列表（跟踪进度）
```

- `StartInitialLoading`：重写引擎的加载入口，游戏启动时触发。
- `StartupJobs`：一个"启动任务数组"，把启动时要干的活（如加载资源、初始化系统）拆成一个个 job，逐个执行并跟踪进度。
- `UpdateInitialGameContentLoadPercent`：把加载进度喂给加载界面（loading screen）。

> **设计意图**：Lyra 把"启动加载"做成**任务队列**（StartupJobs），能显示加载进度、能喂给加载界面——这就是为什么 Lyra 启动有个进度条。

---

## 七、线程安全的资源追踪

```cpp
private:
    // Assets loaded and tracked by the asset manager.
    UPROPERTY()
    TSet<TObjectPtr<const UObject>> LoadedAssets;

    // Used for a scope lock when modifying the list of load assets.
    FCriticalSection LoadedAssetsCritical;
```

- `LoadedAssets`：记录所有被管理器加载、常驻的资源（配合 `bKeepInMemory`）。
- `LoadedAssetsCritical`：**临界区锁**——因为资源可能在**多线程**加载，修改 `LoadedAssets` 时要加锁，防止数据竞争。

> **细节**：`AddLoadedAsset` 注释写了"Thread safe way of adding a loaded asset"——用临界区保证并发安全。这是生产级代码的严谨之处。

---

## 八、和上一篇 `LyraGameData` 的关系

上一篇 `ULyraGameData` 存的是 `TSoftClassPtr<UGameplayEffect>`（软引用），**本身不加载**。这一篇的 `ULyraAssetManager` 就是**负责把它加载出来**的：

```
ULyraGameData（存软引用）
     ↓ 需要用时
ULyraAssetManager::GetSubclass(软引用)   ← 本篇
     ↓ 同步加载
真正加载好的 UGameplayEffect 类
```

> **协作关系**：`LyraGameData` 是"配置抽屉"（存路径），`AssetManager` 是"加载总管"（把路径兑现成资源）。

---

## 九、JS 对照（帮你理解）

```js
// JS 里没有完全等价的资源管理器，最接近的是"懒加载 + 缓存"模式
class AssetManager {
    static #cache = new Map();        // GameDataMap（缓存）
    static #loadedAssets = new Set(); // LoadedAssets（防回收）

    // 软引用 → 加载好的资源
    static getAsset(softPtr, keepInMemory = true) {
        if (this.#cache.has(softPtr)) return this.#cache.get(softPtr);  // ① 先查缓存
        const asset = this.#load(softPtr);                              // ② 没有才加载
        if (keepInMemory) this.#loadedAssets.add(asset);               // ③ 常驻
        return asset;
    }
}
```

| 概念 | JS | C++（LyraAssetManager.h） |
|------|-----|--------------------------|
| 软引用 | 存字符串路径 | `TSoftObjectPtr` / `TSoftClassPtr` |
| 加载兑现 | `import()` / 动态加载 | `SynchronousLoadAsset` |
| 缓存 | `Map` | `GameDataMap` |
| 常驻防回收 | 持有引用（不置空） | `LoadedAssets` 集合 |
| 单例 | `AssetManager.getInstance()` | `static Get()` |

> **关键差异**：JS 的"加载"通常是异步 `import()`；UE 这里为了简单用了**同步加载**（`SynchronousLoadAsset`，阻塞等加载完），因为很多游戏逻辑需要"立刻拿到资源"。

---

## 十、常见疑问速答

| 疑问 | 答案 |
|------|------|
| AssetManager 是什么？ | UE 的资源管理总管，管所有主数据资产的加载 |
| Lyra 为什么要重写它？ | 加游戏专属加载逻辑（管理 GameData/PawnData、启动任务队列） |
| 怎么启用这个管理器？ | `DefaultEngine.ini` 里设 `AssetManagerClassName` |
| 软引用和这个管理器啥关系？ | 软引用只存路径，靠管理器来"兑现"成加载好的资源 |
| `GetAsset` 和 `GetSubclass` 区别？ | 前者拿对象实例，后者拿类（`UClass`） |
| `bKeepInMemory` 干嘛的？ | true 时把加载的资源加进 `LoadedAssets`，防止被 GC |
| 为什么模板实现放头文件？ | C++ 模板必须在头文件里实现，编译期才可见 |
| `GameDataMap` 和 `LyraGameDataPath` 区别？ | 前者是已加载缓存，后者是待加载的路径（软引用） |
| `StartInitialLoading` 什么时候调？ | 引擎启动初始加载阶段（重写的回调） |
| `LoadedAssetsCritical` 是啥？ | 临界区锁，保证多线程加载时 `LoadedAssets` 的并发安全 |

---

## 十一、总结

```
ULyraAssetManager = Lyra 的资源管理总管（继承 UAssetManager）

是什么：
  继承引擎 UAssetManager，全局单例（Get()）
  在 DefaultEngine.ini 里配置启用

核心职责：
  1. 把软引用（TSoftObjectPtr/TSoftClassPtr）兑现成加载好的资源
     - GetAsset()    → 拿对象
     - GetSubclass() → 拿类
     套路：先查缓存 → 没有就同步加载 → 加进 LoadedAssets 防 GC
  2. 统一管理全局数据资产（GameData / PawnData）的加载与缓存
     - GetGameData() / GetDefaultPawnData()
     - GetOrLoadTypedGameData()：取或加载，缓存复用

启动流程：
  StartInitialLoading → StartupJobs 任务队列 → 逐个执行 + 显示进度

严谨细节：
  LoadedAssetsCritical 临界区锁 → 多线程加载并发安全

和 LyraGameData 的关系：
  LyraGameData 存软引用（配置抽屉）
  AssetManager 负责加载（加载总管）
```

**一句话**：`ULyraAssetManager` 是 Lyra 的**资源加载总管**（继承引擎 `UAssetManager`，单例，ini 配置启用），核心是把**软引用兑现成加载好的资源**——`GetAsset` 拿对象、`GetSubclass` 拿类，套路都是"先查缓存 → 没有就同步加载 → 加进 `LoadedAssets` 防 GC"；同时统一管理 GameData/PawnData 等全局数据资产的加载缓存，并用 `StartupJobs` 任务队列驱动启动加载、显示进度。

---

## 十二、下一步

- 看 `.cpp` 里 `StartInitialLoading`、`DoAllStartupJobs` 具体怎么跑启动任务队列。
- 看 `FLyraAssetManagerStartupJob`（启动任务的结构）。
- 看 `SynchronousLoadAsset` 底层怎么调引擎的加载。
- 看 `DefaultGame.ini` 里 `LyraGameDataPath` / `DefaultPawnData` 配的具体路径。
