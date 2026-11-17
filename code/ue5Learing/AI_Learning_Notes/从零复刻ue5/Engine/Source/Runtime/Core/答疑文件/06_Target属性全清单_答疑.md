# 06 — `Target.*` 全部 296 个属性清单（一个不漏）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ReadOnlyTargetRules.cs`（**748 行 / 296 个 public 属性**，2026-11-16 脚本实查）
> 用途：**凡是 `Target.xxx` 都能在这份清单里查到**，不再「突然蹦出来一个属性不知道哪来的」
> **✅ 本节可独立编译**——纯 C# 属性清单。
>
> ⚠️ **本文件是「参考手册」，不是讲义**。教学时**只讲当前这一行用到的属性**，讲完登记到 `从零复刻ue5\Engine\属性教学台账.md`；**不要**把 296 个一次性倒给用户。
> 配套铁律：`教学经验_UE5复刻新对话必看\03_属性首次出现必须教学并登记.md`

---

## 一、怎么用这份清单

| 你看到 | 去哪查 |
|---|---|
| `Target.xxx` | **本文件**（296 个，全在 `ReadOnlyTargetRules.cs`） |
| `Target.WindowsPlatform.xxx` | `05_属性来源总地图_答疑.md` 第三节（78 个） |
| `Target.MacPlatform.xxx` / `IOSPlatform` / `LinuxPlatform` / `AndroidPlatform` | `05` 第四节 |
| 不带 `Target.` 前缀 | `05` 第五节（`ModuleRules.cs` 约 150 个） |

> 本文件按**用途分 21 组**，每组一张表：`属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs 用在哪`。

---

## 二、总览（21 组 / 296 个）

| 组 | 名称 | 数量 |
|---|---|---|
| 1 | 身份与路径 | 23 |
| 2 | 构建设置版本 | 3 |
| 3 | 平台子规则入口 | 5 |
| 4 | 插件 | 10 |
| 5 | 编译开关 `bCompile*` | 25 |
| 6 | 构建开关 `bBuild*` / `bForceBuild*` | 11 |
| 7 | 功能开关 `bUse*` | 34 |
| 8 | 代码包含 `bWith*` | 8 |
| 9 | 启用开关 `bEnable*` | 9 |
| 10 | 强制开关 `bForce*` | 10 |
| 11 | 优化 / 调试信息 | 26 |
| 12 | Unity 合批 | 16 |
| 13 | 警告级别 | 14 |
| 14 | 静态分析 | 6 |
| 15 | 语言标准 / CPU 架构 | 5 |
| 16 | 链接 / 模块合并 | 10 |
| 17 | 全局宏定义 | 2 |
| 18 | 构建环境 / 杂项开关 | 40 |
| 19 | 构建步骤 / 参数 / 路径 | 26 |
| 20 | 测试相关 | 7 |
| 21 | 其他 | 6 |

---

## 三、逐组清单

### 组 1 · 身份与路径（23 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `Name` | 44 | `string` | 目标名（如 `UnrealEditor`） | — |
| `OriginalName` | 46 | `string` | 原始目标名（未加平台后缀前） | — |
| `AdditionalData` | 54 | `object?` | 用户自定义数据，`Target.cs` 写、`Build.cs` 读 | — |
| `Platform` | 56 | `UnrealTargetPlatform` | **当前编译平台** | 第 18、67、73、116、134、180、201、235、241、253、279 行 |
| `Configuration` | 58 | `UnrealTargetConfiguration` | **当前编译配置** | 第 24、99、104、183、241、279 行 |
| `Architectures` | 60 | `UnrealArchitectures` | 架构集合（可能多个） | — |
| `IntermediateEnvironment` | 62 | `UnrealIntermediateEnvironment` | 中间产物环境（IWYU / NonUnity / Analyze…） | — |
| `Architecture` | 64 | `UnrealArch` | 单架构（多架构时抛异常） | — |
| `ProjectFile` | 66 | `FileReference?` | 项目 `.uproject` 文件 | — |
| `Version` | 68 | `ReadOnlyBuildVersion` | 构建版本号 | — |
| `Type` | 70 | `TargetType` | **目标类型**（Game/Editor/Client/Server/Program） | 第 104、183、260 行 |
| `Logger` | 73 | `ILogger` | 日志器 | — |
| `OutputFile` | 83 | `string?` | 输出文件路径 | — |
| `UndecoratedConfiguration` | 98 | `UnrealTargetConfiguration` | 未加修饰的配置（去掉 `-Platform-Arch` 后缀） | — |
| `DecoratedSeparator` | 100 | `string` | 配置名分隔符 | — |
| `ExeBinariesSubFolder` | 131 | `string` | 可执行文件在 `Binaries/` 下的子目录 | — |
| `GeneratedCodeVersion` | 133 | `EGeneratedCodeVersion` | UHT 生成代码版本 | — |
| `BuildVersion` | 566 | `string?` | 构建版本字符串 | — |
| `GeneratedProjectName` | 614 | `string?` | 生成的工程名 | — |
| `SolutionDirectory` | 125 | `string` | 解决方案目录 | — |
| `CustomConfig` | 127 | `string` | 自定义配置名 | — |
| `PakSigningKeysFile` | 123 | `string` | Pak 签名密钥文件 | — |
| `bBuildInSolutionByDefault` | 129 | `bool?` | 是否默认加入解决方案 | — |

