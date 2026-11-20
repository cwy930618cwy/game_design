# `GetOrLoadTypedGameData` —— 取或加载（模板）

> **定位**：解释 `LyraAssetManager.h` 第 57-67 行的模板函数。
>
> **原代码**：
> ```cpp
> template <typename GameDataClass>
> const GameDataClass& GetOrLoadTypedGameData(const TSoftObjectPtr<GameDataClass>& DataPath)
> {
>     // ① 先查缓存
>     if (TObjectPtr<UPrimaryDataAsset> const * pResult = GameDataMap.Find(GameDataClass::StaticClass()))
>     {
>         return *CastChecked<GameDataClass>(*pResult);   // 缓存有，直接返回
>     }
>     // ② 缓存没有，阻塞加载
>     return *CastChecked<const GameDataClass>(LoadGameDataOfClass(GameDataClass::StaticClass(), DataPath, GameDataClass::StaticClass()->GetFName()));
> }
> ```
>
> **一句话**：这是一个"**取或加载**"的模板函数——先查缓存（`GameDataMap`），有就直接返回；没有才去加载，加载完存缓存。**加载一次，缓存复用**。它是 `GetGameData()` 等函数的底层实现。

---

## 一、这行是什么

```cpp
template <typename GameDataClass>
const GameDataClass& GetOrLoadTypedGameData(const TSoftObjectPtr<GameDataClass>& DataPath)
```

三个关键词：**模板**、**取或加载**、**返回引用**。

| 部分 | 含义 |
|------|------|
| `template <typename GameDataClass>` | 模板——传入什么类型，就处理什么类型 |
| `const GameDataClass&` | 返回引用（非空、不用判空） |
| `GetOrLoadTypedGameData` | "取或加载"——核心逻辑 |
| `TSoftObjectPtr<GameDataClass>` | 传入软引用（只存路径，不强制加载） |

> 它的职责：给我一份某类型的数据资产——**有缓存就返回缓存，没有就加载一份**。

---

## 二、"取或加载"是什么模式？—— 缓存的经典套路

这是编程里非常常见的**缓存模式**（cache-aside）：

```
要一份数据
   ↓
缓存里有？
   ├─ 有  → 直接返回缓存（快，不重复加载）
   └─ 没有 → 加载一份 → 存进缓存 → 返回
```

**好处**：
- **避免重复加载**：同一个数据资产，第一次加载后存缓存，之后再要直接给缓存的。
- **省时间**：加载资源（尤其磁盘 IO）很慢，缓存命中就秒回。
- **省内存**：不会同一个资源加载好几份。

> **类比**：像"冰箱 vs 买菜"——家里冰箱（缓存）有菜就直接拿，没有才去菜市场（加载）买，买回来放冰箱。

---

## 三、逐行拆解实现

### 3.1 第一步：先查缓存

```cpp
if (TObjectPtr<UPrimaryDataAsset> const * pResult = GameDataMap.Find(GameDataClass::StaticClass()))
{
    return *CastChecked<GameDataClass>(*pResult);
}
```

- `GameDataMap`：缓存表（`TMap`），按**类型**存已加载的数据资产。
- `GameDataClass::StaticClass()`：拿到这个类型的"类型信息"（作为缓存的 key）。
- `GameDataMap.Find(...)`：用类型去缓存里找。
- **找到了**（`pResult` 非空）→ `CastChecked<GameDataClass>` 转成具体类型 → 返回引用。

> **命中缓存**：直接返回，不加载，秒回。

### 3.2 第二步：缓存没有，才加载

```cpp
return *CastChecked<const GameDataClass>(
    LoadGameDataOfClass(GameDataClass::StaticClass(), DataPath, GameDataClass::StaticClass()->GetFName())
);
```

- 缓存没命中，调 `LoadGameDataOfClass(...)`——真正去**阻塞加载**资源（按 `DataPath` 路径）。
- `LoadGameDataOfClass` 内部会加载资源、存进 `GameDataMap` 缓存（所以下次就能命中）。
- 返回加载好的对象的引用。

> **未命中**：加载一份（慢），但之后就有缓存了。

### 3.3 两个 `CastChecked` 是什么？

- `CastChecked<T>` = "带断言的类型转换"——如果转换失败（类型不对）会报错崩溃，方便排查。
- 从缓存取的是基类 `UPrimaryDataAsset*`，要转成具体的 `GameDataClass*` 才能返回。

---

## 四、为什么是模板？—— 一个函数处理所有数据类型

```cpp
template <typename GameDataClass>
```

**问题**：Lyra 有多种数据资产——`ULyraGameData`、`ULyraPawnData`……每种都要写一个"取或加载"函数吗？

**模板解决**：写**一个**模板函数，传入什么类型就处理什么类型：

```cpp
GetOrLoadTypedGameData<ULyraGameData>(Path);    // 处理 GameData
GetOrLoadTypedGameData<ULyraPawnData>(Path);    // 处理 PawnData
// 同一份代码，两种类型都能用
```

> **模板 = 代码复用**。不用为每种数据类型重复写"查缓存→加载→存缓存"的逻辑。

---

## 五、它和 `GetGameData()` 的关系

上一篇讲的 `GetGameData()`（对外接口）底层就是调它：

