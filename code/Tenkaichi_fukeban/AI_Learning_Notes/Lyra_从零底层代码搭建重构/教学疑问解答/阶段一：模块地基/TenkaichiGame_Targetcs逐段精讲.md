# TenkaichiGame.Target.cs 逐段精讲（每个东西都在干嘛）

> **所属**：阶段一（L1 模块地基）第 1 步。答疑记录。
> **对应 Lyra**：`Source/LyraGame.Target.cs`（283 行）。
> **阅读方式**：按「成员 → 成员内部每个字段/语句」逐层拆，一条一条说清楚它在干嘛。
> **配合**：[TenkaichiGame_Targetcs讲解.md](./TenkaichiGame_Targetcs讲解.md)（整体概览），本文是它的逐行细化。

---

## 零、文件全景（先有地图）

这个文件从外到里一共 **6 个成员**：

| 成员 | 行号 | 一句话干嘛 |
|------|------|-----------|
| `using ...`（7 个） | 3~9 | 引用的命名空间/程序集 |
| `TenkaichiGameTarget` 构造函数 | 13~20 | 定义「这是 Game 目标」 |
| `bHasWarnedAboutShared` 字段 | 22 | 一个「只警告一次」的开关 |
| `ApplySharedTenkaichiTargetSettings` | 24~95 | 所有 Target 共享的编译设置 |
| `ShouldEnableAllGameFeaturePlugins` | 97~116 | 要不要全编 GameFeature 插件 |
| `AllPluginRootJsonObjectsByName` 字段 | 118 | 缓存已读的 `.uplugin` JSON |
| `ConfigureGameFeaturePlugins` | 124~282 | 逐个决定 GameFeature 插件启用/禁用 |

---

## 一、`using` 区（3~9 行）

```csharp
using UnrealBuildTool;              // UE 编译工具的 TargetRules 等核心类
using System;                       // Exception、StringComparison、Environment 等
using System.IO;                    // Path.Combine 等文件路径操作
using EpicGames.Core;               // JsonObject、FileReference、DirectoryReference 等
using System.Collections.Generic;   // List<>、Dictionary<> 集合
using UnrealBuildBase;              // Unreal.GetExtensionDirs 等引擎基础工具
using Microsoft.Extensions.Logging; // ILogger 日志接口
```

| 行 | 干嘛的 |
|----|--------|
| 3 `UnrealBuildTool` | 提供 `TargetRules` 基类、`TargetType`、`TargetBuildEnvironment` 等编译期核心类型 |
| 4 `System` | 提供 `Exception`（异常）、`StringComparison`（字符串比较）、`Environment`（环境变量） |
| 5 `System.IO` | 提供 `Path.Combine`（拼路径） |
| 6 `EpicGames.Core` | 提供 `JsonObject`（读 `.uplugin` 的 JSON）、`FileReference`/`DirectoryReference`（文件/目录引用） |
| 7 `System.Collections.Generic` | 提供 `List<>`、`Dictionary<>` 容器 |
| 8 `UnrealBuildBase` | 提供 `Unreal.GetExtensionDirs`（找扩展目录） |
| 9 `Microsoft.Extensions.Logging` | 提供 `ILogger`（编译日志输出） |

> 一句话：这 7 个 `using` 是后面 280 行用到的所有类型/工具的来源。

---

## 二、构造函数 `TenkaichiGameTarget`（13~20 行）

```csharp
public class TenkaichiGameTarget : TargetRules
{
	public TenkaichiGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.AddRange(new string[] { "TenkaichiGame" });
		TenkaichiGameTarget.ApplySharedTenkaichiTargetSettings(this);
	}
```

| 语句 | 干嘛的 |
|------|--------|
| `class TenkaichiGameTarget : TargetRules` | 这个类**继承 `TargetRules`**，成为一个「编译目标定义」。UE 编译时会自动找 `*.Target.cs` 里继承 `TargetRules` 的类 |
| `TenkaichiGameTarget(TargetInfo Target) : base(Target)` | 构造函数，`base(Target)` 先把 `TargetInfo`（目标信息）交给父类 `TargetRules` 初始化 |
| `Type = TargetType.Game` | **声明这个目标是「游戏」类型**（单机本地运行，逻辑+渲染都在本地） |
| `ExtraModuleNames.AddRange(...)` | 告诉编译器：编译这个目标时，把 `TenkaichiGame` 模块也编进去 |
| `ApplySharedTenkaichiTargetSettings(this)` | **调用共享设置方法**，把一堆通用编译开关套到自己身上（`this` = 当前这个目标） |