### 组 2 · 构建设置版本（3 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `DefaultBuildSettings` | 75 | `BuildSettingsVersion` | 默认构建设置版本（V1~V5） | — |
| `ForcedIncludeOrder` | 77 | `EngineIncludeOrderVersion?` | 命令行强制的 include 顺序 | — |
| `IncludeOrderVersion` | 79 | `EngineIncludeOrderVersion` | 实际使用的 include 顺序 | — |

### 组 3 · 平台子规则入口（5 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `WindowsPlatform` | 624 | `ReadOnlyWindowsTargetRules` | **Windows 专属设置**（78 个属性） | 第 79、94、99、306 行 |
| `MacPlatform` | 622 | `ReadOnlyMacTargetRules` | Mac 专属设置（6 个） | — |
| `IOSPlatform` | 620 | `ReadOnlyIOSTargetRules` | iOS 专属设置（13 个） | — |
| `LinuxPlatform` | 618 | `ReadOnlyLinuxTargetRules` | Linux 专属设置（8 个） | — |
| `AndroidPlatform` | 616 | `ReadOnlyAndroidTargetRules` | Android 专属设置（8 个） | — |

### 组 4 · 插件（10 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `AdditionalPlugins` | 106 | `IReadOnlyList<string>` | 额外要编的插件 | — |
| `EnablePlugins` | 108 | `IReadOnlyList<string>` | 强制启用的插件 | — |
| `DisablePlugins` | 110 | `IReadOnlyList<string>` | 强制禁用的插件 | — |
| `OptionalPlugins` | 112 | `IReadOnlyList<string>` | 可选插件 | — |
| `bAllowEnginePluginsEnabledByDefault` | 114 | `bool` | 是否允许引擎插件默认启用 | — |
| `DisablePluginsConflictWarningLevel` | 116 | `WarningLevel` | 插件冲突警告级别 | — |
| `BuildPlugins` | 118 | `IReadOnlyList<string>` | 实际参与构建的插件 | — |
| `InternalPluginDependencies` | 120 | `IReadOnlyList<string>` | 内部插件依赖 | — |
| `bRuntimeDependenciesComeFromBuildPlugins` | 121 | `bool` | 运行时依赖是否来自构建插件 | — |
| `bIncludePluginsForTargetPlatforms` | 256 | `bool` | 是否为目标平台包含插件 | — |

### 组 5 · 编译开关 `bCompile*`（25 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `bCompileAgainstEngine` | 210 | `bool` | **是否和引擎一起编** | 第 229 行 |
| `bCompileAgainstCoreUObject` | 212 | `bool` | 是否依赖 CoreUObject | — |
| `bCompileAgainstApplicationCore` | 214 | `bool` | 是否依赖 ApplicationCore | 第 155 行 |
| `bCompileAgainstEditor` | 220 | `bool` | 是否依赖编辑器 | — |
| `bCompileICU` | 169 | `bool` | **是否编 ICU（国际化）** | 第 222、226 行 |
| `bCompileCEF3` | 171 | `bool` | 是否编 CEF3（内嵌浏览器） | — |
| `bCompileISPC` | 173 | `bool` | 是否编 ISPC（向量化语言） | — |
| `bCompilePython` | 188 | `bool` | 是否编 Python 支持 | — |
| `bCompileChaos` | 146 | `bool` | 是否编 Chaos 物理 | — |
| `bCompileChaosVisualDebuggerSupport` | 153 | `bool` | 是否编 Chaos 可视化调试器 | — |
| `bCompilePhysX` | 161 | `bool` | 是否编 PhysX（旧物理） | — |
| `bCompileAPEX` | 164 | `bool` | 是否编 APEX（旧布料/破碎） | — |
| `bCompileNvCloth` | 167 | `bool` | 是否编 NvCloth（旧布料） | — |
| `bCompileRecast` | 222 | `bool` | 是否编 Recast（导航网格） | — |
| `bCompileNavmeshSegmentLinks` | 224 | `bool` | 是否编导航网格线段连接 | — |
| `bCompileNavmeshClusterLinks` | 226 | `bool` | 是否编导航网格簇连接 | — |
| `bCompileSpeedTree` | 228 | `bool` | 是否编 SpeedTree | — |
| `bCompileFreeType` | 289 | `bool` | 是否编 FreeType（字体） | — |
| `bCompileForSize` | 294 | `bool` | 是否为体积优化编译 | — |
| `bCompileWithStatsWithoutEngine` | 252 | `bool` | 不依赖引擎也能用 Stats | — |
| `bCompileWithPluginSupport` | 254 | `bool` | 是否支持插件 | — |
| `bCompileWithAccessibilitySupport` | 258 | `bool` | 是否支持无障碍功能 | — |
| `bCompileCustomSQLitePlatform` | 206 | `bool` | 是否编自定义 SQLite 平台层 | — |
| `bCompileIntelMetricsDiscovery` | 186 | `bool` | 是否编 Intel 指标发现 | — |
| `bCompileIoStoreOnDemand` | 179 | `bool` | 是否编 IoStore 按需加载 | — |

