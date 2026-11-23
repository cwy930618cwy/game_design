# 答疑 — `Gauntlet` 是干嘛的？怎么用？

> 触发：`.uproject` 里这一项 ——
> ```jsonc
> {
>     // 自动化测试框架，CI 无人值守跑测试用
>     "Name": "Gauntlet",
>     "Enabled": true
> },
> ```
> 实查：`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Gauntlet\`（插件）
> 　　　`D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\AutomationTool\Gauntlet\`（工具链，**这才是主体**）
> 结论：**它是一套「无人值守跑游戏并判成败」的自动化测试框架，分两层：C++ 侧的测试控制器（游戏里跑的）+ C# 侧的测试节点（外面指挥的）。你日常开发完全用不到，只有搭 CI 才需要。**

---

## 一、先说结论（4 句话）

1. **它不是「单元测试」**，是「**启动整个游戏、自动操作、看结果**」的**集成测试 / 冒烟测试**框架。
2. **两层结构**：C++ 插件（游戏内 `UGauntletTestController`）+ C# 工具链（外部 `BaseTestNode` 指挥启动 / 收集日志 / 判成败）。
3. **典型用途**：每次提交代码后，机器自动启动游戏 → 等它进主菜单 → 截图 → 判「有没有崩 / 有没有报错」→ 退出码告诉 CI 成功或失败。
4. **你现在完全不需要它** —— 保留只是因为 Lyra 原样启用 + `LyraGame.Build.cs` 依赖了 `Gauntlet` 模块（第 04 轮讲义私有依赖第 10 项）。

---

## 二、它到底是什么（两层结构）

```
┌─────────────────────────────────────────────────────────┐
│  外层：C# 工具链（跑在开发机 / CI 服务器上）              │
│  D:\...\Engine\Source\Programs\AutomationTool\Gauntlet\  │
│                                                          │
│  · 编译成 AutomationTool 的一部分                        │
│  · 通过 RunUAT.bat 调用                                  │
│  · 职责：启动游戏进程、传命令行、收日志、判成败、出报告    │
│  · 你写的是：继承 BaseTestNode / UnrealTestNode 的 .cs    │
└───────────────────────┬─────────────────────────────────┘
                        │ 传 -gauntlet=XxxTest 命令行
                        ↓
┌─────────────────────────────────────────────────────────┐
│  内层：C++ 插件（跑在游戏进程里）                         │
│  D:\...\Engine\Plugins\Experimental\Gauntlet\            │
│                                                          │
│  · 模块名 Gauntlet，Type: Runtime                        │
│  · 职责：解析 -gauntlet= 参数 → 创建测试控制器 → 驱动它   │
│  · 你写的是：继承 UGauntletTestController 的 C++ 类       │
└─────────────────────────────────────────────────────────┘
```

### 2.1 内层：C++ 插件（游戏内的「测试导演」）

`.uplugin` 原文（`Gauntlet.uplugin`）：

```json
{
    "FriendlyName": "Gauntlet",
    "Description": "Provides a helper class for creating and managing tests in your game",
    "Category": "Testing",
    "EnabledByDefault": false,
    "IsBetaVersion": true,
    "Modules": [
        {
            "Name": "Gauntlet",
            "Type": "Runtime",        ← 运行时模块（在游戏进程里跑）
            "LoadingPhase": "PreDefault"   ← 引擎启动最早期加载
        }
    ]
}
```

> ⚠️ 注意 `"EnabledByDefault": false` —— 引擎默认不启用，靠 `.uproject` 那行 `"Enabled": true` 才有。

整个插件只有 **4 个类**（9 个文件）：

| 文件 | 作用 |
|---|---|
| `GauntletModule.h/.cpp` | 模块主体 `FGauntletModule`，解析命令行、创建控制器、驱动 tick |
| `GauntletTestController.h/.cpp` | **测试控制器基类** `UGauntletTestController`，你继承它写测试 |
| `GauntletTestControllerBootTest.h/.cpp` | 内置的「只要游戏能启动就算通过」控制器 |
| `GauntletTestControllerErrorTest.h/.cpp` | 内置的「故意报错，验证失败路径能跑通」控制器 |

### 2.2 外层：C# 工具链（外面的「总指挥」）

```
D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\AutomationTool\Gauntlet\
├─ Framework\      ← BaseTestNode / TestContext / TestResult 等基础类
├─ Unreal\         ← UnrealTestNode / EpicGameTestNode（UE 游戏专用）
│  ├─ Base\
│  ├─ Game\
│  ├─ Editor\
│  ├─ Config\
│  └─ ...
├─ Platform\       ← Win64 / Android / IOS / PS4... 平台适配
├─ SelfTest\       ← 自带的最小示例（Blank 测试节点）
└─ ThirdParty\
```

**它不是一个独立 exe** —— 它被编译进 `AutomationTool`，通过 `RunUAT.bat` 调用。

---

## 三、最小可用示例（3 段代码看懂）

### 3.1 内层：写一个 C++ 测试控制器

**第一步**：新建 `Source\LyraGame\Tests\LyraTestControllerBootTest.h`

```cpp
#pragma once

