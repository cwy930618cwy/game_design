# 06 — PCH 怎么打开？具体代码在哪？（源码级追踪）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\` 实查
> 用户问题：「那怎么打开 PCH？具体代码在哪里？」
> **✅ 本节可独立编译**——纯 UBT 源码追踪。

---

## 一、先回答「怎么打开」

**PCH 默认就是开的**，不需要你手动打开。三个开关全在 `TargetRules.cs` 里，默认值都是 `true`：

| 开关 | 出处 | 默认值 | 含义 |
|---|---|---|---|
| `bUsePCHFiles` | `TargetRules.cs:2109` | **`true`** | 是否用 PCH 文件 |
| `bUseSharedPCHs` | `TargetRules.cs:2350` | **`true`** | 是否用共享 PCH |
| `bChainPCHs` | `TargetRules.cs:2140` | **`true`** | 是否链式 PCH |

**要关掉**（不推荐）：

```csharp
// 在 *.Target.cs 里
bUsePCHFiles = false;      // 完全不用 PCH
bUseSharedPCHs = false;    // 不用共享 PCH（但可以用私有 PCH）
```

**要指定用哪个 PCH 头**（这才是 `Core.Build.cs` 干的事）：

```csharp
// Core.Build.cs 第 13、15 行
PrivatePCHHeaderFile = "Private/CorePrivatePCH.h";   // 私有 PCH
SharedPCHHeaderFile = "Public/CoreSharedPCH.h";      // 共享 PCH
```

> 🔑 **一句话**：**PCH 默认开；`Core.Build.cs` 只是「告诉 UBT 用哪个头当 PCH」，不是「打开 PCH」。**

---

## 二、完整调用链（从 `Core.Build.cs` 到编译器命令行）

```
① Core.Build.cs:15
   SharedPCHHeaderFile = "Public/CoreSharedPCH.h"
        ↓
② UEBuildModuleCPP.cs:1137~1140
   if (Rules.SharedPCHHeaderFile != null)
       SharedPchFileItem = FileItem.GetItemByFileReference(...)
        ↓
③ UEBuildModuleCPP.cs:1366~1382
   CreateSharedPCHTemplate(...)  ← 建「PCH 模板」
        ↓
④ UEBuildModuleCPP.cs:1951~2020
   SetupPrecompiledHeaders(...)  ← 决定这个模块用哪个 PCH
        ↓
⑤ UEBuildModuleCPP.cs:1449~1561
   FindOrCreateSharedPCH(...)    ← 找现成的，没有就建
        ↓
⑥ UEBuildModuleCPP.cs:2069~2090
   CreatePCHWrapperFile(...)     ← 生成 wrapper 头（PCH.xxx.h）
        ↓
⑦ UEBuildModuleCPP.cs:1550
   ToolChain.CompileAllCPPFiles(...)  ← 编译 wrapper → 生成 .pch
        ↓
⑧ VCToolChain.cs:301~302
   /Yc"CoreSharedPCH.h" /Fp"xxx.pch"   ← MSVC 创建 PCH
        ↓
⑨ VCToolChain.cs:318~319
   /Yu"CoreSharedPCH.h" /Fp"xxx.pch"   ← MSVC 使用 PCH
        ↓
⑩ VCToolChain.cs:285
   /FI"CoreSharedPCH.h"                ← 强制包含
```

---

## 三、逐段看源码

### 3.1 第 ② 步：登记 PCH 头（`UEBuildModuleCPP.cs:1131~1140`）

```csharp
// UEBuildModuleCPP.cs:1131~1140
if (Rules.PrivatePCHHeaderFile != null)
{
	PrivatePchFileItem = FileItem.GetItemByFileReference(FileReference.Combine(ModuleDirectory, Rules.PrivatePCHHeaderFile));
}

if (Rules.SharedPCHHeaderFile != null)
{
	SharedPchFileItem = FileItem.GetItemByFileReference(FileReference.Combine(ModuleDirectory, Rules.SharedPCHHeaderFile));
}
```