> 为什么只有 `Type` 和 `ExtraModuleNames` 两行自己的设置？因为其余几百行设置全在 `ApplySharedTenkaichiTargetSettings` 里，其他 Target（Editor/Client/Server）也调它，**避免每个 Target 复制粘贴 280 行**。

---

## 三、字段 `bHasWarnedAboutShared`（22 行）

```csharp
private static bool bHasWarnedAboutShared = false;
```

| 要素 | 干嘛的 |
|------|--------|
| `private static` | 类私有、且**静态**（所有 Target 实例共享同一个） |
| `bool bHasWarnedAboutShared` | 一个「**是否已经警告过**」的标记 |

**用途**：在共享/安装引擎分支（`else` 分支，见第四节），有一条 `LogWarning` 警告。如果 10 个 Target 每个都警告一次，日志会被刷屏。所以用这个静态开关保证**只警告第一次**，后面跳过。

---

## 四、`ApplySharedTenkaichiTargetSettings`（24~95 行）

这是**核心共享设置方法**。所有 Target 都调它，把自己套上同一套编译开关。

### 4.1 开头三行（26~29 行）

```csharp
ILogger Logger = Target.Logger;
Target.DefaultBuildSettings = BuildSettingsVersion.V5;
Target.IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
```

| 语句 | 干嘛的 |
|------|--------|
| `ILogger Logger = Target.Logger` | 拿到目标自带的日志对象，后面用 `Logger.LogWarning` 等输出 |
| `DefaultBuildSettings = V5` | 用 **BuildSettingsVersion.V5**（UE5 的构建设置版本） |
| `IncludeOrderVersion = Latest` | 用**最新**的头文件包含顺序规则 |

### 4.2 三个布尔量（31~33 行）

```csharp
bool bIsTest = Target.Configuration == UnrealTargetConfiguration.Test;
bool bIsShipping = Target.Configuration == UnrealTargetConfiguration.Shipping;
bool bIsDedicatedServer = Target.Type == TargetType.Server;
```

| 变量 | 干嘛的（判断什么） |
|------|------------------|
| `bIsTest` | 当前是不是 **Test**（测试）配置 |
| `bIsShipping` | 当前是不是 **Shipping**（发布）配置 |
| `bIsDedicatedServer` | 当前是不是 **Server**（专用服务器）目标 |

> 后面一堆 `if (bIsShipping)` / `if (bIsShipping || bIsTest)` 都靠这三个量做条件。

### 4.3 大分支：`Unique` vs `非 Unique`（34~94 行）

```csharp
if (Target.BuildEnvironment == TargetBuildEnvironment.Unique)
{ ... 完整精细设置 ... }
else
{ ... 受限设置 ... }
```

| 分支 | 什么时候走 | 能干嘛 |
|------|-----------|--------|
| `Unique` | **源码版引擎**（每个目标独立编译） | 能随便启停插件、改任何编译选项 |
| `else`（共享/安装） | **安装版引擎**（复用预编译二进制） | 受限制，不能随便启停插件 |

**为什么分这个支**：源码版引擎你能完全控制编译；但如果你用的是「通过 Epic 启动器安装」的引擎，二进制已经预编译好了，改了插件/选项会导致要重编引擎，所以只能走受限分支。

#### 4.3.1 `Unique` 分支内部（35~72 行）

```csharp
Target.CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Error;
Target.bUseLoggingInShipping = true;
Target.bTrackRHIResourceInfoForTest = true;
```

| 语句 | 干嘛的 |
|------|--------|
| `ShadowVariableWarningLevel = Error` | **变量遮蔽（shadowing）警告升级为错误**——同名变量遮蔽会直接编译失败，逼你改掉隐患 |
| `bUseLoggingInShipping = true` | **发布版也保留日志**（方便线上排查问题） |
| `bTrackRHIResourceInfoForTest = true` | 测试配置下**追踪 RHI 资源信息**（排查图形资源泄漏用） |

