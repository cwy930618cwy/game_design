# 12 — `#define UE_API LYRAGAME_API` 详解（API 导出宏）

> **定位**：解释 LyraPawn.h 里的 `#define UE_API LYRAGAME_API`——这是 **API 导出宏**，控制"类的代码能不能被别的模块看到/使用"。
>
> **一句话**：`UE_API` 被替换成 `LYRAGAME_API`，而 `LYRAGAME_API` 是**模块导出宏**——它告诉编译器"这个类是要导出给别的模块用，还是只在本模块内部"。**跨模块用类必须加它。**

---

## 一、先搞懂：这行代码做了什么

```cpp
#define UE_API LYRAGAME_API
```

**这是一个宏定义**：把 `UE_API` 这个"占位符"替换成 `LYRAGAME_API`。

```
你写代码时：
  class UE_API ALyraPawn { ... };
  //       ↑ UE_API

编译时（宏替换后）：
  class LYRAGAME_API ALyraPawn { ... };
  //       ↑ 变成 LYRAGAME_API
```

**作用**：让代码里写 `UE_API`，编译时自动换成 `LYRAGAME_API`。**为了统一写法**（不同模块都用 `UE_API`，各自替换成自己的导出宏）。

---

## 二、那 `LYRAGAME_API` 又是什么？（模块导出宏）

`LYRAGAME_API` 才是真正干活的——它是**模块导出宏**。在模块的 Build 配置里定义，通常长这样：

```cpp
// 通常在模块的 Build.cs 或预处理器里定义
// LYRAGAME_API 根据编译情况变成导入或导出
#ifdef LYRAGAME_API
    // 当 LYRAGAME 模块内部编译时：
    #define LYRAGAME_API DLLEXPORT   // 导出（让别的模块能用）
#else
    // 当其他模块引用 LyraGame 时：
    #define LYRAGAME_API DLLIMPORT   // 导入（从 LyraGame 拿）
#endif
```

**简化理解**：
```
LYRAGAME_API =
  编译 LyraGame 模块自己时 → "导出"（export，让别人能用）
  其他模块用它时          → "导入"（import，从 LyraGame 拿）
```

---

## 三、为什么要"导出/导入"？（核心概念）

**在 C++ 里，不同模块（编译成 .dll/.so）之间，类的代码默认是"隔离"的**。

```
模块 A（LyraGame.dll）里有 ALyraPawn
模块 B（你的游戏模块）想用 ALyraPawn
  → 但默认情况下，模块 B 看不到 ALyraPawn（符号没导出）
  → 必须用 LYRAGAME_API 标记"导出"，模块 B 才能用
```

**没有导出宏的后果**：
```cpp
// ❌ 没加 LYRAGAME_API：别的模块用不了
class ALyraPawn { ... };   // 其他模块引用会链接错误

// ✅ 加了 LYRAGAME_API：导出，别的模块能用
class LYRAGAME_API ALyraPawn { ... };   // 其他模块能用了
```

**一句话**：**`LYRAGAME_API` = "标记这个类导出给别的模块用"**。跨模块要用，必须加。

---

## 四、配具体场景

**场景：你的游戏模块想用 LyraGame 的 ALyraPawn**

```cpp
// LyraGame 模块里（加了导出宏）
class LYRAGAME_API ALyraPawn : public AModularPawn {
    // 这个类被导出了，别的模块能用
};

// 你的模块里（能用了，因为 LyraGame 导出了）
#include "LyraPawn.h"
class UMySomething {
    ALyraPawn* MyPawn;   // ✅ 能用，因为导出了
};
```

**如果不导出**：你的模块引用 ALyraPawn 会**链接错误**（找不到符号）。

---

## 五、常见写法对照（不同模块）

| 模块 | 它的 API 宏 |
|------|------------|
| Core | `CORE_API` |
| Engine | `ENGINE_API` |
| LyraGame | `LYRAGAME_API` |
| 你自己的模块 | 你的模块名_API |

```cpp
// 不同模块用各自的导出宏
class CORE_API    FString { ... };    // Core 模块
class ENGINE_API  AActor { ... };     // Engine 模块
class LYRAGAME_API ALyraPawn { ... }; // LyraGame 模块
```

---

## 六、那 `UE_API` 统一写法的意义

因为每个模块导出宏名字不同（`CORE_API`/`ENGINE_API`/`LYRAGAME_API`），所以 UE 用 `UE_API` 做统一占位符：

```cpp
// Lyra 的代码里统一写 UE_API
#define UE_API LYRAGAME_API    // Lyra 模块：UE_API → LYRAGAME_API

// 如果这段代码放到别的模块，只需改这一行
#define UE_API OTHERMODULE_API   // 别的模块：UE_API → OTHERMODULE_API
```

**价值**：**代码里统一写 `UE_API`，模块间复制时只改宏定义那一行**，不用改所有代码。

---

## 七、总结速查

```
#define UE_API LYRAGAME_API
  = 把 UE_API 替换成 LYRAGAME_API（统一写法）

LYRAGAME_API = 模块导出宏
  编译自己时 → 导出（export，别人能用）
  别的模块用 → 导入（import，从这拿）

为什么需要：
  不同模块（.dll）间类默认隔离
  要跨模块用 → 必须导出（加 LYRAGAME_API）

常见：
  CORE_API（Core）/ ENGINE_API（Engine）/ LYRAGAME_API（LyraGame）
```

**一句话**：`#define UE_API LYRAGAME_API` 是**宏定义**，把 `UE_API` 换成 `LYRAGAME_API`。`LYRAGAME_API` 是**模块导出宏**——标记"这个类导出给别的模块用"。**跨模块要用类必须加它**，否则其他模块引用会链接错误。`UE_API` 是统一占位符，方便模块间复用代码。

---

## 八、下一步

理解了 API 导出宏，下一步可以继续看 `ALyraPawn` 的其他部分，或深入"模块（Module）"的概念。