| 行 | 干什么 |
|---|---|
| 1131 | 如果设了 `PrivatePCHHeaderFile` |
| 1133 | 把 `Private/CorePrivatePCH.h` 转成 `FileItem`（UBT 的文件对象） |
| 1137 | 如果设了 `SharedPCHHeaderFile` |
| 1139 | 把 `Public/CoreSharedPCH.h` 转成 `FileItem` |

> 注释原文（译）：*If the module has a private PCH, add it to the list of files to compile* —— 如果模块有私有 PCH，把它加进要编译的文件列表。

### 3.2 第 ③ 步：建 PCH 模板（`UEBuildModuleCPP.cs:1366~1382`）

```csharp
// UEBuildModuleCPP.cs:1366~1382
public PrecompiledHeaderTemplate CreateSharedPCHTemplate(UEBuildTarget Target, CppCompileEnvironment BaseCompileEnvironment, ILogger Logger)
{
	CppCompileEnvironment CompileEnvironment = CreateSharedPCHCompileEnvironment(Target, BaseCompileEnvironment);
	FileItem HeaderFile = FileItem.GetItemByFileReference(FileReference.Combine(ModuleDirectory, Rules.SharedPCHHeaderFile!));

	DirectoryReference PrecompiledHeaderDir;
	if (Rules.bUsePrecompiled)
	{
		PrecompiledHeaderDir = DirectoryReference.Combine(Target.ProjectIntermediateDirectory, Name);
	}
	else
	{
		PrecompiledHeaderDir = IntermediateDirectory;
	}

	return new PrecompiledHeaderTemplate(this, CompileEnvironment, HeaderFile, PrecompiledHeaderDir, GetSharedPCHModuleDependencies(CompileEnvironment, HeaderFile, Logger));
}
```

| 行 | 干什么 |
|---|---|
| 1368 | 建「共享 PCH 专用编译环境」（和普通模块不同） |
| 1369 | 把 `Public/CoreSharedPCH.h` 转成 `FileItem` |
| 1371~1379 | 决定 `.pch` 输出到哪个目录（`Intermediate/` 下） |
| 1381 | 返回 `PrecompiledHeaderTemplate`（模板，还没真正编译） |

> **模板 vs 实例**：模板是「配方」，实例是「做出来的菜」。同一个模板可以生成多个实例（不同配置）。

### 3.3 第 ④ 步：决定用哪个 PCH（`UEBuildModuleCPP.cs:1951~2020`）

```csharp
// UEBuildModuleCPP.cs:1951~1967
CppCompileEnvironment SetupPrecompiledHeaders(ReadOnlyTargetRules Target, UEToolChain? ToolChain, CppCompileEnvironment CompileEnvironment, List<FileItem> LinkInputFiles, IActionGraphBuilder Graph, ILogger Logger)
{
	if (Target.bUsePCHFiles && Rules.PCHUsage != ModuleRules.PCHUsageMode.NoPCHs)
	{
		// If this module doesn't need a shared PCH, configure that
		if (Rules.PrivatePCHHeaderFile != null && (Rules.PCHUsage == ModuleRules.PCHUsageMode.NoSharedPCHs || Rules.PCHUsage == ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs))
		{
			PrecompiledHeaderInstance Instance = CreatePrivatePCH(ToolChain, FileItem.GetItemByFileReference(FileReference.Combine(ModuleDirectory, Rules.PrivatePCHHeaderFile)), CompileEnvironment, Graph);

			CompileEnvironment = new CppCompileEnvironment(CompileEnvironment);
			CompileEnvironment.Definitions.Clear();
			CompileEnvironment.PrecompiledHeaderAction = PrecompiledHeaderAction.Include;
			CompileEnvironment.PrecompiledHeaderIncludeFilename = Instance.HeaderFile.Location;
			CompileEnvironment.PCHInstance = Instance;

			LinkInputFiles.AddRange(Instance.Output.ObjectFiles);
		}
```