接着是几个条件块：

```csharp
if (bIsShipping && !bIsDedicatedServer)
{
	Target.bDisableUnverifiedCertificates = true;   // 强制校验 HTTPS 证书
	// 下面三组被注释掉的 GlobalDefinitions，是可选的「命令行参数锁定」开关
}
```

| 语句 | 干嘛的 |
|------|--------|
| `bDisableUnverifiedCertificates = true` | 发布版（且非服务器）**强制校验 HTTPS 证书**，不信任自签证书（安全） |
| 注释掉的 `UE_COMMAND_LINE_USES_ALLOW_LIST=1` | 可选：**只允许白名单里的命令行参数**被解析 |
| 注释掉的 `FILTER_COMMANDLINE_LOGGING` | 可选：**过滤掉敏感命令行参数**，不让它进日志 |

```csharp
if (bIsShipping || bIsTest)
{
	Target.bAllowGeneratedIniWhenCooked = false;
	Target.bAllowNonUFSIniWhenCooked = false;
}
```

| 语句 | 干嘛的 |
|------|--------|
| `bAllowGeneratedIniWhenCooked = false` | 发布/测试时**禁止读烘焙时生成的 ini**（锁定配置，防止运行时被改） |
| `bAllowNonUFSIniWhenCooked = false` | 发布/测试时**禁止读非 UFS（非 Unreal 文件系统）的 ini** |

```csharp
if (Target.Type != TargetType.Editor)
{
	Target.DisablePlugins.Add("OpenImageDenoise");
	Target.GlobalDefinitions.Add("UE_ASSETREGISTRY_INDIRECT_ASSETDATA_POINTERS=1");
}
```

| 语句 | 干嘛的 |
|------|--------|
| `DisablePlugins.Add("OpenImageDenoise")` | 非编辑器目标**禁掉光追降噪插件**（DLL 很大，运行时用不到） |
| `UE_ASSETREGISTRY_INDIRECT_ASSETDATA_POINTERS=1` | **降低 AssetRegistry 常驻内存**（用间接指针存资产数据，代价是查询更耗 CPU） |

最后一行：

```csharp
TenkaichiGameTarget.ConfigureGameFeaturePlugins(Target);
```

→ 在 Unique 分支里**调用 GameFeature 插件配置方法**（见第六节）。

#### 4.3.2 `else`（共享/安装）分支内部（74~94 行）

```csharp
// 这里面的改动不能影响 PCH 生成，否则要设成 Unique
if (Target.Type == TargetType.Editor)
{
	TenkaichiGameTarget.ConfigureGameFeaturePlugins(Target);  // 编辑器还能配插件
}
else
{
	if (!bHasWarnedAboutShared)
	{
		bHasWarnedAboutShared = true;
		Logger.LogWarning("... disabled when packaging from an installed version ...");
	}
}
```

| 语句 | 干嘛的 |
|------|--------|
| 注释 `不能影响 PCH 生成` | 提醒：共享构建下改动不能影响预编译头（PCH），否则要切回 Unique |
| `if (Editor)` → 调 `ConfigureGameFeaturePlugins` | **编辑器**在共享构建下仍能配置插件 |
| `else` → 只警告一次 | 非编辑器 + 共享构建，**无法启停插件**，只打一条警告（用 `bHasWarnedAboutShared` 保证只打一次） |

---

## 五、`ShouldEnableAllGameFeaturePlugins`（97~116 行）

```csharp
static public bool ShouldEnableAllGameFeaturePlugins(TargetRules Target)
{
	if (Target.Type == TargetType.Editor) { /* return true; */ }
	bool bIsBuildMachine = (Environment.GetEnvironmentVariable("IsBuildMachine") == "1");
	if (bIsBuildMachine) { /* return true; */ }
	return false;
}
```

