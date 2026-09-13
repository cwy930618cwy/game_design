# `UE_API`、模块边界、`#include` 三者关系 —— 彻底理清

> **定位**：解答"为什么有的函数加 `UE_API` 有的不加？别的文件怎么用？`#include` 又是干嘛的？"这个普遍困惑。
>
> **触发疑问**：
> - 其他不加 `UE_API` 的函数，别的文件都用不了了？
> - 不是到处 `#include` class 吗？
> - 也没见别的文件加 `UE_API` 啊？
>
> **一句话**：`UE_API` 只在**跨模块**时才需要——**同模块内**随便用（所以你看不到到处加）；`#include` 只是把函数"招牌"（声明）抄进来让编译器认识，真正跨 DLL 链接到实现靠的是 `UE_API`。三者管的是不同层面，别混。

---

## 一、先把三件事分开：声明、定义、include

这是所有困惑的根源——把这三件事搞混了。

```cpp
// 【声明】（在 .h 里）—— 函数的"招牌"
UE_API ULyraAssetManager();   // 告诉编译器：有这么个函数，长这样

// 【定义/实现】（在 .cpp 里）—— 函数的"真身"
ULyraAssetManager::ULyraAssetManager()
{
    // 真正干活的代码
}
```

| 概念 | 在哪 | 作用 | 类比 |
|------|------|------|------|
| **声明** | `.h` | 函数的"招牌"（名字/参数/返回值） | 一个人的名片 |
| **定义/实现** | `.cpp` | 函数的"真身"（具体代码） | 这个人本人 |
| **`#include`** | 谁用谁写 | 把**声明**（.h）抄进来，让编译器认识 | 你手里有张名片 |

> **关键**：`#include` 只是把"招牌/名片"抄过来，让编译器**认识**这个函数。但**认识 ≠ 能真正调用到实现**——跨模块时还得能"链接"到那个真身。

---

## 二、核心：`UE_API` 只在"跨模块"时才需要

这是你晕的根源。**`UE_API` 不是到处都要加，只有跨模块才需要。**

### 2.1 什么是"模块"？

一个 `.cpp` 集合编译成一个模块（在 Windows 上是一个 DLL）。Lyra 里的模块：

```
LyraGame        ← 一个模块（大部分游戏代码都在这）
Engine          ← 另一个模块（引擎本体）
GameplayAbilities ← 又一个模块（GAS 插件）
...每个插件各是各的模块
```

### 2.2 规则（背下来）

| 调用场景 | 需要 `UE_API` 吗？ | 为什么 |
|---------|-------------------|--------|
| **同一个模块内**（都在 LyraGame） | ❌ **不需要** | 都在一个 DLL 里，符号互相可见 |
| **跨模块**（引擎调 Lyra、别的插件调 Lyra） | ✅ **需要** | 跨 DLL 必须导出符号才能链接 |

> **DLL** = 动态链接库，Windows 上模块的实体。不同 DLL 是"不同的房子"，符号默认互相看不见，必须"导出"（`UE_API`）才能被外面看到。

---

## 三、回答你的三个疑问

### 疑问 1："其他不加 UE_API，别的文件都用不了了？"

**不对**。要看"别的文件"是**同模块**还是**跨模块**：

```
【LyraGame 模块内部】（大部分代码）
  A.cpp 调 B.cpp 的函数 → 不用 UE_API，照样能用 ✅
  （同一个 DLL，符号都在，互相可见）

【Engine 模块 / 其他插件】
  调 ULyraAssetManager 的函数 → 必须 UE_API，不导出就用不了 ❌
  （跨 DLL，链接不到）
```

> 所以"别的文件"要分清楚：**同模块随便用**（这是大多数情况），**跨模块才需要 `UE_API`**。

### 疑问 2："不是到处再 #include class 吗？"

`#include` 只解决"**编译器认识它**"（语法层面），不解决"链接到实现"（链接层面）。