| 行 | 干什么 |
|---|---|
| 1953 | **总开关**：`bUsePCHFiles` 为真 且 `PCHUsage != NoPCHs` |
| 1956 | 如果设了私有 PCH 且 `PCHUsage` 是 `NoSharedPCHs` 或 `UseExplicitOrSharedPCHs` |
| 1958 | **创建私有 PCH**（`CreatePrivatePCH`） |
| 1962 | 设置 `PrecompiledHeaderAction = Include`（**使用** PCH） |
| 1963 | 设置 PCH 头文件名 |
| 1966 | 把 PCH 的 `.obj` 加进链接列表 |

```csharp
// UEBuildModuleCPP.cs:1969~1978
		// Try to find a suitable shared PCH for this module
		if (CompileEnvironment.bHasPrecompiledHeader == false && CompileEnvironment.SharedPCHs.Count > 0 && !CompileEnvironment.bIsBuildingLibrary && Rules.PCHUsage != ModuleRules.PCHUsageMode.NoSharedPCHs)
		{
			// Find the first shared PCH module we can use that doesn't reference this module
			HashSet<UEBuildModule> AllDependencies = GetAllDependencyModulesForPCH(true, false);
			PrecompiledHeaderTemplate? Template = CompileEnvironment.SharedPCHs.FirstOrDefault(x => !x.ModuleDependencies.Contains(this) && AllDependencies.Contains(x.Module) && x.IsValidFor(CompileEnvironment));

			if (Template != null)
			{
				PrecompiledHeaderInstance Instance = FindOrCreateSharedPCH(ToolChain, Template, CompileEnvironment, Graph);
```

| 行 | 干什么 |
|---|---|
| 1970 | 如果**还没有** PCH（`bHasPrecompiledHeader == false`）且**有共享 PCH 可用**且**不是建库**且**没禁用共享 PCH** |
| 1973 | 拿到本模块的所有依赖模块 |
| 1974 | **找第一个可用的共享 PCH 模板**（条件：不引用本模块 + 是本模块的依赖 + 对本环境有效） |
| 1978 | **找现成的实例，没有就建**（`FindOrCreateSharedPCH`） |

> 🔑 **关键**：`Core.Build.cs` 两个都设了，但**私有 PCH 优先**（第 1956 行先判断）。因为 `CorePrivatePCH.h` 第 5 行 include 了 `CoreSharedPCH.h`，所以私有 PCH 实际上包含了共享 PCH 的全部内容。

### 3.4 第 ⑤ 步：找或建共享 PCH 实例（`UEBuildModuleCPP.cs:1449~1561`）

```csharp
// UEBuildModuleCPP.cs:1449~1452
public PrecompiledHeaderInstance FindOrCreateSharedPCH(UEToolChain? ToolChain, PrecompiledHeaderTemplate Template, CppCompileEnvironment ModuleCompileEnvironment, IActionGraphBuilder Graph)
{
	PrecompiledHeaderInstance? Instance = Template.Instances.Find(x => IsCompatibleForSharedPCH(x.CompileEnvironment, ModuleCompileEnvironment));
	if (Instance == null)
	{
```

| 行 | 干什么 |
|---|---|
| 1451 | **先找现成的**：在模板的实例列表里找「编译环境兼容」的那个 |
| 1452 | 如果没找到 → 新建 |

**兼容性判断**（`UEBuildModuleCPP.cs:1569~1600`）：

```csharp
internal static bool IsCompatibleForSharedPCH(CppCompileEnvironment ModuleCompileEnvironment, CppCompileEnvironment CompileEnvironment)
{
	if (ModuleCompileEnvironment.bTreatAsEngineModule != CompileEnvironment.bTreatAsEngineModule) return false;
	if (ModuleCompileEnvironment.bOptimizeCode != CompileEnvironment.bOptimizeCode) return false;
	if (ModuleCompileEnvironment.bUseRTTI != CompileEnvironment.bUseRTTI) return false;
	if (ModuleCompileEnvironment.bEnableExceptions != CompileEnvironment.bEnableExceptions) return false;
	if (ModuleCompileEnvironment.CppStandard != CompileEnvironment.CppStandard) return false;
	if (ModuleCompileEnvironment.IncludeOrderVersion != CompileEnvironment.IncludeOrderVersion) return false;
	...
}
```