| 部分 | 干嘛的 |
|------|--------|
| 方法作用 | 返回「**要不要把所有 GameFeature 插件都编译进来**」 |
| `if (Editor) { return true; }`（注释掉） | 可选：**编辑器构建编译所有 GameFeature 插件**（但不一定全加载），方便编辑器里启用插件而不用重编代码 |
| `bIsBuildMachine` | 读环境变量 `IsBuildMachine`，判断是不是**构建机** |
| `if (build machine) { return true; }`（注释掉） | 可选：给构建机启用所有插件 |
| `return false`（默认） | **默认不全部启用**，走编辑器插件浏览器里手动勾选的规则 |

> 为什么默认 `false` 很重要：对于「通过启动器安装的引擎」，这段代码可能根本不会执行，所以默认规则得安全。

---

## 六、`AllPluginRootJsonObjectsByName`（118 行）

```csharp
private static Dictionary<string, JsonObject> AllPluginRootJsonObjectsByName = new Dictionary<string, JsonObject>();
```

| 要素 | 干嘛的 |
|------|--------|
| `Dictionary<string, JsonObject>` | 键=插件名，值=该插件的 `.uplugin` 的 JSON 对象 |
| 用途 | **缓存**：同一个插件的 `.uplugin` 文件只读一次，下次直接用缓存的 JSON（避免重复读磁盘） |

---

## 七、`ConfigureGameFeaturePlugins`（124~282 行，最长的部分）

**作用**：扫描 `Plugins/GameFeatures/` 下所有 GameFeature 插件，逐个读它们的 `.uplugin` 描述，按规则决定「启用/禁用/忽略」，最后写进 `Target.EnablePlugins` / `Target.DisablePlugins`。

### 7.1 开头（126~138 行）

```csharp
ILogger Logger = Target.Logger;
Log.TraceInformationOnce("Compiling GameFeaturePlugins in branch {0}", Target.Version.BranchName);
bool bBuildAllGameFeaturePlugins = ShouldEnableAllGameFeaturePlugins(Target);
List<FileReference> CombinedPluginList = new List<FileReference>();
List<DirectoryReference> GameFeaturePluginRoots = Unreal.GetExtensionDirs(Target.ProjectFile.Directory, Path.Combine("Plugins", "GameFeatures"));
foreach (DirectoryReference SearchDir in GameFeaturePluginRoots)
{
	CombinedPluginList.AddRange(PluginsBase.EnumeratePlugins(SearchDir));
}
```

| 语句 | 干嘛的 |
|------|--------|
| `Log.TraceInformationOnce(...)` | 打印一条（只一次）「正在编译 GameFeature 插件，分支名 XXX」 |
| `bBuildAllGameFeaturePlugins` | 调用第五节那个方法，拿到「要不要全编」 |
| `CombinedPluginList` | 空列表，准备装所有找到的 `.uplugin` 文件 |
| `Unreal.GetExtensionDirs(...)` | 找到项目 `Plugins/GameFeatures/` 这个目录（含引擎扩展目录） |
| `foreach` + `EnumeratePlugins` | 把该目录下所有 `.uplugin` 文件**枚举**进 `CombinedPluginList` |

### 7.2 主循环：逐个插件判断（140~277 行）

```csharp
if (CombinedPluginList.Count > 0)
{
	Dictionary<string, List<string>> AllPluginReferencesByName = ...;
	foreach (FileReference PluginFile in CombinedPluginList)
	{
		if (PluginFile != null && FileReference.Exists(PluginFile))
		{
			bool bEnabled = false;      // 是否启用
			bool bForceDisabled = false; // 是否强制禁用
			try { ... 读 JSON、按字段判断 ... }
			catch (Exception ParseException) { ... 解析失败就禁用 ... }
			// 禁用优先，最后写进 EnablePlugins / DisablePlugins
		}
	}
}
```

| 变量 | 干嘛的 |
|------|--------|
| `AllPluginReferencesByName` | 记录每个插件**引用了哪些别的插件**（供后续校验） |
| `bEnabled` | 这个插件**最终是否启用** |
| `bForceDisabled` | 这个插件**是否被强制禁用**（优先级高于启用） |

#### 7.2.1 读 JSON（152~157 行）