```cpp
// 别的文件想用
#include "LyraAssetManager.h"   // ← 把"招牌"（声明）抄进来
// 现在编译器知道有 Get() 这个函数了（语法层面 OK）

ULyraAssetManager::Get();        // ← 真正调用，要链接到 .cpp 的实现
                                 //   跨模块时，靠 UE_API 才能链接到
```

| 层面 | 靠什么解决 | 作用 |
|------|-----------|------|
| **编译**（语法） | `#include` | 让编译器"认识"这个函数（有招牌） |
| **链接**（跨模块） | `UE_API` | 让链接器"找到"这个函数的实现（有电话） |

> **类比**：`#include` = 你知道一个人的名字（有名片）；`UE_API` = 你有他的电话号码（能打通）。同模块的人在一个办公室，喊名字就行；跨模块的人在不同楼，必须有电话。

### 疑问 3："也没见别的文件加啊"

**对**——因为大部分调用都发生在 **LyraGame 模块内部**，同模块不需要 `UE_API`。

只有少数函数（`Get()`、构造函数、`GetGameData()` 等）需要被**引擎或其他模块**调用，才标 `UE_API`。

> 你看到的"到处 `#include`"是**声明层面**（让编译器认识）；"没见加 `UE_API`"是因为那些调用都在**同模块内**（不需要导出）。两件事不冲突。

---

## 四、一张图理清

```
【LyraGame 模块（一个 DLL）】
  LyraAssetManager.cpp ──实现──> 所有函数真身都在这
       ↑                          ↑
   同模块的 A.cpp            同模块的 B.cpp
   调 GetGameData()          调 SynchronousLoadAsset()
   （不用 UE_API，能用 ✅）    （不用 UE_API，能用 ✅）
   只需 #include 认识一下      只需 #include 认识一下

【Engine 模块（另一个 DLL）】
  引擎启动代码
       ↓ 要 new ULyraAssetManager()、调 Get()
       ↓ 跨模块了！必须 UE_API 导出才能链接
  ULyraAssetManager()  ← 所以构造函数要 UE_API ✅
  Get()                ← 所以单例入口要 UE_API ✅
  （其他内部函数不用，因为引擎不调）
```

---

## 五、为什么构造函数和 Get() 要 UE_API，其他不要？

回到 `LyraAssetManager.h`，看哪些标了、哪些没标：

```cpp
UE_API ULyraAssetManager();                              // ✅ 引擎要 new 它
static UE_API ULyraAssetManager& Get();                  // ✅ 外部要拿单例
static UE_API void DumpLoadedAssets();                   // ✅ 调试用，可能外部调
UE_API const ULyraGameData& GetGameData();               // ✅ 外部要拿数据
UE_API const ULyraPawnData* GetDefaultPawnData() const;  // ✅ 外部要拿 Pawn 数据

// 下面这些没标 UE_API：
const GameDataClass& GetOrLoadTypedGameData(...);        // ❌ 内部自用
UObject* SynchronousLoadAsset(...);                      // ❌ 内部自用
void AddLoadedAsset(...);                                // ❌ 内部自用
```

**规律一目了然**：
- 标 `UE_API` 的 = **外部模块要调**的接口（构造函数、单例、拿数据的）
- 没标的 = **本模块内部自用**的辅助函数

> **判断标准**：问自己"这个函数会不会被 LyraGame 以外的模块调用？"会 → 加 `UE_API`；不会 → 不加。

---

## 六、`UE_API` 到底是什么（展开）

```cpp
#define UE_API LYRAGAME_API    // 文件开头定义
```

`LYRAGAME_API` 在不同平台展开不同：

| 平台 | 展开成 | 含义 |
|------|--------|------|
| Windows | `__declspec(dllexport)` | 导出符号，让别的 DLL 能用 |
| Mac/Linux | `__attribute__((visibility("default")))` | 同上（符号可见性） |
| 被其他模块 include 时 | `__declspec(dllimport)` | 告诉编译器"这是从别的 DLL 导入的" |

> 所以 `UE_API` 是个"智能宏"——同一个标记，编译本模块时是"导出"，被别的模块引用时自动变"导入"。这就是跨模块链接的机制。

