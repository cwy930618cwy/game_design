# 01 — `ULyraGameEngine` 详解：整个进程"第一站"的引擎对象

> **定位**：上一篇 `00` 把 `System/` 全目录过了一遍。这篇啃第一个文件——`LyraGameEngine.h/.cpp`（全工程最短的类之一，却站在"进程最底层"）。读懂它，你就明白**游戏进程是从哪一棵树上长出来的**。
>
> **一句话**：它是**游戏运行时整个进程的"总引擎对象"**——由引擎按 ini 配置创建，生命周期 == 游戏进程的生命周期，`Init()` 比任何玩法代码都先跑。

---

## 一、先放对位置：它是"进程第一站"

```
 双击游戏 exe
   │
   ▼ ① 引擎底层启动（FEngineLoop）
   │    └─ 加载模块 → 执行各模块 StartupModule
   │        （LyraGameModule.cpp，见 11 系列 04）
   │
   ▼ ② 引擎按 ini 配置"造出"一个引擎对象 ← ★ULyraGameEngine 在这诞生
   │       DefaultEngine.ini:
   │         GameEngine=/Script/LyraGame.LyraGameEngine
   │
   ▼ ③ ULyraGameEngine::Init() ← 最先跑到的项目代码之一
   │
   ▼ ④ 创建 GameInstance → ULyraGameInstance::Init()（下一篇的主角）
   │
   ▼ ⑤ 创建/加载 World，GameMode 进场，玩法开始
```

> 💡 **场景记忆**：整个进程像一个公司——
> - `exe` = 公司大门
> - **GameEngine = 公司的"法人/总裁"**（从头到尾只有一个，进程关了它才走）
> - GameInstance = 这家公司的"总经理办公室"（管每局生意的状态）
> - GameMode = 每个"项目组"的组长（进图才存在）

---

## 二、先理解它是什么：`UGameEngine` 在 UE 里的地位

**一句话图（先看这张，晕的感觉就消了）**：

```
 ① 血缘：所有引擎类从同一棵树上长出来
 ─────────────────────────────────────────────
                    UEngine（地基）
                       │ 派生
        ┌──────────────┴──────────────┐
   UGameEngine                     UnrealEdEngine
   （游戏运行时用）                  （编辑器进程用）
        │
   ULyraGameEngine
   （Lyra 项目版 = 我们看的这个）

 ② 选谁当引擎：不是代码决定，是 ini 决定
 ─────────────────────────────────────────────
   你跑玩家版 exe ──► ini 读 GameEngine= 行
        │            GameEngine=/Script/LyraGame.LyraGameEngine
        ▼
   进程里造出：ULyraGameEngine（游戏版引擎）

   你在编辑器开发 ──► ini 读 EditorEngine= 行
        │            EditorEngine=/Script/LyraEditor.LyraEditorEngine
        ▼
   进程里造出：LyraEditorEngine（编辑器版引擎）

 ③ 一句话记法：同一个项目有两套"引擎大脑"，
    游戏版跑在玩家机器上，编辑器版跑在你电脑上。
```

`ULyraGameEngine` 继承的是 `UGameEngine`。要理解子类，先理解基类：

| 东西 | 是什么 |
|---|---|
| `UGameEngine` | 引擎的**游戏运行时引擎类**：负责管理 World、Viewport（游戏窗口）、把每帧 Tick 派发给所有系统、加载地图 |
| `UEngine`（再上层） | 所有引擎类的地基（编辑器、游戏都用） |
| `UnrealEdEngine` | 编辑器进程用的引擎类（跟 `UGameEngine` 是**兄弟**，不在游戏里出现） |

**关键认知：一台机器上具体用哪个引擎类，不是写死在代码里，而是由 ini 指定的**：

```ini
; Config/DefaultEngine.ini（Lyra 里的真实配置）
GameEngine=/Script/LyraGame.LyraGameEngine        ← 打包运行时用的引擎类
UnrealEdEngine=/Script/LyraEditor.LyraEditorEngine ← 编辑器进程用这个
EditorEngine=/Script/LyraEditor.LyraEditorEngine
```