### 组 6 · 构建开关 `bBuild*` / `bForceBuild*`（11 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `bBuildEditor` | 190 | `bool` | **是否编编辑器** | 第 61、128 行 |
| `bBuildRequiresCookedData` | 192 | `bool` | **是否要求已 cook 的数据** | 第 229 行 |
| `bBuildWithEditorOnlyData` | 194 | `bool` | 是否带编辑器专用数据 | — |
| `bBuildDeveloperTools` | 196 | `bool` | 是否编开发者工具 | — |
| `bBuildTargetDeveloperTools` | 198 | `bool` | 是否编目标开发者工具 | — |
| `bBuildAllModules` | 104 | `bool` | 是否编所有模块 | — |
| `bBuildAdditionalConsoleApp` | 332 | `bool` | 是否额外编控制台程序 | — |
| `bBuildConsoleAppOnly` | 334 | `bool` | 是否只编控制台程序 | — |
| `bForceBuildTargetPlatforms` | 200 | `bool` | 强制编所有目标平台模块 | — |
| `bForceBuildShaderFormats` | 202 | `bool` | 强制编所有着色器格式模块 | — |
| `bNeedsExtraShaderFormats` | 204 | `bool` | 是否需要额外着色器格式 | — |### 组 7 · 功能开关 `bUse*`（34 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `bUseIris` | 181 | `bool` | **是否用 Iris 网络复制** | 第 270 行 |
| `bUseVerseVM` | 138 | `bool` | 是否用 Verse 虚拟机 | — |
| `bUseVerseBPVM` | 136 | `bool` | 是否用 Verse 蓝图虚拟机 | — |
| `bUseAutoRTFMCompiler` | 140 | `bool` | 是否用 AutoRTFM 编译器 | — |
| `bUseAutoRTFMVerifier` | 141 | `bool` | 是否用 AutoRTFM 校验器 | — |
| `bUseChaos` | 149 | `bool` | 是否用 Chaos 物理 | — |
| `bUseChaosMemoryTracking` | 151 | `bool` | 是否用 Chaos 内存追踪 | — |
| `bUseChaosChecked` | 155 | `bool` | 是否用 Chaos 检查版 | — |
| `bUseGameplayDebugger` | 175 | `bool` | 是否用 GameplayDebugger | — |
| `bUseGameplayDebuggerCore` | 177 | `bool` | 是否用 GameplayDebugger 核心 | — |
| `bUseStaticCRT` | 92 | `bool` | 是否静态链接 CRT | — |
| `bUseUnityBuild` | 338 | `bool` | 是否用 Unity 合批 | — |
| `bUseSharedPCHs` | 492 | `bool` | 是否用共享 PCH | — |
| `bUsePCHFiles` | 429 | `bool` | 是否用 PCH 文件 | — |
| `bUsePDBFiles` | 427 | `bool` | 是否生成 PDB | — |
| `bUseVFS` | 433 | `bool` | 是否用虚拟文件系统 | — |
| `bUseInlining` | 242 | `bool` | 是否允许内联 | — |
| `bUseMallocProfiler` | 488 | `bool` | 是否用内存分配分析器 | — |
| `bUseFastMonoCalls` | 397 | `bool` | 是否用快速单体调用 | — |
| `bUseIncrementalLinking` | 463 | `bool` | 是否增量链接 | — |
| `bUseHeaderUnitsForPch` | 522 | `bool` | 是否用 C++20 头单元做 PCH | — |
| `bUseAdaptiveUnityBuild` | 532 | `bool` | 是否用自适应 Unity | — |
| `bUseXGEController` | 308 | `bool` | 是否用 XGE 分布式编译 | — |
| `bUseCacheFreedOSAllocs` | 208 | `bool` | 是否缓存已释放的 OS 分配 | — |
| `bUseEstimatedUtcNow` | 287 | `bool` | 是否用估算的 UTC 时间 | — |
| `bUseLauncherChecks` | 277 | `bool` | 是否用启动器检查 | — |
| `bUseChecksInShipping` | 279 | `bool` | Shipping 是否保留 check | — |
| `bUseLoggingInShipping` | 271 | `bool` | Shipping 是否保留日志 | — |
| `bUseConsoleInShipping` | 273 | `bool` | Shipping 是否保留控制台 | — |
| `bUseExecCommandsInShipping` | 291 | `bool` | Shipping 是否保留 exec 命令 | — |
| `bUseDebugLiveCodingConsole` | 264 | `bool` | 是否用 LiveCoding 调试控制台 | — |
| `bUseFastPDBLinking` | 502 | `bool?` | 是否快速 PDB 链接 | — |
| `bUseShippingPhysXLibraries` | 494 | `bool` | 是否用 Shipping 版 PhysX 库 | — |
| `bUseCheckedPhysXLibraries` | 496 | `bool` | 是否用 Checked 版 PhysX 库 | — |

