# 00 — `ModuleRules` —— 总 md（答疑）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs`（**2119 行 / 76049 字节**）
> 你新建文件：`ue_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs`（答疑用，非当前主线）
> **✅ 本节可独立编译**——UBT 自己的 C# 源码，由 .NET 编译，不参与 C++ 编译。

---

## 一、这个类解决什么问题（为什么）

上一课 `Core.Build.cs` 里写了 `public class Core : ModuleRules`——那个 `ModuleRules` 就是本文件。它是**所有 `*.Build.cs` 的基类**：你写的每个模块单子，本质都是继承它、在构造函数里往它的清单里加名字。

**说个场景**：`ModuleRules` 是一张**空白施工单模板**，印好了所有栏目（依赖模块 / 宏定义 / 库 / 框架 / PCH / 优化等级…）。`Core.Build.cs` 是填好的那一份——它只填了 Core 需要的栏目，其余留空。UBT 拿到填好的单子，才知道这个模块怎么编。

> 🔑 **一句话**：`ModuleRules` = **模块施工单的模板类**，2119 行里 90% 是「栏目定义」（属性）+ 一小撮「填表助手」（`AddEngineThirdPartyPrivateStaticDependencies` 这类方法）。

---

## 二、内部结构全景

```
namespace UnrealBuildTool
├─ enum WarningLevel            18~39    警告级别：Default / Off / Warning / Error
├─ enum VerseScope              44~65    Verse 代码可见性：PublicAPI / InternalAPI / PublicUser / InternalUser
├─ enum IWYUSupport             70~98    IWYU 支持程度：None / KeepAsIs / KeepAsIsForNow / KeepPublicAsIsForNow / Full
└─ class ModuleRules           103~2118  ★ 模块施工单模板
     ├─ 嵌套枚举（7 个）
     │    ├─ ModuleType              108~119   CPlusPlus / External
     │    ├─ PackageOverrideType     126~157   None / EditorOnly / EngineDeveloper / GameDeveloper / EngineUncookedOnly / GameUncookedOnly
     │    ├─ CodeOptimization        162~188   Never / InNonDebugBuilds / InShippingBuildsOnly / Always / Default
     │    ├─ PCHUsageMode            193~219   Default / NoPCHs / NoSharedPCHs / UseSharedPCHs / UseExplicitOrSharedPCHs
     │    ├─ PrecompileTargetsType   224~250   None / Default / Game / Editor / Any
     │    ├─ SymbolVisibility        255~266   Default / VisibileForDll
     │    └─ UnityFileType           272~308   None / Cpp / C / CC / CXX / M / MM（[Flags]）
     ├─ 嵌套辅助类（6 个）
     │    ├─ RuntimeDependency       314~354   Path / SourcePath / Type + 2 个构造函数
     │    ├─ RuntimeDependencyList   360~408   内部 List + 3 个 Add 重载
     │    ├─ ReceiptPropertyList     414~452   Add(name,value) / RemoveAll
     │    ├─ Framework               457~531   Name / Path / CopyBundledAssets / Mode + FrameworkMode 枚举
     │    ├─ BundleResource          536~565   ResourcePath / BundleContentsSubdir / bShouldLog
     │    └─ TypeLibrary             570~606   FileName / Attributes / Header / Include
     ├─ 身份与路径（611~667）
     │    ├─ Name / File / Directory / PlatformDirectory / bIsPlatformExtension
     │    ├─ DirectoriesForModuleSubClasses / AdditionalModuleDirectories
     │    ├─ RulesAssembly / Plugin / IsPlugin / Context
     │    └─ Target（ReadOnlyTargetRules）★ 所有平台判断的源头
     ├─ 编译配置（672~1066）
     │    ├─ Type / Logger / OverridePackageType / HasPackageOverride / BinariesSubFolder
     │    ├─ OptimizeCode / OptimizationLevel / FPSemantics
     │    ├─ ForceIncludeFiles / PrivatePCHHeaderFile / SharedPCHHeaderFile / ShortName / PCHUsage
     │    ├─ bTreatAsEngineModule / SDKVersionRelevantPlatforms
     │    ├─ bValidateFormatStrings / bValidateInternalApi / bValidateExperimentalApi
     │    ├─ DefaultBuildSettings / IncludeOrderVersion
     │    ├─ bUseRTTI / bVcRemoveUnreferencedComdat / bCodeCoverage / bUseAVX(废弃) / MinCpuArchX64
     │    ├─ bEnableBufferSecurityChecks / bEnableExceptions / bEnableObjCExceptions / bEnableObjCAutomaticReferenceCounting
     │    └─ CppCompileWarningSettings + 8 个已废弃的警告级别属性
     ├─ 静态分析（1059~1125）
     │    ├─ bDisableStaticAnalysis / bStaticAnalyzerExtensions
     │    └─ StaticAnalyzerRulesets / Checkers / DisabledCheckers / AdditionalCheckers / PVSDisabledErrors
     ├─ Unity 合批（1127~1167）
     │    ├─ bUseUnity / bMergeUnityFiles / UnityFileTypes
     │    ├─ MinSourceFilesForUnityBuildOverride / MinFilesUsingPrecompiledHeaderOverride
     │    └─ NumIncludedBytesPerUnityCPPOverride ★ Core.Build.cs 第 11 行用的就是它
     ├─ 依赖与链接清单（1268~1438）★ 最常用
     │    ├─ PublicIncludePathModuleNames / PublicDependencyModuleNames
     │    ├─ PrivateIncludePathModuleNames / PrivateDependencyModuleNames
     │    ├─ CircularlyReferencedDependentModules
     │    ├─ PublicSystemIncludePaths / PublicIncludePaths / InternalIncludePaths / PrivateIncludePaths
     │    ├─ PublicSystemLibraryPaths / PrivateRuntimeLibraryPaths / PublicRuntimeLibraryPaths
     │    ├─ PublicAdditionalLibraries / PublicDebugVisualizerPaths / PublicPreBuildLibraries
     │    ├─ PublicSystemLibraries / PublicFrameworks / PublicWeakFrameworks / PublicAdditionalFrameworks
     │    ├─ AdditionalBundleResources / TypeLibraries / PublicDelayLoadDLLs
     │    ├─ PrivateDefinitions / PublicDefinitions
     │    ├─ DynamicallyLoadedModuleNames / RuntimeDependencies / AdditionalPropertiesForReceipt
     │    └─ PrecompileForTargets / ExternalDependencies / SubclassRules / bRequiresImplementModule
     ├─ 杂项开关（1466~1576）
     │    ├─ ExtraRootPath / VersePath / VerseScope
     │    ├─ bLegacyPublicIncludePaths / bLegacyParentIncludePaths / bValidateCircularDependencies
     │    ├─ CppStandard / CStandard / GenerateHeaderFuncs / ModuleSymbolVisibility
     │    ├─ AutoSdkDirectory / EngineDirectory / PluginDirectory / ModuleDirectory
     │    ├─ PlatformModuleDirectory / PlatformSubdirectoryName / TestsDirectory
     │    └─ SwiftInteropHeader / FPassPlugins
     └─ 方法（1584~2117）
          ├─ 构造函数 ModuleRules(ReadOnlyTargetRules)      1584
          ├─ AddEngineThirdPartyPrivateStaticDependencies   1599  ★ Core.Build.cs 大量使用
          ├─ AddEngineThirdPartyPrivateDynamicDependencies  1615
          ├─ SetupVerse ×2 / EnableMeshEditorSupport        1627~1655
          ├─ SetupGameplayDebuggerSupport / SetupIrisSupport 1660~1733
          ├─ IsChaosVisualDebuggerSupported / SetupModuleChaosVisualDebuggerSupport 1738~1759
          ├─ SetupModulePhysicsSupport                      1764
          ├─ IsValidForTarget ×2 / IsPlatformAvailable      1793~1868
          ├─ GetModulesInGroup / PrepareModuleForTests      1876~1929
          ├─ GetModuleDirectoryForSubClass / GetAllModuleDirectories / ConditionalAddModuleDirectory 1943~1970
          ├─ IsVcPackageSupported / GetVcPackageRoot / AddVcPackage 1976~2084
          └─ ChangeDefinition / ReplaceModule               2094~2117
```

