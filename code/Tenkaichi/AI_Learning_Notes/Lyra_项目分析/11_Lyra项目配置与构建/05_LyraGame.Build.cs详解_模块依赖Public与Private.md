# 05 — `LyraGame.Build.cs` 详解：模块的依赖清单与 Public / Private

> **定位**：`04` 讲了 `Target.cs`（编出哪几种程序）。这篇讲另一半——`LyraGame.Build.cs`（107 行）——它回答"**LyraGame 这个模块能用哪些其他模块**"。它是 `04` 里那个 `LyraGameTarget` 点名要编的模块，也是全 Lyra 唯一运行时主模块的"依赖总账"。
>
> **一句话**：Target 决定"编不编我"，Build.cs 决定"我 include 谁时编译器不报错"。

---

## 一、先看它在文件系统里的位置

```
Source/
├── LyraGame/                  ← 一个代码模块 = 一坨 .h/.cpp + 一个 .Build.cs
│   ├── LyraGame.Build.cs      ← 本文件：给 UBT 看的"依赖清单"
│   ├── LyraGame.h / .cpp      ← 真正的游戏代码
│   └── (几百个 .h/.cpp)
└── LyraGame.Target.cs         ← 04 篇：声明"要编 LyraGame 这个模块"
```

UBT 编译时的配合关系：

```
Target.cs:  ExtraModuleNames.Add("LyraGame")   "我点名要它"
Build.cs:   LyraGame : ModuleRules             "它是什么、能用谁"
uproject:   Modules: [{ Name:"LyraGame" }]     "项目登记它"
```

> 💡 三个文件谁都不能缺：`uproject` 缺了=引擎不认；`Target.cs` 缺了=没人点名编它；`Build.cs` 缺了=模块本身不存在。

---

## 二、逐段解剖：先看整体骨架

```
 LyraGame.Build.cs（107行）骨架：左=代码本身 │ 右=一句批注
═══════════════════════════════════════════════════════════════
 using UnrealBuildTool;                    │ C#：引入 UBT 类型
 public class LyraGame : ModuleRules       │ ★类名=模块名，必须
 {                                         │   继承 ModuleRules
   public LyraGame(ReadOnlyTargetRules T)  │ 构造函数(UBT调用)
   {                                       │
     PCHUsage = UseExplicitOrSharedPCHs;   │ 用"预编译头"提速编译
     PublicIncludePaths.Add("LyraGame");   │ 别人 include 本模块
     PrivateIncludePaths.Add(...空);       │   头文件时的搜索路径
     ──────────────────────────────────────┼────────────────
     PublicDependencyModuleNames.Add(...)  │ ① 公开依赖(22个)
     PrivateDependencyModuleNames.Add(...) │ ② 私有依赖(26个)
     DynamicallyLoadedModuleNames(空)      │ ③ 动态加载依赖(不用)
     ──────────────────────────────────────┼────────────────
     PublicDefinitions.Add("SHIPPING_DRAW_ │ ④ 编译宏：
       DEBUG_ERROR=1");                    │   用DrawDebug=编译错误
     PrivateDependencyModuleNames.Add(     │ ⑤ RPC调试框架
       "ExternalRpcRegistry");             │   + HTTPServer
     if (Shipping) { WITH_RPC_REGISTRY=0 } │   → 正式包全部关掉
       else { WITH_RPC_REGISTRY=1 }        │   (防漏洞)
     SetupGameplayDebuggerSupport(Target); │ ⑥ 游戏调试器支持
     SetupIrisSupport(Target);             │ ⑦ 新网络复制(Iris)
   }                                       │
 }                                         │
═══════════════════════════════════════════════════════════════
```

---

## 三、核心概念：依赖为什么要分 Public / Private

一个模块 include 别的模块，分三种依赖：

| 类型 | 含义 | 会传给"引用我的模块"吗 |
|---|---|---|
| `PublicDependency` | **我 .h 头文件里用到的**模块 | ✅ 会——别人 include 我的头文件，也就间接需要它 |
| `PrivateDependency` | **只有我 .cpp 里用**的模块 | ❌ 不会——别人看不到 |
| `DynamicallyLoaded` | 运行时才按需 Load 的模块 | 特殊，一般不用 |

> 💡 **一句话区分**：**头文件里 `#include` 了它的，放 Public；只有 .cpp 里 `#include` 的，放 Private。**

为什么要分这么细？举个例子：

```
模块 A：PublicDependency 加了 20 个模块
模块 B：PublicDependency 加了 A
→ B 无形中也能 include A 的那 20 个模块（被传导）
```

如果不分公私，全塞 Public，依赖会像雪球越滚越大，最后**改一个模块引发全项目重编**。放 Private 能"藏起来"：你内部用的，别人不该知道。

> 💡 **场景记忆**：你写了个 `UHealthComponent`（头文件里 include 了 `GameplayEffectTypes.h`）→ 这个 GAS 依赖就该放 **Public**；如果你的 .cpp 里悄悄用了 `Json` 存档，别人根本不需要知道 → 放 **Private**。

---

## 四、看 Lyra 到底怎么分的（有规律可循）