#include "GauntletTestControllerBootTest.h"
#include "LyraTestControllerBootTest.generated.h"

// 继承内置的「启动测试」基类
UCLASS()
class ULyraTestControllerBootTest : public UGauntletTestControllerBootTest
{
    GENERATED_BODY()

protected:
    // 游戏启动后等 20 秒，才算「启动完成」
    const double TestDelay = 20.0f;

    // 基类每 tick 会调用它，返回 true 就判定通过
    virtual bool IsBootProcessComplete() const override;
};
```

**第二步**：新建 `Source\LyraGame\Tests\LyraTestControllerBootTest.cpp`

```cpp
#include "Tests/LyraTestControllerBootTest.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraTestControllerBootTest)

bool ULyraTestControllerBootTest::IsBootProcessComplete() const
{
    static double StartTime = FPlatformTime::Seconds();
    const double TimeSinceStart = FPlatformTime::Seconds() - StartTime;

    // 从第一次被调用算起，过了 20 秒就认为启动成功
    if (TimeSinceStart >= TestDelay)
    {
        return true;
    }

    return false;
}
```

> 📌 这就是 Lyra 真实源码里的那份（`E:\ue5\LyraStarterGame\Source\LyraGame\Tests\LyraTestControllerBootTest.h/.cpp`），
> 注释里还留着 `//@TODO: actually do some useful testing here` —— 说明连 Epic 自己也只是「先确认能启动」。

**控制器有哪些可重写的钩子**（`GauntletTestController.h:38-59`）：

| 钩子 | 什么时候调用 |
|---|---|
| `OnInit()` | 控制器刚被创建时 |
| `OnPreMapChange()` | 换图之前 |
| `OnPostMapChange(UWorld*)` | 换图之后 |
| `OnTick(float TimeDelta)` | 每帧 |
| `OnStateChange(FName OldState, FName NewState)` | 状态变化时（状态是你自己定义的） |

**关键 API**：

| 函数 | 作用 |
|---|---|
| `GetWorld()` | 拿当前世界 |
| `GetFirstPlayerController()` | 拿第一个玩家控制器（可以用来发指令） |
| `GetCurrentMap()` | 当前地图名 |
| `MarkHeartbeatActive("...")` | 打一次「我还活着」的心跳日志（CI 卡死时能定位） |
| **`EndTest(int32 ExitCode)`** | ★ **结束测试并退出游戏**；`0` = 成功，非 0 = 失败 |

### 3.2 外层：写一个 C# 测试节点

```csharp
using EpicGame;
using Gauntlet;

namespace LyraTest
{
    // 继承 EpicGameTestNode<你的配置类>
    public class BootTest : EpicGameTestNode<LyraTestConfig>
    {
        public BootTest(UnrealTestContext InContext) : base(InContext)
        {
        }

        public override LyraTestConfig GetConfiguration()
        {
            LyraTestConfig Config = base.GetConfiguration();
            Config.NoMCP = true;     // 不装 MCP（崩溃报告）插件

            // 要求启动 1 个客户端
            UnrealTestRole Client = Config.RequireRole(UnrealTargetRole.Client);
            // ★ 关键：告诉游戏进程「跑 BootTest 这个控制器」
            Client.Controllers.Add("BootTest");

            return Config;
        }
    }
}
```