---

## 七、JS 对照（帮你理解）

```js
// JS 没有 DLL 概念，最接近的是"模块导出"
// LyraAssetManager.js
export class LyraAssetManager {   // ← export = UE_API
    static get() { /* ... */ }
    getGameData() { /* ... */ }
    
    // 不 export 的 = 内部自用
    #syncLoadAsset() { /* ... */ }  // 私有方法
}

// 别的文件
import { LyraAssetManager } from './LyraAssetManager.js';  // ← #include
LyraAssetManager.get();  // 用导出的
```

| 概念 | JS | C++（UE） |
|------|-----|----------|
| 让外部能用 | `export` | `UE_API` |
| 内部自用 | 不 export / `#` 私有 | 不标 `UE_API` |
| 引用别人 | `import` | `#include` + 链接 |
| 层面 | 模块系统（运行时） | 编译/链接（编译期+链接期） |

> **关键差异**：JS 的 `export/import` 是**运行时模块系统**；C++ 的 `UE_API` 是**编译/链接层面**的符号导出，直接决定"别的 DLL 能不能链接到这个符号"。JS 没有"跨 DLL"的链接问题，所以体会不到 `UE_API` 的必要性。

---

## 八、常见疑问速答

| 疑问 | 答案 |
|------|------|
| 不加 UE_API 别人就用不了？ | 只有跨模块才用不了；同模块随便用 |
| 同模块内需要 UE_API 吗？ | 不需要，符号在同一 DLL 互相可见 |
| `#include` 解决什么？ | 让编译器"认识"函数（声明层面），不解决跨模块链接 |
| 为什么到处 include 却没到处加 UE_API？ | include 是认识（多数同模块），UE_API 是跨模块导出（少数） |
| 怎么判断要不要加 UE_API？ | 问"会不会被本模块外的代码调用"，会就加 |
| `UE_API` 展开是什么？ | Windows `__declspec(dllexport)`，被引用时变 `dllimport` |
| 声明和定义区别？ | 声明在 .h（招牌），定义在 .cpp（真身） |
| 跨模块链接失败报什么错？ | `unresolved external symbol`（找不到符号） |

---

## 九、总结

```
三件事分开看：
  声明（.h）    = 函数招牌，#include 把它抄进来让编译器认识
  定义（.cpp）  = 函数真身，真正干活的代码
  UE_API        = 跨模块导出标记，让别的 DLL 能链接到

核心规则：
  同模块内调用   → 不用 UE_API（符号都在一个 DLL，互相可见）
  跨模块调用     → 必须 UE_API（跨 DLL 要导出才能链接）

回答三个疑问：
  1. 不加 UE_API 别人用不了？→ 只有跨模块才用不了，同模块随便用
  2. 不是到处 include？      → include 是"认识"，和"能不能链接"是两回事
  3. 没见别处加 UE_API？     → 因为多数调用在同模块内，不需要

判断要不要加 UE_API：
  问"这个函数会不会被本模块外的代码调用？"
  会 → 加；不会 → 不加
```

**一句话**：`UE_API`、`#include`、声明/定义管的是三个不同层面——`#include` 让编译器"认识"函数（同模块够用），`UE_API` 让别的模块"链接到"实现（跨模块必需）。你"没见别处加 `UE_API`"是因为大部分调用都在**同模块内**（不需要）；只有构造函数、`Get()` 这种要被**引擎/其他模块**调用的才导出。判断标准就一句：**会不会被本模块外的代码调用？会就加，不会就不加。**

---

## 十、下一步

- 看 `LYRAGAME_API` 在 `LyraGame.Build.cs` 里怎么定义、怎么自动切换 dllexport/dllimport。
- 理解 UE 的模块系统（`.Build.cs`、`.Target.cs` 怎么组织模块）。
- 实际找一个跨模块调用的例子（引擎调 Lyra 的某处），看链接怎么发生。
- 对比"静态库 vs 动态库"的符号可见性差异。
