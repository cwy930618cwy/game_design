# 11 — Misc 杂项详解（Misc/）

> **定位**：`Core/Public/Misc/`，**Core 里最大的目录**（181 个文件）。它装各种"没法归到容器/数学/线程"的**零散基础工具**。
>
> **一句话**：`Misc/` 是 Core 的"杂物间"，装的是**断言、配置、命令行、全局变量、路径、字符串编码**等杂七杂八但很常用的基础功能。
>
> **文件**：`Engine/Source/Runtime/Core/Public/Misc/`（181 个文件，最大目录）

---

## 一、为什么叫"杂项"？

`Misc` = Miscellaneous（杂项/杂七杂八）。Core 里有些工具**既不属于容器、也不属于数学、也不属于线程**，就统一塞到 Misc/ 里。所以它**很大但很杂**。

**虽然叫杂，但里面有你好几个"天天碰"的东西**：断言（check）、日志类别声明（其实是 Logging 的，但 Misc 也有）、配置读取、命令行、路径工具、文本编码……

---

## 二、Misc/ 里你会碰到的重点（配具体场景）

### 2.1 断言（check / ensure）—— 代码调试利器

断言 = "这里必须成立，否则报错"。帮你抓"不该发生却发生"的逻辑错误。

```cpp
#include "Misc/AssertionMacros.h"

// check：条件不成立就崩溃（Debug 模式）
check(Health > 0);          // 如果 Health <= 0，程序直接崩溃提示

// ensure：条件不成立就报错，但不崩溃（更温和）
ensure(Player != nullptr);  // 如果 Player 是空，打印错误但不崩
```

**区别**：

| | `check` | `ensure` |
|---|---|---|
| 不成立时 | **崩溃** | **报错但不崩** |
| 适合 | 必须永远成立的逻辑 | 可能偶发、但不能崩的情况 |

**具体场景：检查不该出现的错误**

```cpp
void TakeDamage(float Amount) {
    // 掉血不应该出现负数
    check(Amount >= 0.f);   // 如果是负数，说明逻辑错了，直接崩

    // 敌人引用可能失效，用 ensure 温和处理
    if (ensure(Enemy != nullptr)) {
        Enemy->ReactToHit();
    }
}
```

> **建议**：`check` 用在"必须成立"的硬逻辑，`ensure` 用在"可能出问题但别崩"的情况。日常写代码加断言能提前发现很多 bug。

### 2.2 配置读取（GConfig）—— 读取 ini 配置

UE 用 `.ini` 存配置，Misc 里提供读取接口。

```cpp
#include "Misc/ConfigCacheIni.h"

// 读取配置值
int32 MaxPlayers;
GConfig->GetInt(TEXT("/Script/MyGame.MySettings"), TEXT("MaxPlayers"), MaxPlayers, GGameIni);

// 写入配置
GConfig->SetInt(TEXT("/Script/MyGame.MySettings"), TEXT("MaxPlayers"), 10, GGameIni);
```

**具体场景：把游戏难度存到配置**

```cpp
// 读取玩家设置的音量
float Volume;
GConfig->GetFloat(TEXT("Audio"), TEXT("MasterVolume"), Volume, GGameIni);
```

> 新手可能用蓝图/存档系统更多，但看引擎/插件代码时会碰到 GConfig。

### 2.3 命令行（CommandLine）—— 启动参数

```cpp
#include "Misc/CommandLine.h"

// 判断是否加了启动参数
if (FCommandLine::HasParam(TEXT("-NoSound"))) {
    // 玩家用 -NoSound 启动，关声音
}
```

**具体场景：打包发布时用命令行控制**

```cpp
// 玩家用 -windowed 启动就窗口化
if (FCommandLine::HasParam(TEXT("-windowed"))) {
    SetWindowMode(true);
}
```

### 2.4 全局变量（CoreGlobals / Misc）—— GEngine、GWarn

```cpp
#include "Misc/...";

// 常见的全局单例
GEngine       // 引擎实例
GWarn         // 警告输出
GGameplayStatics  // 其实在别的模块
```

> 具体：`GEngine->AddOnScreenDebugMessage`（见 03 日志）就在 Misc 的全局变量里。

### 2.5 路径/编码/其他工具

