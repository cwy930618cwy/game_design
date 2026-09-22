# 用餐厅故事讲透 `#include` 和 `UE_API`

> **定位**：用一个具体故事，讲清"#include 就能用别的代码，那 UE_API 到底啥区别"这个终极困惑。
>
> **触发疑问**：不是 `#include` 就能用别的文件的代码了吗？那 `UE_API` 到底有啥用？
>
> **一句话**：`#include` 只是"**看菜单**"（知道有这道菜），真正"**上菜**"（用到函数实现）时——如果这道菜在**别家厨房**（跨模块）做，就必须有"合作许可"（`UE_API`），否则菜单上写着，后厨却做不出来。

---

## 一、先看一段真实代码

`LyraHealthComponent.cpp`（血量组件）开头 include 了一堆头文件：

```cpp
#include "Character/LyraHealthComponent.h"
#include "System/LyraAssetManager.h"      // ← include 了资产管理器
#include "System/LyraGameData.h"          // ← include 了全局数据
#include "Engine/World.h"                 // ← include 了引擎的 World
#include "GameFramework/PlayerState.h"    // ← include 了引擎的 PlayerState
```

它用 `#include` 引用了两类东西：
- **Lyra 自己的**：`LyraAssetManager.h`、`LyraGameData.h`（和它在同一个模块）
- **引擎的**：`Engine/World.h`、`PlayerState.h`（在 Engine 模块，另一个模块）

> **你的困惑**：这些都能 `#include`，都能用啊，那为什么有的函数还要加 `UE_API`？

**答案在下面这个故事里。**

---

## 二、餐厅故事：`#include` = 看菜单，`UE_API` = 别家厨房的合作许可

把写程序想象成**一家餐厅营业**。

### 2.1 角色对应

| 程序概念 | 餐厅角色 |
|---------|---------|
| 一个 `.cpp` 文件 | 餐厅里的**一个服务员** |
| 编译一个 `.cpp` | 服务员**上岗前看菜单** |
| `#include "X.h"` | 服务员**把 X 厨房的菜单抄一份**放桌上 |
| 函数的**声明**（.h 里） | 菜单上的**菜名**（"宫保鸡丁 38元"） |
| 函数的**实现**（.cpp 里） | 后厨**真正炒这道菜** |
| 一个**模块（.dll）** | 一家**独立的厨房** |
| `UE_API` | 别家厨房给的**合作许可**（能调用它做菜） |

### 2.2 故事第一幕：`#include` = 看菜单（每个服务员独立看）

餐厅有好多服务员（`.cpp`）。**每个服务员上岗前，都要单独看一遍菜单**：

```
服务员 A（LyraHealthComponent.cpp）上岗：
  他把这些菜单抄到自己桌上：
    - LyraAssetManager 的菜单（#include）
    - Engine/World 的菜单（#include）
  现在 A 知道餐厅能做哪些菜了（编译器认识了这些函数）
```

**关键点**：
- 每个服务员（`.cpp`）**独立地**看菜单（编译时一个个单独处理）。
- `#include` 就是"**抄菜单**"——把别的厨房的菜单文字复制到自己桌上。
- **抄菜单这步，没有门槛**——不管菜在哪家厨房，菜单都能抄来看。**这就是"到处都能 #include"的原因。**

> 所以你说"#include 就能用别的文件的代码"——**对，但只对了一半**：你能"看到菜单"（认识函数），不代表能"让别家厨房真做出来"（链接到实现）。

### 2.3 故事第二幕：真正点菜时，分"自家厨房"和"别家厨房"

现在服务员 A 要真的给客人上菜了（**代码里真正调用函数**）。关键来了——**这道菜在哪家厨房做？**

**情况一：菜在自家厨房（同一个模块）**

```
A 要调 LyraAssetManager::GetGameData()
  → 这道菜在"LyraGame 厨房"做（和 A 同一个模块）
  → A 自己厨房的人，喊一声"做份宫保鸡丁"，后厨就做了
  → 不需要任何"合作许可"（不用 UE_API）✅
```

> **同模块** = 同一个厨房。你自己厨房的菜，喊一声就做了，**不需要许可**。

**情况二：菜在别家厨房（跨模块）**

```
A 要调 Engine 的某个函数（比如 GWorld 相关）
  → 这道菜在"Engine 厨房"做（另一个模块/dll）
  → A 不能直接冲进 Engine 厨房乱喊
  → 必须有 Engine 厨房给的"合作许可"（UE_API），才能合法调用
  → 没有许可，后厨不认你，上不了这道菜 ❌（链接失败）
```

> **跨模块** = 别家厨房。你想用别家厨房的菜，必须有"合作许可"（`UE_API`），否则调用不到（链接报错）。

### 2.4 故事的核心（你一直卡住的地方）

```
"能 #include 看到菜单"  ≠  "能让别家厨房真做这道菜"

看菜单（#include）：所有菜都能看，没门槛 —— 所以"到处都能 include"
真做菜（调用+链接）：
  自家厨房（同模块）→ 喊一声就行，不用许可
  别家厨房（跨模块）→ 必须有合作许可（UE_API）
```

> **这就是为什么"到处都 #include，却没到处加 UE_API"**：
> - include 是"看菜单"，所有菜都能看（多数调用还发生在自家厨房，不用许可）。
> - UE_API 是"别家厨房的合作许可"，只有**真要用别家厨房的菜**时才需要（少数跨模块调用）。

---

## 三、回到代码：谁需要 UE_API，谁不需要