```csharp
JsonObject RawObject;
if (!AllPluginRootJsonObjectsByName.TryGetValue(PluginFile.GetFileNameWithoutExtension(), out RawObject))
{
	RawObject = JsonObject.Read(PluginFile);
	AllPluginRootJsonObjectsByName.Add(PluginFile.GetFileNameWithoutExtension(), RawObject);
}
```

| 语句 | 干嘛的 |
|------|--------|
| `TryGetValue` | 先查缓存（第六节那个字典），**读过了就不再读磁盘** |
| `JsonObject.Read(PluginFile)` | 没缓存就**读 `.uplugin` 文件，解析成 JSON 对象** |
| `Add(...)` | 把解析结果**存进缓存** |

#### 7.2.2 检查各字段（158~227 行）

| 字段 | 干嘛的 | 判断结果 |
|------|--------|---------|
| `EnabledByDefault`（158~166 行） | 是否默认启用 | GameFeature 插件应设 `false`（注释里说明了原因，警告目前被注释掉） |
| `ExplicitlyLoaded`（168~174 行） | 是否显式加载 | 必须是 `true`，否则 `LogWarning` 警告（GameFeature 要求启动后手动加载） |
| 注释掉的「项目专属字段」（176~181 行） | 示例：可按发布版本决定启用 | 可选扩展点 |
| `bBuildAllGameFeaturePlugins`（183~187 行） | 全编模式 | 若为真，`bEnabled = true` |
| `EditorOnly`（189~202 行） | 是否仅编辑器可用 | 非编辑器目标 + 非全编 → `bForceDisabled = true` |
| `RestrictToBranch`（204~218 行） | 限定分支 | 不匹配当前分支 → `bForceDisabled = true` |
| `NeverBuild`（220~227 行） | 永不编译 | 为真 → `bForceDisabled = true` |
| `Plugins` 引用（229~250 行） | 该插件引用了哪些插件 | 记录进 `AllPluginReferencesByName` 供校验 |

> 核心逻辑：**「禁用」优先级高于「启用」**——只要命中 `EditorOnly`/`RestrictToBranch`/`NeverBuild` 任一「强制禁用」条件，最终就是禁用。

#### 7.2.3 异常处理（252~256 行）

```csharp
catch (Exception ParseException)
{
	Logger.LogWarning("Failed to parse GameFeaturePlugin file {Name}, disabling...", ...);
	bForceDisabled = true;
}
```

| 语句 | 干嘛的 |
|------|--------|
| `catch` | 读/解析 `.uplugin` 出错（JSON 坏了等）就捕获 |
| `bForceDisabled = true` | **解析失败一律禁用**（宁可不用，也别用坏插件） |

#### 7.2.4 收尾：写入结果（258~277 行）

```csharp
if (bForceDisabled) { bEnabled = false; }              // 禁用优先
Logger.LogDebug("... decided to {Action} feature {Name}", ...);  // 打印最终决定
if (bEnabled) { Target.EnablePlugins.Add(name); }        // 启用
else if (bForceDisabled) { Target.DisablePlugins.Add(name); }  // 禁用
```

| 语句 | 干嘛的 |
|------|--------|
| `if (bForceDisabled) bEnabled = false` | **禁用优先级高于启用**（强制禁用覆盖启用） |
| `LogDebug(...)` | 打印每个插件的最终决定（enable/disable/ignore） |
| `Target.EnablePlugins.Add` | 最终**启用**这个插件 |
| `Target.DisablePlugins.Add` | 最终**禁用**这个插件 |

### 7.3 结尾注释（279~280 行）

```csharp
// 如果你用了类似发布版本的东西，可以考虑做引用校验，
// 确保较早发布版本的插件不依赖较晚发布版本的内容
```

→ 一个「可选的后续扩展点」提示，本步不实现。

---

## 八、一句话总结

这个文件干两件事：

1. **定义自己**：`Type = Game` + 编 `TenkaichiGame` 模块（构造函数）。
2. **当公共工具箱**：
   - `ApplySharedTenkaichiTargetSettings` → 一套共享编译开关（给所有 Target 复用）；
   - `ConfigureGameFeaturePlugins` → 扫描 GameFeature 插件目录，逐个决定启用/禁用。

所以它是 10 个 Target 里的**基类 + 公共配置中心**，必须最先补、且一字不差。