**6 个兼容条件**（任一不同 → 不能共用，要新建一个 PCH 变体）：

| 条件 | 含义 |
|---|---|
| `bTreatAsEngineModule` | 是不是引擎模块 |
| `bOptimizeCode` | 是否优化 |
| `bUseRTTI` | 是否用 RTTI |
| `bEnableExceptions` | 是否启用异常 |
| `CppStandard` | C++ 标准（Cpp17 / Cpp20） |
| `IncludeOrderVersion` | include 顺序版本 |

> 所以一个 `CoreSharedPCH.h` 可能生成**多个 `.pch` 变体**（如「引擎模块 + 优化 + Cpp20」一个，「游戏模块 + 不优化 + Cpp20」另一个）。

### 3.5 第 ⑥ 步：生成 wrapper 头（`UEBuildModuleCPP.cs:2069~2090`）

```csharp
// UEBuildModuleCPP.cs:2069~2090
static FileItem CreatePCHWrapperFile(FileReference OutputFile, FileItem DefinitionsFile, FileItem IncludedFile, IActionGraphBuilder Graph)
{
	// Build the contents of the wrapper file
	StringBuilder WrapperContents = new StringBuilder();
	{
		WrapperContents.AppendLine("// PCH for {0}", IncludedFile.Name);
		WrapperContents.AppendLine("#include \"{0}\"", DefinitionsFile.Name);
		WrapperContents.AppendLine("#include \"{0}\"", IncludedFile.Name);
		WrapperContents.AppendLine("#ifdef __ISPC_ALIGN__");
		WrapperContents.AppendLine("#error ispc.generated.h files are not allowed in precompiled headers (This is a build time optimization to reduce action dependencies)");
		WrapperContents.AppendLine("#endif");
	}

	// Create the item
	FileItem WrapperFile = Graph.CreateIntermediateTextFile(OutputFile, WrapperContents.ToString(), AllowAsync: false);
	...
}
```

**生成的 wrapper 文件长这样**（`Intermediate/Build/.../SharedPCH.Core.h`）：

```cpp
// PCH for CoreSharedPCH.h
#include "SharedDefinitions.Core.h"     // 宏定义
#include "CoreSharedPCH.h"              // 真正的 PCH 头
#ifdef __ISPC_ALIGN__
#error ispc.generated.h files are not allowed in precompiled headers (This is a build time optimization to reduce action dependencies)
#endif
```

| 行 | 干什么 |
|---|---|
| 2074 | 注释：`// PCH for CoreSharedPCH.h` |
| 2075 | include 宏定义文件（`SharedDefinitions.Core.h`） |
| 2076 | include 真正的 PCH 头（`CoreSharedPCH.h`） |
| 2077~2079 | 检查：ISPC 生成的头不许进 PCH（编译期优化） |

> **为什么需要 wrapper**：PCH 编译时要把「宏定义」和「头文件」一起编进去。wrapper 就是「把两者拼起来」的临时文件。

### 3.6 第 ⑦ 步：编译 wrapper → 生成 `.pch`（`UEBuildModuleCPP.cs:1542~1551`）

```csharp
// UEBuildModuleCPP.cs:1542~1551
				// Create the PCH
				CPPOutput Output;
				if (ToolChain == null)
				{
					Output = new CPPOutput();
				}
				else
				{
					Output = ToolChain.CompileAllCPPFiles(CompileEnvironment, new List<FileItem>() { WrapperFile }, Template.OutputDir, "Shared", Graph);
				}
```

| 行 | 干什么 |
|---|---|
| 1550 | **编译 wrapper 文件**（`CompileAllCPPFiles`）→ 生成 `.pch` |