> 📌 这也是 Lyra 真实源码（`E:\ue5\LyraStarterGame\Build\Scripts\Automation\LyraTest.BootTest.cs`）。

**这行是关键**：

```csharp
Client.Controllers.Add("BootTest");
   ↓
Gauntlet 会在启动游戏的命令行里加上：-gauntlet=BootTest
   ↓
游戏内 GauntletModule 解析到 "BootTest"
   ↓
遍历所有 UClass，找「类名以 BootTest 开头 / 包含 BootTest」且继承自 UGauntletTestController 的类
   ↓
找到 ULyraTestControllerBootTest → new 出来 → 开始驱动它
```

**源码证据**（`Gauntlet.UnrealTestNode.cs:657`）：

```csharp
// add controllers
SessionRole.CommandLineParams.Add("gauntlet",
    TestRole.Controllers.Count > 0 ? string.Join(",", TestRole.Controllers) : null);
```

**游戏内解析**（`GauntletModule.cpp:177`）：

```cpp
FString ControllerString;
// 解析命令行里的 -gauntlet=XXX
FParse::Value(FCommandLine::Get(), TEXT("gauntlet="), ControllerString, false);

if (ControllerString.Len())
{
    TArray<FString> ControllerNames;
    // 支持逗号分隔多个控制器：-gauntlet=A,B,C
    ControllerString.ParseIntoArrayWS(ControllerNames, TEXT(","));
    ...
    // 遍历所有 UClass，找名字匹配且继承自 UGauntletTestController 的
    FString PartialName1 = Name + TEXT("Controller");     // BootTest → BootTestController
    FString PartialName2 = FString(TEXT("Controller")) + Name;  // 兼容旧写法
```

> 🔑 **命名规则**：你的控制器类叫 `ULyraTestControllerBootTest`，C# 里写 `"BootTest"` 就能匹配上（因为类名里含 `BootTest`）。

### 3.3 运行：一行命令

```powershell
# 在工程根目录执行
"D:\ue5\Epic Games\UE_5.4\Engine\Build\BatchFiles\RunUAT.bat" ^
    RunUnreal ^
    -test=LyraTest.BootTest ^
    -platform=Win64 ^
    -configuration=Development ^
    -build=local ^
    -project="E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject"
```

> `RunUnreal` 是 Gauntlet 注册给 AutomationTool 的命令。
> 具体参数随版本略有差异，`-help` 可以看全部。

---

## 四、Gauntlet 到底能测什么（不只是「能不能启动」）

| 测试类型 | 怎么做 | 典型判断 |
|---|---|---|
| **冒烟测试**（最常见） | 启动游戏 → 等进主菜单 → 通过 | 没崩溃 = 通过（就是 Lyra 的 `BootTest`） |
| **进关卡测试** | 加载指定地图 → 等 N 秒 → 通过 | 关卡能加载、无致命错误 |
| **联机测试** | `RequireRole(Server)` + `RequireRole(Client)` | 服务端起得来、客户端能连上 |
| **多客户端测试** | 起 2~4 个客户端 | 多人同房不崩 |
| **内存泄漏检测** | 跑 10 分钟，比对 `MemReport` | 内存是否单调上涨 |
| **性能回归** | 记录每帧耗时，与基线对比 | 是否变慢 |
| **截图对比** | `SetScreenshotPeriod(5.0f)` 定期截图 | 画面是否正常（需人看或图像比对） |
| **PGO 采样** | `Gauntlet.UnrealPGONode.cs` | 生成 Profile-Guided Optimization 数据 |

**引擎源码里的能力**（`GauntletModule.cpp:346-355`）：