### Public（22 个）—— 头文件级别就要用，还要露给下游

```
Core              CoreOnline        CoreUObject       ApplicationCore
Engine            PhysicsCore       GameplayTags      GameplayTasks
GameplayAbilities AIModule          ModularGameplay   ModularGameplayActors
DataRegistry      ReplicationGraph  GameFeatures      SignificanceManager
Hotfix            CommonLoadingScreen Niagara         AsyncMixin
ControlFlows      PropertyPath
```

### Private（26 个）—— 只有实现用，藏起来

```
InputCore    Slate    SlateCore     RenderCore    DeveloperSettings
EnhancedInput NetCore RHI           Projects      Gauntlet
UMG          CommonUI CommonInput   GameSettings  CommonGame
CommonUser   GameSubtitles GameplayMessageRuntime AudioMixer
NetworkReplayStreaming UIExtension  ClientPilot   AudioModulation
EngineSettings DTLSHandlerComponent Json
```

看规律：

| 为什么放 Public | 为什么放 Private |
|---|---|
| `LyraGame.h` 这类公共头里 `#include` GAS 类型 → 引用 LyraGame 的模块（如 LyraEditor）也要能编译 | UI/输入只在 .cpp 里用，不暴露给下游 |
| GAS、ModularGameplay、GameFeatures 是 Lyra 的**立身之本**，任何引用方都要知道 | Slate/CommonUI 等属于"实现细节" |
| Niagara 这种被公共 API 暴露的 | 网络加密组件 DTLS 只是内部握手用 |

> 💡 **场景**：你去 LyraGame 里找一个类，报错"找不到 UAbilitySystemComponent"——大概率是这个头文件来自某个被 Private 藏起来的依赖，你得先把它提到 Public（或改 include 位置）。

---

## 五、剩下的几个"花样"逐段讲

### ① `PCHUsage = UseExplicitOrSharedPCHs`（L9）

用"共享/显式预编译头"加速编译。几乎每个 UE 模块都这么写，照抄即可，不用深究。

### ② `PublicIncludePaths.Add("LyraGame")`（L11-15）

别人 include 你模块头文件时去哪找。一般也照抄（值 = 模块目录名）。

### ③ `SHIPPING_DRAW_DEBUG_ERROR=1`（L86）

定义一个编译宏：**在 Test/Shipping 包用 `DrawDebugLine` 等调试绘制函数 → 直接编译错误**。防止把调试线框漏进发布版。

> 💡 **场景**：你调试时到处画线框，忘了删就打包——有这个宏在，打包直接失败并告诉你哪个文件在用 DrawDebug，逼你删干净。

### ④ RPC 调试框架（L88-101）—— 正式包一键阉割

```csharp
PrivateDependencyModuleNames.Add("ExternalRpcRegistry");  // 外部RPC注册表
PrivateDependencyModuleNames.Add("HTTPServer");           // 跑HTTP服务用

if (Shipping) { WITH_RPC_REGISTRY=0; WITH_HTTPSERVER_LISTENERS=0; }  // 正式版关
else          { WITH_RPC_REGISTRY=1; WITH_HTTPSERVER_LISTENERS=1; }  // 开发版开
```

这给 Lyra 留了个"开发期能通过 HTTP/RPC 远程操作游戏"的后门（自动化测试/调试用），但只要打 Shipping 正式包，两个宏自动变 0，**相关代码被编译器剔除**——防漏洞。

> 💡 **场景**：测试机器人夜里通过 HTTP 控制游戏跑自动化；而发给玩家的正式包里这段能力根本不存在。

### ⑤ `SetupGameplayDebuggerSupport(Target)`（L103）

给这个模块挂上 `GameplayDebugger`（游戏内按 `~` 键弹出的调试菜单）支持，方便运行时看 AI/能力状态。

### ⑥ `SetupIrisSupport(Target)`（L104）

开启 **Iris**（UE 新一代网络复制系统）支持。Lyra 在向新复制架构迁移，老项目可以不管。

---

## 六、对照你仓库的 `code.Build.cs`

```
 code.Build.cs（你写的）          LyraGame.Build.cs（107行）
 ────────────────────────       ────────────────────────
 class code : ModuleRules        class LyraGame : ModuleRules
 加了 GAS 三件套到 Public：       22 Public + 26 Private 大而全
  GameplayAbilities             ＋一堆宏/框架/Iris支持
  GameplayTags
  GameplayTasks
```

你的 `code` 模块现在只有几个依赖——因为还不需要。等你像 Lyra 一样要 `AttributeSet`、要 CommonUI、要联机时，就往 Public/Private 里加，**加在哪**按这一节的标准：**头文件用→Public；.cpp 用→Private**。

---

## 七、本篇一句话

`LyraGame.Build.cs` 就是 LyraGame 模块的"**允许 include 名单**"：Public 名单上的模块会被露给所有引用者，Private 名单被藏起来，最后再叠几个编译宏和安全开关。你将来写每个模块时，先想清楚"我这个 .h 要 include 谁"——它决定了 `PublicDependencyModuleNames` 里要写什么。