### 组 8 · 代码包含 `bWith*`（8 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `bWithClientCode` | 244 | `bool` | 是否包含客户端代码 | — |
| `bWithServerCode` | 246 | `bool` | 是否包含服务器代码 | — |
| `bWithPushModel` | 250 | `bool` | 是否用 Push Model（网络属性） | — |
| `bWithPerfCounters` | 260 | `bool` | 是否用性能计数器 | — |
| `bWithLiveCoding` | 262 | `bool` | 是否用 LiveCoding | — |
| `bWithDirectXMath` | 267 | `bool` | 是否用 DirectXMath | — |
| `bWithFixedTimeStepSupport` | 269 | `bool` | 是否支持固定时间步 | — |
| `bWithAssembly` | 443 | `bool` | 是否输出汇编 | — |

### 组 9 · 启用开关 `bEnable*`（9 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `bEnableTrace` | 216 | `bool` | 是否启用 Trace（性能追踪） | — |
| `bEnablePIE` | 238 | `bool` | 是否启用 PIE（地址随机化） | — |
| `bEnableStackProtection` | 240 | `bool` | 是否启用栈保护 | — |
| `bEnablePrivateBuildInformation` | 236 | `bool` | 是否启用私有构建信息 | — |
| `bEnableMeshEditor` | 134 | `bool` | 是否启用 Mesh 编辑器 | — |
| `bEnableCppModules` | 481 | `bool` | 是否启用 C++20 模块 | — |
| `bEnableCppCoroutinesForEvaluation` | 483 | `bool` | 是否启用 C++ 协程（评估用） | — |
| `bEnableProcessPriorityControl` | 485 | `bool` | 是否启用进程优先级控制 | — |
| `bEnableOSX109Support` | 328 | `bool` | 是否支持 OSX 10.9 | — |

### 组 10 · 强制开关 `bForce*`（10 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `bForceEnableTrace` | 218 | `bool` | 强制启用 Trace | — |
| `bForceEnableExceptions` | 230 | `bool` | 强制启用 C++ 异常 | — |
| `bForceEnableObjCExceptions` | 232 | `bool` | 强制启用 ObjC 异常 | — |
| `bForceEnableRTTI` | 234 | `bool` | 强制启用 RTTI | — |
| `bForceUnityBuild` | 340 | `bool` | 强制 Unity 合批 | — |
| `bForceCompileDevelopmentAutomationTests` | 302 | `bool` | 强制编 Development 自动化测试 | — |
| `bForceCompilePerformanceAutomationTests` | 304 | `bool` | 强制编性能自动化测试 | — |
| `bForceDisableAutomationTests` | 306 | `bool` | 强制禁用自动化测试 | — |
| `bForcePrecompiledHeaderForGameModules` | 461 | `bool` | 强制游戏模块用 PCH | — |
| `bForceIncludePCHHeadersForGenCppFilesWhenPCHIsDisabled` | 437 | `bool` | PCH 关闭时强制包含 PCH 头 | — |

### 组 11 · 优化 / 调试信息（26 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `OptimizationLevel` | 296 | `OptimizationMode` | 优化倾向（Speed/Size/SizeAndSpeed） | — |
| `FPSemantics` | 298 | `FPSemanticsMode` | 浮点语义（Default/Precise/Imprecise） | — |
| `bAllowLTCG` | 465 | `bool` | **是否允许 LTCG（链接时代码生成）** | 第 294 行 |
| `bPGOOptimize` | 473 | `bool` | **是否 PGO 优化** | 第 295 行 |
| `bPGOProfile` | 471 | `bool` | **是否 PGO 采样** | 第 296 行 |
| `bPreferThinLTO` | 467 | `bool` | 是否优先 ThinLTO | — |
| `bCodeCoverage` | 477 | `bool` | 是否代码覆盖率 | — |
| `bDisableDebugInfo` | 409 | `bool` | 是否禁用调试信息 | — |
| `DebugInfo` | 411 | `DebugInfoMode` | 调试信息范围（`[Flags]`） | — |
| `DisableDebugInfoModules` | 413 | `IReadOnlySet<string>` | 禁用调试信息的模块 | — |
| `DisableDebugInfoPlugins` | 415 | `IReadOnlySet<string>` | 禁用调试信息的插件 | — |
| `DebugInfoLineTablesOnly` | 417 | `DebugInfoMode` | 只生成行表的范围 | — |
| `DebugInfoLineTablesOnlyModules` | 419 | `IReadOnlySet<string>` | 只生成行表的模块 | — |
| `DebugInfoLineTablesOnlyPlugins` | 421 | `IReadOnlySet<string>` | 只生成行表的插件 | — |
| `bDisableDebugInfoForGeneratedCode` | 423 | `bool` | 生成代码是否禁用调试信息 | — |
| `bOmitPCDebugInfoInDevelopment` | 425 | `bool` | Development 是否省略 PC 调试信息 | — |
| `bDeterministic` | 431 | `bool` | 是否确定性构建 | — |
| `bOmitFramePointers` | 479 | `bool` | 是否省略帧指针 | — |
| `bRetainFramePointers` | 300 | `bool` | 是否保留帧指针 | — |
| `bCompressDebugFile` | 469 | `bool` | 是否压缩调试文件 | — |
| `bCreateMapFile` | 504 | `bool` | 是否生成 map 文件 | — |
| `bSupportEditAndContinue` | 475 | `bool` | 是否支持 Edit and Continue | — |
| `bAllowRuntimeSymbolFiles` | 506 | `bool` | 是否允许运行时符号文件 | — |
| `bPublicSymbolsByDefault` | 540 | `bool` | 符号是否默认公开 | — |
| `bStripExports` | 570 | `bool` | 是否剥离导出符号 | — |
| `bShaderCompilerWorkerTrace` | 490 | `bool` | 着色器编译 worker 是否追踪 | — |