### 3.7 第 ⑧⑨⑩ 步：MSVC 参数（`VCToolChain.cs`）

**创建 PCH**（`VCToolChain.cs:301~302`）：

```csharp
Arguments.Add($"/Yc\"{PchThroughHeaderFilePath}\"");
Arguments.Add($"/Fp\"{CreatePchFilePath}\"");
```

| 参数 | 含义 |
|---|---|
| `/Yc"CoreSharedPCH.h"` | **创建** PCH，以这个头为「截止点」 |
| `/Fp"xxx.pch"` | PCH 输出到哪个文件 |

**使用 PCH**（`VCToolChain.cs:318~319`）：

```csharp
Arguments.Add($"/Yu\"{PchThroughHeaderFilePath}\"");
Arguments.Add($"/Fp\"{UsingPchFilePath}\"");
```

| 参数 | 含义 |
|---|---|
| `/Yu"CoreSharedPCH.h"` | **使用** PCH，跳过这个头之前的所有解析 |
| `/Fp"xxx.pch"` | 从哪个文件读 PCH |

**强制包含**（`VCToolChain.cs:285`）：

```csharp
Arguments.Add($"/FI\"{ForceIncludeFileString}\"");
```

| 参数 | 含义 |
|---|---|
| `/FI"CoreSharedPCH.h"` | **强制**在每个 `.cpp` 最前面 include 这个头（不用你手写） |

---

## 四、`PrecompiledHeaderAction` 三个值（关键）

```csharp
// CppCompileEnvironment.cs（枚举定义）
public enum PrecompiledHeaderAction
{
	None,      // 不用 PCH
	Create,    // 创建 PCH（编译 wrapper 时）
	Include,   // 使用 PCH（编译普通 .cpp 时）
}
```

| 值 | 什么时候用 | 对应 MSVC 参数 |
|---|---|---|
| `None` | 不用 PCH | 无 |
| `Create` | 编译 wrapper 文件时 | `/Yc` |
| `Include` | 编译普通 `.cpp` 时 | `/Yu` |

**源码里的用法**：

| 位置 | 值 | 场景 |
|---|---|---|
| `UEBuildModuleCPP.cs:1422` | `Create` | `CreatePrivatePCH` 里 |
| `UEBuildModuleCPP.cs:1520` | `Create` | `FindOrCreateSharedPCH` 里 |
| `UEBuildModuleCPP.cs:1962` | `Include` | 私有 PCH 使用 |
| `UEBuildModuleCPP.cs:2013` | `Include` | 共享 PCH 使用 |
| `UEBuildModuleCPP.cs:824` / `836` | `None` | 生成代码 / 单文件编译时禁用 |

---

## 五、`Core.Build.cs` 两个 PCH 字段的实际效果

```csharp
// Core.Build.cs 第 13、15 行
PrivatePCHHeaderFile = "Private/CorePrivatePCH.h";
SharedPCHHeaderFile = "Public/CoreSharedPCH.h";
```

**UBT 的处理顺序**（`UEBuildModuleCPP.cs:1956~1978`）：

```
① 先看 PrivatePCHHeaderFile（第 1956 行）
   → 有 → 创建私有 PCH（CorePrivatePCH.h）
   → 设 PrecompiledHeaderAction = Include
        ↓
② 再看 bHasPrecompiledHeader（第 1970 行）
   → 已经是 true（第 ① 步设了）→ 跳过共享 PCH
```

**所以 Core 实际用的是私有 PCH**（`CorePrivatePCH.h`），而 `CorePrivatePCH.h` 第 5 行 include 了 `CoreSharedPCH.h` —— 等于「私有 PCH = 共享 PCH + 5 个额外头」。

**那 `SharedPCHHeaderFile` 有什么用？**

它让 **Core 对外提供**一个共享 PCH 模板（`CreateSharedPCHTemplate`），**别的模块**（如 `Engine` / `Slate`）可以用它。Core 自己不用，但别人用。