> 💡 **场景记忆**：跑玩家版 exe → 引擎读 `GameEngine=` 那行，造出 **LyraGameEngine**；在编辑器里开发 → 引擎读 `EditorEngine=` 那行，造出 **LyraEditorEngine**。同一个项目，两套引擎类，靠 ini 切换。

---

## 三、源码解剖：整个类就 20 行，全是"预留的钩子"

### `LyraGameEngine.h`（头文件）

```
 LyraGameEngine.h（26行）
 ═══════════════════════════════════════════════════════════
 #pragma once                                │ 防止重复包含
 #include "Engine/GameEngine.h"              │ 基类在这
 #include "LyraGameEngine.generated.h"        │ UHT 生成物(必须有)
                                             │
 UCLASS()                                    │ 这是个 UObject 类
 class ULyraGameEngine : public UGameEngine  │ ★继承游戏引擎类
 {                                           │
   GENERATED_BODY()                          │ UHT 必备宏
                                             │
 public:                                     │
   ULyraGameEngine(                          │ 构造函数（很少动）
     const FObjectInitializer& ObjectInitializer
       = FObjectInitializer::Get());         │
 protected:                                  │
   virtual void Init(IEngineLoop* InEngineLoop) override;  │ ★核心扩展点
 };                                          │
```

### `LyraGameEngine.cpp`（实现）

```
 LyraGameEngine.cpp（19行）
 ═══════════════════════════════════════════════════════════
 ULyraGameEngine::ULyraGameEngine(...)       │ 构造：把活交给父类
   : Super(ObjectInitializer)                │  → 目前什么都没加
 {                                           │
 }                                           │
                                             │
 void ULyraGameEngine::Init(IEngineLoop* InEngineLoop)  │ ★覆写 Init
 {                                           │
   Super::Init(InEngineLoop);                │ 先执行引擎默认初始化
 }                                           │  → 目前也没加东西
```

> ⚠️ **重点观察**：这个类**现在什么都没做**——构造空、Init 只调父类。它是 Lyra 故意留下的"**占位扩展点**"：一旦哪天需要在引擎最早期挂逻辑，直接往 `Init()` 里加，不用改引擎。

---

## 四、它为什么"有用却空着"：什么时候你才需要动它

UE 默认的 `UGameEngine` 能直接跑游戏，绝大多数项目**根本不需要子类化**。只有当你需要在这些时机介入时才用：

| 你想干的事 | 该去哪 |
|---|---|
| 模块加载时执行代码（注册控制台命令等） | 模块 `StartupModule()`（更轻） |
| **引擎对象初始化时**干全局的事 | 覆写 `Init()` ← LyraGameEngine 的口子 |
| 想控制"游戏窗口/视口" | 另配 `GameViewportClientClassName`（Lyra 也配了：`LyraGameViewportClient`） |
| 每局游戏的逻辑状态 | **别放这**，放 GameInstance |
| 某个图里的规则 | **别放这**，放 GameMode |

### GameEngine vs GameInstance（最容易混，一次分清）

| | `UGameEngine` | `UGameInstance` |
|---|---|---|
| 几个 | 每进程 1 个 | 每进程 1 个 |
| 出现时机 | 更早（进程刚起） | 稍晚（引擎 Init 之后） |
| 管什么 | World/视口/全局Tick/底层 | 跨图数据、会话、玩家管理 |
| Lyra 的类 | `ULyraGameEngine` | `ULyraGameInstance` |
| 玩法代码该挂这吗 | 不该（太底层） | **该**（下一篇主角） |

> 💡 **场景**：游戏要支持"Ctrl+Alt+Shift+F12 切出调试 HUD 且全进程生效"——这是引擎级功能，可考虑放 GameEngine；而"玩家死亡后掉的道具要保留到下一局"是跨图数据，放 **GameInstance** 才对。放错层是新手最常见的架构错误。

---

## 五、本篇一句话

`ULyraGameEngine` = Lyra 的**进程级引擎对象**：由 `DefaultEngine.ini` 的 `GameEngine=` 一行指定，在 `exe` 启动后、任何玩法代码之前被创建，`Init()` 是它唯一的核心扩展点——目前是**空壳占位**。它和 `UGameInstance` 的分工是"**总裁管进程底层、总经理管游戏状态**"，玩法代码默认该挂后者。下一篇就讲这位"总经理"`ULyraGameInstance`。