### 组 12 · Unity 合批（16 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `NumIncludedBytesPerUnityCPP` | 399 | `int` | **每个 unity cpp 的字节上限** | —（模块用 Override 覆盖） |
| `bDisableModuleNumIncludedBytesPerUnityCPPOverride` | 401 | `bool` | 是否禁止模块覆盖上面这个值 | — |
| `MinGameModuleSourceFilesForUnityBuild` | 358 | `int` | 游戏模块启用 Unity 的最少文件数 | — |
| `MinFilesUsingPrecompiledHeader` | 459 | `int` | 使用 PCH 的最少文件数 | — |
| `bStressTestUnity` | 403 | `bool` | 是否压力测试 Unity | — |
| `bDetailedUnityFiles` | 406 | `bool` | 是否生成详细 Unity 文件 | — |
| `bAlwaysUseUnityForGeneratedFiles` | 342 | `bool` | 生成文件是否总用 Unity | — |
| `DisableMergingModuleAndGeneratedFilesInUnityFiles` | 344 | `IReadOnlyList<string>?` | 不合并模块与生成文件的模块列表 | — |
| `bAdaptiveUnityDisablesOptimizations` | 346 | `bool` | 自适应 Unity 是否禁用优化 | — |
| `bAdaptiveUnityDisablesPCH` | 348 | `bool` | 自适应 Unity 是否禁用 PCH | — |
| `bAdaptiveUnityDisablesPCHForProject` | 350 | `bool` | 项目模块是否禁用 PCH | — |
| `bAdaptiveUnityCreatesDedicatedPCH` | 352 | `bool` | 自适应 Unity 是否建专用 PCH | — |
| `bAdaptiveUnityEnablesEditAndContinue` | 354 | `bool` | 自适应 Unity 是否启用 E&C | — |
| `bAdaptiveUnityCompilesHeaderFiles` | 356 | `bool` | 自适应 Unity 是否编头文件 | — |
| `DisableUnityBuildForModules` | 632 | `IReadOnlyList<string>?` | 禁用 Unity 的模块列表 | — |
| `bChainPCHs` | 435 | `bool` | 是否链式 PCH | — |

### 组 13 · 警告级别（14 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `CppCompileWarningSettings` | 367 | `ReadOnlyCppCompileWarnings` | **警告设置总对象** | —（模块侧用 `ModuleRules.CppCompileWarningSettings`） |
| `DefaultWarningLevel` | 362 | `WarningLevel` | 默认警告级别 | — |
| `DeprecationWarningLevel` | 365 | `WarningLevel` | 废弃警告级别 | — |
| `ShadowVariableWarningLevel` | 370 | `WarningLevel` | 影子变量警告级别 | — |
| `UnsafeTypeCastWarningLevel` | 373 | `WarningLevel` | 不安全类型转换警告级别 | — |
| `UndefinedIdentifierWarningLevel` | 376 | `WarningLevel` | 未定义标识符警告级别 | — |
| `PCHPerformanceIssueWarningLevel` | 382 | `WarningLevel` | PCH 性能问题警告级别 | — |
| `ModuleIncludePathWarningLevel` | 385 | `WarningLevel` | 模块 include 路径警告级别 | — |
| `ModuleIncludePrivateWarningLevel` | 388 | `WarningLevel` | 私有 include 警告级别 | — |
| `ModuleIncludeSubdirectoryWarningLevel` | 391 | `WarningLevel` | 子目录 include 警告级别 | — |
| `bWarningsAsErrors` | 395 | `bool` | 是否警告当错误 | — |
| `bValidateFormatStrings` | 360 | `bool` | 是否校验 `UE_LOG` 格式串 | — |
| `bUndefinedIdentifierErrors` | 379 | `bool` | 未定义标识符是否当错误 | — |
| `bShowIncludes` | 393 | `bool` | 是否显示 include 树 | — |