> 🔑 **关键**：`SharedPCHHeaderFile` 是「**给别人用的**」，`PrivatePCHHeaderFile` 是「**自己用的**」。

---

## 六、`.pch` 文件在哪（实际路径）

| 项 | 路径 |
|---|---|
| 引擎源码 | `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreSharedPCH.h` |
| 生成的 wrapper | `Engine\Intermediate\Build\<Platform>\<Config>\Core\SharedPCH.Core.h` |
| 生成的 `.pch` | `Engine\Intermediate\Build\<Platform>\<Config>\Core\SharedPCH.Core.h.pch`（或 `.obj`） |
| 生成的宏定义 | `Engine\Intermediate\Build\<Platform>\<Config>\Core\SharedDefinitions.Core.h` |

> 实际路径由 `UEBuildModuleCPP.cs:1371~1379` 决定（`IntermediateDirectory`）。

---

## 七、速查表 · 口诀

| 问题 | 答案 |
|---|---|
| PCH 怎么打开？ | **默认就开**（`bUsePCHFiles = true`，`TargetRules.cs:2109`） |
| 怎么指定用哪个头？ | `Core.Build.cs` 的 `PrivatePCHHeaderFile` / `SharedPCHHeaderFile` |
| 核心代码在哪？ | `UEBuildModuleCPP.cs:1951~2020`（`SetupPrecompiledHeaders`） |
| 创建 PCH 的代码？ | `UEBuildModuleCPP.cs:1412`（`CreatePrivatePCH`）/ `:1449`（`FindOrCreateSharedPCH`） |
| wrapper 生成？ | `UEBuildModuleCPP.cs:2069`（`CreatePCHWrapperFile`） |
| MSVC 参数？ | `VCToolChain.cs:301`（`/Yc`）/ `:318`（`/Yu`）/ `:285`（`/FI`） |
| 三个 Action？ | `None` / `Create` / `Include` |

**口诀**：**PCH 默认开（`bUsePCHFiles=true`）；`Core.Build.cs` 只是「指定用哪个头」；UBT 在 `SetupPrecompiledHeaders` 里决定用私有还是共享；wrapper 文件把「宏定义 + PCH 头」拼起来；MSVC 用 `/Yc` 建、`/Yu` 用、`/FI` 强制包含。**

---

## 八、一句话总结

> **PCH 不需要「打开」——它默认就是开的（`bUsePCHFiles = true`）。`Core.Build.cs` 的 `SharedPCHHeaderFile` / `PrivatePCHHeaderFile` 只是「告诉 UBT 用哪个头当 PCH」。**
>
> **核心代码在 `UEBuildModuleCPP.cs:1951~2020` 的 `SetupPrecompiledHeaders`：先看私有 PCH（第 1956 行），没有再找共享 PCH（第 1970 行）。找到后生成 wrapper 文件（`:2069`），编译成 `.pch`（`:1550`），最后 MSVC 用 `/Yc` 建、`/Yu` 用、`/FI` 强制包含。**

---

## 待确认

1. 这条链路看懂了吗？重点记：**PCH 默认开** + **`SetupPrecompiledHeaders` 是核心**。
2. 看完说「下一步」，我教 `Core.h`（319 行）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\TargetRules.cs`（`bUsePCHFiles` 2109、`bChainPCHs` 2140、`bUseSharedPCHs` 2350）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\UEBuildModuleCPP.cs`（`PrivatePCHHeaderFile` 登记 1131、`SharedPCHHeaderFile` 登记 1137、`CreateSharedPCHTemplate` 1366、`CreatePrivatePCH` 1412、`FindOrCreateSharedPCH` 1449、`IsCompatibleForSharedPCH` 1569、`SetupPrecompiledHeaders` 1951、`CreatePCHWrapperFile` 2069）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Platform\Windows\VCToolChain.cs`（`/FI` 285、`/Yc` 301、`/Fp` 302、`/Yu` 318）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（330 行，第 13、15 行）