看 `LyraAssetManager.h` 里的函数，用"厨房"视角重新看：

```cpp
class ULyraAssetManager : public UAssetManager
{
public:
    UE_API ULyraAssetManager();                     // 别家厨房(引擎)要 new 它 → 要许可
    static UE_API ULyraAssetManager& Get();         // 别家厨房(引擎)要拿单例 → 要许可
    UE_API const ULyraGameData& GetGameData();      // 别家厨房(外部)要拿数据 → 要许可

    // 下面这些没 UE_API：
    UObject* SynchronousLoadAsset(...);             // 自家厨房自用 → 不用许可
    void AddLoadedAsset(...);                       // 自家厨房自用 → 不用许可
};
```

**用故事解释**：
- 标 `UE_API` 的（构造函数、`Get()`、`GetGameData()`）= **别家厨房（引擎/其他模块）会来点的菜** → 需要合作许可。
- 没标的（`SynchronousLoadAsset`、`AddLoadedAsset`）= **只在自家厨房内部用的备菜流程** → 客人看不到，不用许可。

> **判断标准**：这道菜（函数）会不会被**别家厨房（其他模块）**点？会 → 加 `UE_API`；只自家厨房用 → 不加。

---

## 四、那 `LyraHealthComponent.cpp` 里，哪些调用是"跨厨房"？

回到开头的真实代码，它 include 了两类：

```cpp
#include "System/LyraAssetManager.h"      // 同模块（都在 LyraGame 厨房）
#include "Engine/World.h"                 // 跨模块（Engine 厨房）
#include "GameFramework/PlayerState.h"    // 跨模块（Engine 厨房）
```

**当它调用这些时**：

```cpp
// 调 LyraAssetManager::Get() —— 同模块（自家厨房）
//   → 不用 UE_API？但 Get() 标了 UE_API 啊？
//   答：Get() 标 UE_API 是因为"引擎也会调它"，不代表你调它时必须有。
//       你（同模块）调它，不需要许可也能用；它导出是给引擎准备的。

// 调 Engine 的 GWorld、PlayerState 的函数 —— 跨模块（别家厨房）
//   → 这些引擎函数，引擎自己早就用 UE_API 导出好了
//   → 你（LyraGame 厨房）要用，直接调就行（引擎已经给了许可）
```

> **关键领悟**：`UE_API` 是"**提供方**"为了"别家能用"而加的。`LyraHealthComponent` 调引擎函数能成功，是因为**引擎那边**早就把那些函数用 `UE_API` 导出好了。每一方要为自己提供的、跨模块可用的函数加 `UE_API`。

---

## 五、一张图总结整个故事

```
【服务员 A = LyraHealthComponent.cpp】
  上岗前看菜单（#include）：
    ├─ LyraAssetManager 菜单（同模块）
    └─ Engine/World 菜单（跨模块）
  ※ 看菜单没门槛，所以"到处都能 include"

  真正上菜（调用函数）：
    ├─ 调 LyraAssetManager::GetGameData()
    │    → 自家厨房（同模块）→ 喊一声就做了 → 不用许可
    │
    └─ 调 Engine::GWorld 的函数
         → 别家厨房（跨模块）→ 需要 Engine 给的许可（UE_API）
         → 引擎早就导出了，所以 A 能成功调用

【谁加 UE_API？】
  "提供菜"的一方，为了让"别家厨房能点"而加
  - LyraAssetManager 的 Get() 标 UE_API → 因为引擎要调
  - Engine 的函数标 UE_API → 因为 Lyra 要调
```

---

## 六、终极一句话（背下来）

> **`#include` = 看菜单（所有菜都能看，没门槛，所以到处能 include）。**
> **真正上菜（调用）时：自家厨房（同模块）喊一声就行；别家厨房（跨模块）必须有合作许可（`UE_API`）。**
> **`UE_API` 是"提供菜的一方"为"别家能点"加的许可——所以每个模块只为自己对外提供的函数加，调用方不用加。**

---

## 七、如果还是晕，记住这个口诀

```
include 是看菜单，谁都能看（到处能用）
调用是真正点菜，分自家别家
自家厨房（同模块）：不用许可
别家厨房（跨模块）：要有许可（UE_API）
许可是"做菜方"加的，不是"点菜方"加的
```

---

## 八、常见疑问速答

| 疑问 | 答案 |
|------|------|
| #include 就能用别的代码吗？ | 能"看到菜单"（认识函数），但跨模块真调用还要 UE_API |
| 为什么到处能 include 却没到处加 UE_API？ | include 是看菜单（没门槛）；UE_API 是别家厨房许可（只有跨模块才要） |
| 模块是什么？ | 一家独立的厨房 = 一个 .dll |
| 同模块调用需要 UE_API 吗？ | 不需要（自家厨房喊一声就行） |
| 谁该加 UE_API？ | "提供函数的一方"，为了让别家能调用 |
| 调用方要加 UE_API 吗？ | 不用，是提供方加的 |
| LyraHealthComponent 调引擎函数为啥能成？ | 引擎那边早把那些函数用 UE_API 导出好了 |

---

## 九、下一步

- 找一个"跨模块调用失败"的真实报错（`unresolved external symbol`）感受一下。
- 看 `LyraGame.Build.cs`，确认哪些 cpp 编进同一个 dll（同一个厨房）。
- 理解"为什么引擎函数不用你加 UE_API 也能调"（引擎已导出）。
- 回头看 `03` 篇的三层面对照表，配合这个故事就通了。