### 组 14 · 静态分析（6 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `StaticAnalyzer` | 447 | `StaticAnalyzer` | **静态分析器选择** | 第 88 行 |
| `StaticAnalyzerOutputType` | 449 | `StaticAnalyzerOutputType` | 输出格式（Text/Html） | — |
| `StaticAnalyzerMode` | 451 | `StaticAnalyzerMode` | 分析深度（Deep/Shallow） | — |
| `StaticAnalyzerPVSPrintLevel` | 453 | `int` | PVS 输出级别 | — |
| `bStaticAnalyzerProjectOnly` | 455 | `bool` | 只分析项目模块 | — |
| `bStaticAnalyzerIncludeGenerated` | 457 | `bool` | 是否分析生成代码 | — |

### 组 15 · 语言标准 / CPU 架构（5 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `CppStandard` | 556 | `CppStandardVersion` | C++ 标准（Cpp17/Cpp20/Latest） | — |
| `CppStandardEngine` | 554 | `CppStandardVersion` | 引擎模块的 C++ 标准 | — |
| `CStandard` | 558 | `CStandardVersion` | C 标准（C89~C17） | — |
| `MinCpuArchX64` | 560 | `MinimumCpuArchitectureX64` | x64 最低 CPU 架构 | — |
| `MinArm64CpuTarget` | 562 | `Arm64TargetCpuType` | ARM64 目标 CPU 类型 | — |

### 组 16 · 链接 / 模块合并（10 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `LinkType` | 568 | `TargetLinkType` | 链接方式（Default/Monolithic/Modular） | — |
| `bMergeModules` | 572 | `bool` | **是否合并模块** | 第 311 行 |
| `MergePlugins` | 574 | `IReadOnlyList<string>` | 参与合并的插件 | — |
| `MergePluginsIgnored` | 576 | `IReadOnlyList<string>` | 不参与合并的插件 | — |
| `MergePluginsShared` | 578 | `IReadOnlyDictionary<string, IEnumerable<string>>` | 共享合并的插件 | — |
| `MergePluginsLaunch` | 580 | `IReadOnlyList<string>` | 启动时合并的插件 | — |
| `bShouldCompileAsDLL` | 626 | `bool` | 是否编成 DLL | — |
| `bDisableLinking` | 526 | `bool` | 是否禁用链接 | — |
| `bIgnoreBuildOutputs` | 528 | `bool` | 是否忽略构建产物 | — |
| `bFormalBuild` | 530 | `bool` | 是否正式构建 | — |

### 组 17 · 全局宏定义（2 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `GlobalDefinitions` | 582 | `IReadOnlyList<string>` | 全局宏定义 | — |
| `ProjectDefinitions` | 584 | `IReadOnlyList<string>` | 项目级宏定义 | — |