```cpp
// 定期截图（SetScreenshotPeriod 开启后）
if (ScreenshotPeriod > 0.0f && (FPlatformTime::Seconds() - LastScreenshotTime) > ScreenshotPeriod)
{
    TakeScreenshot();
    LastScreenshotTime = FPlatformTime::Seconds();
}

// 定期打心跳日志（CI 卡住时能看出卡在哪一步）
if (HeartbeatPeriod > 0.0f && (FPlatformTime::Seconds() - LastHeartbeatTime) > HeartbeatPeriod)
{
    LogHeartbeat();
}
```

---

## 五、`Gauntlet` 与「普通单元测试」的区别（很多人搞混）

| | Gauntlet | UE 自动化测试框架（`IMPLEMENT_SIMPLE_AUTOMATION_TEST`） |
|---|---|---|
| 层级 | **集成测试 / 端到端** | **单元测试 / 功能测试** |
| 跑什么 | **真的启动整个游戏**（甚至打包后的 exe） | 在编辑器进程内跑测试函数 |
| 需要渲染？ | 可以开真渲染（也能 `-NullRHI`） | 不需要 |
| 需要打包？ | 通常测打包后的版本 | 不打包 |
| 能测网络？ | ✅ 起多进程 | ❌ 难 |
| 适合 CI？ | ✅ 就是为 CI 设计的 | ✅ 也能，但测不到「打包后能不能跑」 |
| 代码在哪 | C++ `UGauntletTestController` + C# `BaseTestNode` | C++ `IMPLEMENT_SIMPLE_AUTOMATION_TEST` |

🔑 **一句话区别**：
- 自动化测试框架问「**这个函数对不对**」
- Gauntlet 问「**这个打包好的游戏能不能跑起来、能不能玩**」

---

## 六、有它 vs 没它

### 6.1 编译层

| | 有 | 没有 |
|---|---|---|
| 编译产物 | 多一个 `UnrealEditor-Gauntlet.dll`（Runtime 模块） | — |
| 编译时间 | 略增（9 个源文件，约 2.5 万字节） | — |
| **打包体积** | **略增**（Runtime 模块，会进包里） | — |

> ⚠️ 与 `ActorPalette`（Editor 模块，零打包体积）**不同** —— `Gauntlet` 是 `Runtime`，打包时代码会进 exe/dll。
> 不过体积很小（几十 KB），可忽略。

### 6.2 为什么 `LyraGame.Build.cs` 要依赖它

`E:\ue5\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs:60`（我们第 04 轮讲义的第 10 个私有依赖）：

```csharp
PrivateDependencyModuleNames.AddRange(new string[] {
    ...
    "Gauntlet",     // ← 这里
    ...
});
```

因为 `Source\LyraGame\Tests\LyraTestControllerBootTest.h` 里：

```cpp
#include "GauntletTestControllerBootTest.h"   // 来自 Gauntlet 模块
```

**不依赖 → 编不过**（找不到头文件）。

### 6.3 运行时层

| | 有 | 没有 |
|---|---|---|
| 正常启动游戏 | 模块加载了，但**什么都不做**（没传 `-gauntlet=` 参数） | — |
| 传了 `-gauntlet=XXX` | 创建控制器并驱动 | 参数被忽略，无测试 |
| 对玩家 / 游戏逻辑 | **零影响** | — |

**源码证据**（`GauntletModule.cpp:173-180`）：

```cpp
void FGauntletModuleImpl::LoadControllers()
{
    FString ControllerString;
    FParse::Value(FCommandLine::Get(), TEXT("gauntlet="), ControllerString, false);

    if (ControllerString.Len())     // ★ 命令行没传 -gauntlet= 就整段跳过
    {
        ...
    }
}
```

> 🔑 **不传参数 = 完全空转。** 所以启用了它对正常游戏一点影响都没有。

---

## 七、你现在需要做什么

**什么都不用做。** 三条理由：