```cpp
#include "Misc/Paths.h"

// 路径工具
FPaths::ProjectDir();       // 项目根目录
FPaths::GameContentDir();   // Content 目录
FPaths::ConvertRelativePathToFull(...);  // 相对转绝对路径
```

```cpp
#include "Misc/...";

// 字符串编码、进制转换、文件查找等杂工具
```

**具体场景：保存存档到项目目录**

```cpp
FString SavePath = FPaths::ProjectDir() + TEXT("Save/Game.sav");
// 用 FPaths 拼路径，跨平台安全
```

---

## 三、Misc/ 里还有什么（完整了解）

| 类别 | 代表内容 | 常用度 |
|------|---------|:---:|
| **断言** | check / ensure / verify | ⭐⭐⭐ |
| **配置** | GConfig、ConfigCacheIni | ⭐⭐ |
| **命令行** | FCommandLine | ⭐⭐ |
| **全局变量** | GEngine、GWarn | ⭐⭐ |
| **路径** | FPaths | ⭐⭐⭐ |
| **日志相关** | FOutputDevice | ⭐⭐ |
| **文本/编码** | FBase64、字符转换 | ⭐ |
| **时间/日期** | FDateTime | ⭐ |
| **其他工具** | 文件查找、随机、异常 | ⭐ |

> **必学的重点**：`check`/`ensure`（断言）、`FPaths`（路径）、`GConfig`（配置）、`FCommandLine`（命令行）。

---

## 四、日常开发要学吗？—— 分轻重

| 工具 | 日常用得勤吗 | 建议 |
|------|:---:|------|
| `check` / `ensure` | **常写** | **必学**（加断言防 bug） |
| `FPaths` | 拼路径常用 | **必学** |
| `GConfig` | 配置读写 | 了解 |
| `FCommandLine` | 启动参数 | 了解 |
| 编码/时间 | 少 | 遇到再看 |

**结论**：
1. **`check`/`ensure`（断言）和 `FPaths`（路径）是必学重点**
2. `GConfig`、`FCommandLine` 了解即可
3. Misc 里大部分杂工具**遇到再看**

---

## 五、常见误区

**① 用 check 判断"可能会发生"的情况（应该用 if）**
```cpp
// ❌ check 用于"必须成立"，不是普通判断
check(Player != nullptr);   // 如果玩家可能消失，不该用 check（会崩）
// ✅ 可能发生的情况用 if 处理
if (Player) { ... }
```

**② 忘了加断言（该 check 却全靠日志）**
```cpp
// ❌ 只在日志里打，不中断
UE_LOG(LogTemp, Error, TEXT("数据错误"));
// 继续跑，可能后续崩
// ✅ 硬逻辑错误用 check 立即中断
check(Data.IsValid());
```

**③ 手动拼路径（该用 FPaths）**
```cpp
// ❌ 手动拼 "C:/..." 不跨平台
FString Path = TEXT("C:/Game/Content/xxx");
// ✅ 用 FPaths 跨平台
FString Path = FPaths::ProjectDir() + TEXT("Content/xxx");
```

---

## 六、总结速查

```
Misc/ = 杂项（Core 最大目录，181 个文件）
├── 断言：check（崩）/ ensure（报错不崩）← 必学
├── 路径：FPaths::ProjectDir() 等          ← 必学
├── 配置：GConfig（读 ini）                ← 了解
├── 命令行：FCommandLine::HasParam         ← 了解
├── 全局变量：GEngine、GWarn
└── 其他：编码、时间、文件查找             ← 遇到再看

日常重点：check/ensure（断言）+ FPaths（路径）
```

**一句话**：`Misc/` 是 Core 的"杂物间"，虽然大但很杂。你日常最该掌握的是 **`check`/`ensure`（断言，抓 bug）和 `FPaths`（路径，跨平台拼路径）**，其他（配置、命令行、编码）遇到再看。

---

## 七、什么时候深入？

- **现在**：掌握 `check`/`ensure`（断言）和 `FPaths`（路径）
- **做配置系统**时：学 GConfig
- **打包/发布**时：学 FCommandLine
- **读引擎代码**时：遇到 Misc 各种工具再查

> 和其他笔记一样：**先会用高频的（check、FPaths），冷门的遇到再看**。Misc 虽然大，但你只需要掌握其中几个，不用全学。