### 组 18 · 构建环境 / 杂项开关（40 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `BuildEnvironment` | 596 | `TargetBuildEnvironment` | 构建环境（Shared/Unique/UniqueIfNeeded） | — |
| `bOverrideBuildEnvironment` | 598 | `bool` | 是否覆盖构建环境 | — |
| `bIsEngineInstalled` | 630 | `bool` | 是否安装版引擎 | — |
| `bGenerateProjectFiles` | 628 | `bool` | 是否生成工程文件 | — |
| `bAllowHotReload` | 102 | `bool` | 是否允许热重载 | — |
| `bLegalToDistributeBinary` | 96 | `bool` | 是否可合法分发二进制 | — |
| `bUsesSteam` | 86 | `bool` | 是否用 Steam（已废弃） | — |
| `bUsesCEF3` | 88 | `bool` | 是否用 CEF3 | — |
| `bUsesSlate` | 90 | `bool` | 是否用 Slate | — |
| `bDebugBuildsActuallyUseDebugCRT` | 94 | `bool` | Debug 是否真用 Debug CRT | — |
| `bTCHARIsUTF8` | 285 | `bool` | TCHAR 是否 UTF-8 | — |
| `bFNameOutlineNumber` | 248 | `bool` | FName 是否带 outline number | — |
| `bEventDrivenLoader` | 310 | `bool` | 是否事件驱动加载 | — |
| `bIWYU` | 314 | `bool` | 是否启用 IWYU | — |
| `bEnforceIWYU` | 320 | `bool` | 是否强制 IWYU | — |
| `bIncludeHeaders` | 316 | `bool` | 是否包含头文件 | — |
| `bHeadersOnly` | 318 | `bool` | 是否只编头文件 | — |
| `bWarnAboutMonolithicHeadersIncluded` | 322 | `bool` | 单体头文件是否警告 | — |
| `bHasExports` | 324 | `bool` | 是否有导出符号 | — |
| `bPrecompile` | 326 | `bool` | 是否预编译 | — |
| `bIsBuildingConsoleApplication` | 330 | `bool` | 是否编控制台程序 | — |
| `bDisableSymbolCache` | 336 | `bool` | 是否禁用符号缓存 | — |
| `bDeployAfterCompile` | 518 | `bool` | 编译后是否部署 | — |
| `bAllowRemotelyCompiledPCHs` | 520 | `bool` | 是否允许远程编 PCH | — |
| `bCheckSystemHeadersForModification` | 524 | `bool` | 是否检查系统头改动 | — |
| `bFlushBuildDirOnRemoteMac` | 534 | `bool` | 远程 Mac 是否清构建目录 | — |
| `bPrintToolChainTimingInfo` | 536 | `bool` | 是否打印工具链耗时 | — |
| `bParseTimingInfoForTracing` | 538 | `bool` | 是否解析耗时用于追踪 | — |
| `bLegacyPublicIncludePaths` | 548 | `bool` | 是否用旧版 Public include 路径 | — |
| `bLegacyParentIncludePaths` | 550 | `bool` | 是否用旧版父目录 include 路径 | — |
| `bDisableEngineDeprecations` | 552 | `bool` | 是否禁用引擎废弃警告 | — |
| `bCheckLicenseViolations` | 498 | `bool` | 是否检查许可违规 | — |
| `bBreakBuildOnLicenseViolation` | 500 | `bool` | 许可违规是否中断构建 | — |
| `bAllowUbaCompression` | 445 | `bool` | 是否允许 UBA 压缩 | — |
| `bPreprocessOnly` | 439 | `bool` | 是否只预处理 | — |
| `bPreprocessDepends` | 441 | `bool` | 是否预处理依赖 | — |
| `bAllowProfileGPUInTest` | 281 | `bool` | Test 是否允许 GPU 分析 | — |
| `bAllowProfileGPUInShipping` | 283 | `bool` | Shipping 是否允许 GPU 分析 | — |
| `bLoggingToMemoryEnabled` | 275 | `bool` | 是否启用内存日志 | — |
| `bTrackRHIResourceInfoForTest` | 183 | `bool` | 是否追踪 RHI 资源信息 | — |
| `bCustomSceneQueryStructure` | 158 | `bool` | 是否自定义场景查询结构 | — |

### 组 19 · 构建步骤 / 参数 / 路径（26 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `PreBuildTargets` | 600 | `IReadOnlyList<TargetInfo>` | 预构建目标 | — |
| `PreBuildSteps` | 602 | `IReadOnlyList<string>` | 预构建步骤 | — |
| `PostBuildSteps` | 604 | `IReadOnlyList<string>` | 后构建步骤 | — |
| `AdditionalBuildProducts` | 606 | `IReadOnlyList<string>` | 附加构建产物 | — |
| `AdditionalCompilerArguments` | 608 | `string?` | 附加编译器参数 | — |
| `AdditionalLinkerArguments` | 610 | `string?` | 附加链接器参数 | — |
| `MemoryPerActionGB` | 612 | `double` | 每个编译动作的内存（GB） | — |
| `MSVCCompileActionWeight` | 544 | `float` | MSVC 编译动作权重 | — |
| `ClangCompileActionWeight` | 546 | `float` | Clang 编译动作权重 | — |
| `ToolChainName` | 542 | `string?` | 工具链名 | — |
| `PackagePath` | 508 | `string?` | 打包路径 | — |
| `CrashDiagnosticDirectory` | 510 | `string?` | 崩溃诊断目录 | — |
| `ThinLTOCacheDirectory` | 512 | `string?` | ThinLTO 缓存目录 | — |
| `ThinLTOCachePruningArguments` | 514 | `string?` | ThinLTO 缓存清理参数 | — |
| `BundleVersion` | 516 | `string?` | 包版本 | — |
| `LaunchModuleName` | 586 | `string?` | 启动模块名 | — |
| `ExportPublicHeader` | 588 | `string?` | 导出公开头文件 | — |
| `ExtraModuleNames` | 590 | `IReadOnlyList<string>` | 额外模块名 | — |
| `ManifestFileNames` | 592 | `IReadOnlyList<FileReference>` | 清单文件名 | — |
| `DependencyListFileNames` | 594 | `IReadOnlyList<FileReference>` | 依赖列表文件名 | — |
| `RelativeEnginePath` | 652 | `string` | **引擎根相对当前目录** | 第 29 行 |
| `UEThirdPartySourceDirectory` | 657 | `string` | `"ThirdParty/"` | — |
| `UEThirdPartyBinariesDirectory` | 662 | `string` | `"../Binaries/ThirdParty/"` | — |
| `EnableOptimizeCodeForModules` | 634 | `IReadOnlyList<string>?` | 强制优化的模块 | — |
| `DisableOptimizeCodeForModules` | 636 | `IReadOnlyList<string>?` | 强制不优化的模块 | — |
| `OptedInModulePlatforms` | 642 | `IReadOnlyList<UnrealTargetPlatform>?` | 选定加入的模块平台 | — |