```cpp
// GetGameData() 大致这样（对外，只读）
const ULyraGameData& ULyraAssetManager::GetGameData()
{
    return GetOrLoadTypedGameData<ULyraGameData>(LyraGameDataPath);
    //     ↑ 把路径传进去，走"取或加载"逻辑
}
```

**分工**：
- `GetGameData()` / `GetDefaultPawnData()`：**对外的具体接口**（名字明确，返回具体类型）。
- `GetOrLoadTypedGameData()`：**底层通用实现**（模板，处理"取或加载"的通用逻辑）。

> 这是"**通用逻辑下沉**"的设计——把重复的缓存逻辑抽到模板里，对外接口只管"传什么路径、返回什么类型"。

---

## 六、为什么没标 UE_API？（呼应前面学的）

注意这个函数**没标 `UE_API`**：

```cpp
template <typename GameDataClass>
const GameDataClass& GetOrLoadTypedGameData(...)   // 没有 UE_API
```

**为什么？** 因为它是 `protected` 的**内部实现**，只被**本类/本模块**的 `GetGameData()` 等调用，**外部模块不调它**。

> 回顾前面的"餐厅故事"：这是"自家厨房自用的备菜流程"，别家厨房（LyraEditor）不直接点这道菜，所以不用加"合作许可"（UE_API）。对外只暴露 `GetGameData()` 那道"成品菜"。

---

## 七、完整流程图

```
调用 GetGameData()
     ↓
GetOrLoadTypedGameData<ULyraGameData>(LyraGameDataPath)
     ↓
① 查缓存 GameDataMap.Find(ULyraGameData::StaticClass())
     ├─ 命中 → 直接返回缓存的（快）
     └─ 未命中 ↓
② LoadGameDataOfClass() 阻塞加载
     ↓ 加载时存进 GameDataMap 缓存
③ 返回加载好的引用
     ↓
（下次再调，① 就命中了）
```

---

## 八、JS 对照

```js
class AssetManager {
    #cache = new Map();  // GameDataMap

    // 取或加载（模板在 JS 里就是普通函数，类型运行时判断）
    getOrLoadTypedGameData(dataClass, dataPath) {
        if (this.#cache.has(dataClass)) {
            return this.#cache.get(dataClass);   // ① 命中缓存
        }
        const loaded = this.loadGameDataOfClass(dataPath);  // ② 加载
        this.#cache.set(dataClass, loaded);      // 存缓存
        return loaded;
    }
}
```

| 概念 | JS | C++（本函数） |
|------|-----|--------------|
| 泛型/模板 | 运行时传类型 | `template <typename>` 编译期 |
| 缓存 | `Map` | `GameDataMap` |
| 缓存 key | 类型本身 | `GameDataClass::StaticClass()` |
| 加载 | 动态 import | `LoadGameDataOfClass` |

> **关键差异**：JS 的"泛型"是运行时概念，模板参数在编译期就确定类型；C++ 模板是**编译期**生成多份代码（每种类型一份），零运行时开销。

---

## 九、常见疑问速答

| 疑问 | 答案 |
|------|------|
| 这函数干嘛的？ | 取或加载：先查缓存，没有才加载 |
| 为什么是模板？ | 一个函数处理所有数据类型（GameData/PawnData），复用逻辑 |
| 缓存在哪？ | `GameDataMap`（按类型存已加载的数据资产） |
| 缓存 key 是什么？ | `GameDataClass::StaticClass()`（类型信息） |
| 加载用什么？ | `LoadGameDataOfClass`（阻塞加载 + 存缓存） |
| 为什么返回引用？ | 保证非空，用起来方便（和 Get() 一样） |
| 为什么没 UE_API？ | protected 内部实现，外部模块不调 |
| 和 GetGameData 关系？ | GetGameData 是对外接口，底层调这个模板 |
| CastChecked 是啥？ | 带断言的类型转换，失败会报错 |

---

## 十、总结

```
GetOrLoadTypedGameData = 取或加载的模板函数（缓存模式）

核心逻辑：
  ① 先查缓存 GameDataMap
     命中 → 直接返回（快）
  ② 没命中 → LoadGameDataOfClass 加载 → 存缓存 → 返回

设计要点：
  模板      → 一个函数处理所有数据类型，复用缓存逻辑
  返回引用  → 保证非空
  protected + 无 UE_API → 内部实现，外部不调
  是 GetGameData() 等对外接口的底层实现

模式：
  经典 cache-aside（旁路缓存）：查缓存→没有才加载→存缓存
```

**一句话**：`GetOrLoadTypedGameData` 是一个**取或加载的模板函数**——先查缓存 `GameDataMap`，命中就直接返回（快），没命中才 `LoadGameDataOfClass` 阻塞加载并存缓存（加载一次，缓存复用）；用**模板**让一份代码处理所有数据类型（GameData/PawnData），是 `GetGameData()` 等对外接口的底层通用实现；因为它是 `protected` 内部实现、外部模块不调，所以没标 `UE_API`。

---

## 十一、下一步

- 看 `LoadGameDataOfClass` 具体怎么加载、怎么存进 `GameDataMap`。
- 看 `GetGameData()` / `GetDefaultPawnData()` 怎么调这个模板。
- 理解"缓存模式"在其他引擎系统的应用（如对象池、资源缓存）。
- 对比模板 vs 虚函数（编译期多态 vs 运行时多态）。