| # | 理由 |
|---|---|
| 1 | S00 阶段没有 `Source\LyraGame\Tests\` 目录，也没有任何 `UGauntletTestController` 子类 → Gauntlet 空转 |
| 2 | 没搭 CI → 不会有人传 `-gauntlet=` 参数 |
| 3 | 留它是「一比一」（Lyra `.uproject:39-42` 原样）+ `Build.cs` 依赖它（第 04 轮已列） |

### 什么时候会真正用到它

| 时机 | 场景 |
|---|---|
| **S21**（System 进阶阶段） | 会复刻 `Source\LyraGame\Tests\LyraTestControllerBootTest.h/.cpp` 这两个文件 |
| 你搭 CI 时 | 想在每次提交后自动验证「游戏还能启动」 |
| 做性能回归时 | 想自动记录每版本帧率 |

### 万一它出问题（症状 → 原因）

| 症状 | 原因 |
|---|---|
| 编译报「找不到 `GauntletTestControllerBootTest.h`」 | `Build.cs` 私有依赖漏了 `"Gauntlet"` |
| 传了 `-gauntlet=XXX` 但测试没跑 | ① 找不到匹配的 `UGauntletTestController` 子类（名字对不上）② `.uproject` 里 `"Enabled": false` |
| 游戏启动就退，退出码非 0 | 控制器里调了 `EndTest(非0值)` |
| 编辑器菜单里找不到 Gauntlet | **正常** —— 它没有编辑器 UI，纯代码框架 |

---

## 八、一句话总结

**`Gauntlet` = 「无人值守跑游戏并判成败」的自动化集成测试框架：C++ 侧写 `UGauntletTestController` 子类（游戏内导演），C# 侧写 `BaseTestNode` 子类（外部指挥），外部通过 `RunUAT.bat RunUnreal -test=XXX` 启动游戏并传 `-gauntlet=XXX` 命令行。不传参数就完全空转，所以启用它对正常开发零影响；Lyra 启用它是因为有 `Tests\LyraTestControllerBootTest` 需要它 + 一键冒烟测试。你 S00 阶段什么都不用做。**

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Gauntlet\Gauntlet.uplugin:1-25`（`Description` / `Category:Testing` / `EnabledByDefault:false` / `IsBetaVersion:true` / `Modules[0].Type:Runtime` / `LoadingPhase:PreDefault`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Gauntlet\Source\Gauntlet\Public\GauntletTestController.h:20-108`（`UGauntletTestController`：5 个可重写钩子 + `EndTest` / `MarkHeartbeatActive` / `GetCurrentMap`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Gauntlet\Source\Gauntlet\Public\GauntletModule.h:14-101`（`FGauntletStates` / `FGauntletModule` 接口）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Gauntlet\Source\Gauntlet\Public\GauntletTestControllerBootTest.h:8-17`（内置 BootTest 基类，`IsBootProcessComplete()` 纯虚钩子）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Gauntlet\Source\Gauntlet\Private\GauntletModule.cpp:173-210`（`LoadControllers`：解析 `-gauntlet=` + 按类名匹配）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Gauntlet\Source\Gauntlet\Private\GauntletModule.cpp:346-355`（定期截图 + 心跳日志）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\AutomationTool\Gauntlet\`（C# 工具链：`Framework\` / `Unreal\` / `Platform\` / `SelfTest\`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\AutomationTool\Gauntlet\Unreal\Base\Gauntlet.UnrealTestNode.cs:657-659`（`SessionRole.CommandLineParams.Add("gauntlet", ...)`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\AutomationTool\Gauntlet\SelfTest\Gauntlet.SelfTest.Blank.cs:15-22`（最小 C# 测试节点）
- `E:\ue5\LyraStarterGame\Source\LyraGame\Tests\LyraTestControllerBootTest.h:11-26`（Lyra 的控制器声明，`TestDelay = 20.0f`）
- `E:\ue5\LyraStarterGame\Source\LyraGame\Tests\LyraTestControllerBootTest.cpp:8-31`（`IsBootProcessComplete` 实现）
- `E:\ue5\LyraStarterGame\Build\Scripts\Automation\LyraTest.BootTest.cs:16-32`（`Client.Controllers.Add("BootTest")`）
- `E:\ue5\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs:60`（`"Gauntlet"` 私有依赖）
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject:39-42`（`"Name": "Gauntlet", "Enabled": true`）