### 组 20 · 测试相关（7 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `IsTestTarget` | 667 | `bool` | 是否测试目标 | — |
| `ExplicitTestsTarget` | 674 | `bool` | 是否显式测试目标 | — |
| `WithLowLevelTests` | 679 | `bool` | 是否带低级测试 | — |
| `InnerTestTargetRules` | 644 | `TestTargetRules?` | 内部测试规则 | — |
| `SupportedPlatforms` | 684 | `IEnumerable<UnrealTargetPlatform>` | 支持的平台 | — |
| `SupportedConfigurations` | 689 | `IEnumerable<UnrealTargetConfiguration>` | 支持的配置 | — |
| `StaticAllocator` | 746 | `StaticAllocatorType` | 静态分配器类型 | — |

### 组 21 · 其他（6 个）

| 属性 | 行号 | 类型 | 一句话说明 | Core.Build.cs |
|---|---|---|---|---|
| `OptimizeForSizeModules` | 638 | `IReadOnlyList<string>?` | 按体积优化的模块 | — |
| `OptimizeForSizeAndSpeedModules` | 640 | `IReadOnlyList<string>?` | 按体积+速度优化的模块 | — |
| `NativePointerMemberBehaviorOverride` | 312 | `PointerMemberBehavior?` | 原生指针成员行为覆盖 | — |
| `bAutoRTFMVerify` | 142 | `bool` | 是否 AutoRTFM 校验 | — |
| `bAutoRTFMClosedStaticLinkage` | 143 | `bool` | AutoRTFM 是否封闭静态链接 | — |
| `bRuntimeDependenciesComeFromBuildPlugins` | 121 | `bool` | 运行时依赖是否来自构建插件 | — |

---

## 四、`Core.Build.cs` 实际用到的 20 个（速查）

| 属性 | 行号 | 用在哪 |
|---|---|---|
| `Platform` | 56 | 第 18、67、73、116、134、180、201、235、241、253、279 行 |
| `Configuration` | 58 | 第 24、99、104、183、241、279 行 |
| `Type` | 70 | 第 104、183、260 行 |
| `bBuildEditor` | 190 | 第 61、128 行 |
| `bBuildRequiresCookedData` | 192 | 第 229 行 |
| `bCompileAgainstEngine` | 210 | 第 229 行 |
| `bCompileAgainstApplicationCore` | 214 | 第 155 行 |
| `bCompileICU` | 169 | 第 222、226 行 |
| `bUseIris` | 181 | 第 270 行 |
| `bMergeModules` | 572 | 第 311 行 |
| `bAllowLTCG` | 465 | 第 294 行 |
| `bPGOOptimize` | 473 | 第 295 行 |
| `bPGOProfile` | 471 | 第 296 行 |
| `StaticAnalyzer` | 447 | 第 88 行 |
| `RelativeEnginePath` | 652 | 第 29 行 |
| `WindowsPlatform` | 624 | 第 79、94、99、306 行 |
| `WindowsPlatform.Architecture` | 945（`UEBuildWindows.cs`） | 第 79 行 |
| `WindowsPlatform.bUseBundledDbgHelp` | 1059（同上） | 第 94 行 |
| `WindowsPlatform.bPixProfilingEnabled` | 959（同上） | 第 99 行 |
| `WindowsPlatform.bEnableInstrumentation` | 985（同上） | 第 306 行 |

---

## 五、速查表 · 口诀

| 前缀 | 来源 | 数量 |
|---|---|---|
| `Target.` | `ReadOnlyTargetRules.cs` | **296** |
| `Target.WindowsPlatform.` | `UEBuildWindows.cs:902` | 78 |
| `Target.IOSPlatform.` | `UEBuildIOS.cs:177` | 13 |
| `Target.LinuxPlatform.` | `UEBuildLinux.cs:115` | 8 |
| `Target.AndroidPlatform.` | `UEBuildAndroid.cs:70` | 8 |
| `Target.MacPlatform.` | `UEBuildMac.cs:93` | 6 |
| 无前缀 | `ModuleRules.cs` | 约 150 |

**口诀**：**`Target.` 后面 296 个全在 `ReadOnlyTargetRules.cs`；`bCompile*` 管编什么、`bBuild*` 管构建什么、`bUse*` 管用什么、`bEnable*` 管开什么、`bForce*` 管强制什么、`bWith*` 管带什么。**

---

## 待确认

1. 296 个属性全列完了，还有哪个没讲到的告诉我。
2. 重点记：`bCompile*`（25）/ `bBuild*`（11）/ `bUse*`（34）/ `bEnable*`（9）/ `bForce*`（10）/ `bWith*`（8）——这 6 组前缀覆盖了 97 个开关。
3. 看完说「下一步」，回主线继续 `Core.Build.cs` 验收 → `Core_APL.xml`（73 行）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ReadOnlyTargetRules.cs`（748 行 / 296 个 public 属性，2026-11-16 脚本实查）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Platform\Windows\UEBuildWindows.cs`（`ReadOnlyWindowsTargetRules` 902 行起）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（330 行）