---

## 三、依赖分析

| 项 | 依赖 | 能否独立编译 | 需挂起？ |
|------|------|-------------|---------|
| `ModuleRules.cs` | `EpicGames.Core` / `Microsoft.Extensions.Logging` / `UnrealBuildBase` + UBT 内部类型（`ReadOnlyTargetRules`、`RulesAssembly`、`CppCompileWarnings`、`StagedFileType`…） | ✅ 能（UBT 工程内编译） | 否 |

**关键外部类型**（都在 UBT 工程内，不是第三方）：

| 类型 | 用在哪 | 说明 |
|---|---|---|
| `ReadOnlyTargetRules` | `Target` 属性（667 行） | 只读的目标规则，`Core.Build.cs` 构造函数参数就是它 |
| `CppCompileWarnings` | `CppCompileWarningSettings`（982 行） | 警告级别设置对象，`Core.Build.cs` 第 302 行用的就是它 |
| `RulesAssembly` | `RulesAssembly` 属性（647 行） | 规则程序集，负责找模块 |
| `StagedFileType` | `RuntimeDependency.Type`（329 行） | 打包方式枚举 |
| `UnrealTargetPlatform` / `UnrealPlatformGroup` | 各方法 | 平台判断 |

---

## 四、怎么教（推进节奏）

**先建本总 md（已建）→ 停，等确认 → 教完整 2119 行代码（`01_ModuleRules.cs_完整代码.md`，注释已中文化）**。

> ✅ **2026-11-16 已建**：`01_ModuleRules.cs_完整代码.md`（2223 行 / 82.6KB，含完整 2119 行代码 + 10 段逐段拆解 + 差异表 + 待确认 + 源码依据）。代码块已脚本比对：**925 行非注释非空代码与引擎原文逐字一致，零差异**（唯一差异是第 626 行行尾注释中文化）。

> ⚠️ 这份是**答疑文件**，不是主线。主线仍是 `Core.Build.cs` → `Core_APL.xml` → `Public\` 7 个散头。

---

**下一步**：看完 `01_ModuleRules.cs_完整代码.md` 说「下一步」，回主线继续 `Core.Build.cs` 验收 → `Core_APL.xml`。