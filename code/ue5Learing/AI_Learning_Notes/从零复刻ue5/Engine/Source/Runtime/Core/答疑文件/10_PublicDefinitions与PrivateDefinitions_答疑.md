# 10 — `PublicDefinitions.Add` / `PrivateDefinitions.Add` 答疑（为什么这样就能定义宏）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs` 第 302~315 行
> **✅ 本节可独立编译**——纯 C# + UBT 流程答疑。

---

## 一、先看这两行在干什么

```csharp
PrivateDefinitions.Add("PLATFORM_SUPPORTS_BINARYCONFIG=" + (SupportsBinaryConfig(Target) ? "1" : "0"));
PublicDefinitions.Add("WITH_MALLOC_STOMP=" + (bWithMallocStomp ? "1" : "0"));
```

**一句话**：往两个 `List<string>` 里塞字符串，UBT 后面会把这些字符串变成编译器的 `/D` 参数。

---

## 二、它们是什么（源码）

### 2.1 定义位置

```csharp
// ModuleRules.cs:1394~1397
/// <summary>
/// Private compiler definitions for this module
/// </summary>
public List<string> PrivateDefinitions = new();

// ModuleRules.cs:1399~1402
/// <summary>
/// Public compiler definitions for this module
/// </summary>
public List<string> PublicDefinitions = new();
```

| 项 | `PrivateDefinitions` | `PublicDefinitions` |
|---|---|---|
| 类型 | `List<string>` | `List<string>` |
| 出处 | `ModuleRules.cs:1397` | `ModuleRules.cs:1402` |
| 初始值 | `new()`（空列表） | `new()`（空列表） |
| 内容格式 | `"NAME"` 或 `"NAME=VALUE"` | 同 |

### 2.2 区别（关键）

| 项 | `PrivateDefinitions` | `PublicDefinitions` |
|---|---|---|
| 谁定义这个宏 | **只有本模块** | **本模块 + 所有依赖本模块的模块** |
| 引擎注释原话 | *Private compiler definitions for this module* | *Public compiler definitions for this module* |
| 典型用途 | 内部实现开关（如 `PLATFORM_BUILDS_MIMALLOC=1`） | 对外契约（如 `UE_WITH_IRIS=1`） |

> 🔑 **一句话**：`Private` = 只自己知道；`Public` = 告诉所有用我的人。

---

## 三、为什么「Add 一个字符串」就能定义宏（完整链路）

### 3.1 链路总览（5 步）

```
① Core.Build.cs
   PublicDefinitions.Add("WITH_MALLOC_STOMP=1")
        ↓
② UEBuildModule.cs:224
   PublicDefinitions = HashSetFromOptionalEnumerableStringParameter(Rules.PublicDefinitions)
        ↓
③ UEBuildModuleCPP.cs:2466~2468
   AddDefinitions(Result.Definitions, PublicDefinitions)
   Result.Definitions.AddRange(Rules.PrivateDefinitions)
        ↓
④ VCToolChain.cs:2630~2636
   foreach (string Definition in CompileEnvironment.Definitions)
       AddDefinition(Arguments, Definition)
        ↓
⑤ VCToolChain.cs:203~219
   Arguments.Add("/D" + Variable + "=" + Value)
        ↓
   最终命令行：cl.exe ... /DWITH_MALLOC_STOMP=1 ...
        ↓
   C++ 代码里 #if WITH_MALLOC_STOMP 生效
```

### 3.2 逐步看源码

**第 ② 步：`UEBuildModule.cs:224`**

```csharp
// UEBuildModule.cs:215~224
public UEBuildModule(ModuleRules Rules, ...)
{
	this.Rules = Rules;
	...
	PublicDefinitions = HashSetFromOptionalEnumerableStringParameter(Rules.PublicDefinitions);
```

把 `ModuleRules.PublicDefinitions`（`List<string>`）转成 `UEBuildModule.PublicDefinitions`（`HashSet<string>`，去重）。

**第 ③ 步：`UEBuildModuleCPP.cs:2465~2468`**

```csharp
// Add the module's public and private definitions.
AddDefinitions(Result.Definitions, PublicDefinitions);

Result.Definitions.AddRange(Rules.PrivateDefinitions);
```

- `PublicDefinitions` → 走 `AddDefinitions`（会做格式规范化）
- `PrivateDefinitions` → 直接 `AddRange`（不规范化）

**`AddDefinitions` 实现（`UEBuildModule.cs:603~628`）**

```csharp
protected void AddDefinitions(List<string> Target, HashSet<string> Source)
{
	StringBuilder Builder = new();
	foreach (string Def in Source)
	{
		string FixedDef = Def;
		int IndexOfAssign = Def.IndexOf("=");
		if (IndexOfAssign != -1)
		{
			// 把 "NAME = VALUE" 规范成 "NAME=VALUE"（去掉等号两边空格）
			ReadOnlySpan<char> Name = Def.AsSpan().Slice(0, IndexOfAssign);
			ReadOnlySpan<char> Value = Def.AsSpan().Slice(IndexOfAssign + 1);
			...
		}
		Target.Add(FixedDef);
	}
}
```

> 注释原话：*This code also reformats defines to match strict rules which does not allow space before and after '='*

**第 ④ 步：`VCToolChain.cs:2629~2636`**

```csharp
// Preprocessor definitions.
foreach (string Definition in CompileEnvironment.Definitions)
{
	if (!Definition.Contains("_API"))
	{
		AddDefinition(Arguments, Definition);
	}
}
```

> 注意 `if (!Definition.Contains("_API"))` —— 带 `_API` 的宏（如 `CORE_API`）不走这里，它们由 `ModuleApiDefine` 单独处理。

**第 ⑤ 步：`VCToolChain.cs:203~219`**

```csharp
public static void AddDefinition(List<string> Arguments, string Variable, string? Value)
{
	// If the value has a space in it and isn't wrapped in quotes, do that now
	if (Value != null && !Value.StartsWith("\"") && (Value.Contains(' ') || Value.Contains('$')))
	{
		Value = "\"" + Value + "\"";
	}

	if (Value != null)
	{
		Arguments.Add("/D" + Variable + "=" + Value);
	}
	else
	{
		Arguments.Add("/D" + Variable);
	}
}
```

**最终生成的命令行**（MSVC）：

```
cl.exe ... /DWITH_MALLOC_STOMP=1 /DPLATFORM_SUPPORTS_BINARYCONFIG=1 ...
```

**Clang 对应**（`ClangToolChain.cs`）：`-D` 而不是 `/D`。

### 3.3 所以「Add 一个字符串」= 定义宏

| 你写的 | 最终命令行 | C++ 里等价于 |
|---|---|---|
| `Add("FOO=1")` | `/DFOO=1` | `#define FOO 1` |
| `Add("FOO")` | `/DFOO` | `#define FOO` |
| `Add("FOO=0")` | `/DFOO=0` | `#define FOO 0` |

> 🔑 **关键**：`/D` 是编译器的「命令行定义宏」参数，等价于在源文件最前面写 `#define`。所以 UBT 只要把字符串拼成 `/D...` 传给编译器，宏就定义了。

---

## 四、`Core.Build.cs` 里这两行的具体含义

### 4.1 `PLATFORM_SUPPORTS_BINARYCONFIG`

```csharp
PrivateDefinitions.Add("PLATFORM_SUPPORTS_BINARYCONFIG=" + (SupportsBinaryConfig(Target) ? "1" : "0"));
```

| 部分 | 含义 |
|---|---|
| `SupportsBinaryConfig(Target)` | 调本文件第 316 行的方法，恒返回 `true` |
| `? "1" : "0"` | 三元运算符：true → `"1"`，false → `"0"` |
| 最终字符串 | `"PLATFORM_SUPPORTS_BINARYCONFIG=1"` |
| 最终命令行 | `/DPLATFORM_SUPPORTS_BINARYCONFIG=1` |
| C++ 里 | `#if PLATFORM_SUPPORTS_BINARYCONFIG` 为真 |

### 4.2 `WITH_MALLOC_STOMP`

```csharp
PublicDefinitions.Add("WITH_MALLOC_STOMP=" + (bWithMallocStomp ? "1" : "0"));
```

| 部分 | 含义 |
|---|---|
| `bWithMallocStomp` | 本文件第 319 行的只读属性（非 Shipping 且平台是 Mac/Linux/LinuxArm64/Win64 时为 true） |
| 最终字符串 | `"WITH_MALLOC_STOMP=1"` 或 `"WITH_MALLOC_STOMP=0"` |
| 最终命令行 | `/DWITH_MALLOC_STOMP=1` |
| C++ 里 | `#if WITH_MALLOC_STOMP` 控制是否启用内存踩踏检测 |

> 注意：**无论真假都定义**，只是值不同。这样 C++ 里 `#if WITH_MALLOC_STOMP` 永远能编过（不会因为宏未定义而报错）。

---

## 五、为什么用 `Add` 而不是直接写 `#define`

| 原因 | 说明 |
|---|---|
| **C# 不能写 C++ 的 `#define`** | `Core.Build.cs` 是 C# 文件，`#define` 在 C# 里是另一回事 |
| **宏要传给 C++ 编译器** | 只能通过命令行参数 `/D` 传 |
| **UBT 统一管理** | 所有模块的宏集中收集，最后统一拼命令行 |
| **可条件化** | 能根据平台 / 配置动态决定定义什么（如 `if (Target.Platform == Android)`） |

> 类比：`PublicDefinitions.Add("FOO=1")` 是「往清单上写一行」，UBT 最后把整张清单翻译成 `cl.exe /DFOO=1`。

---

## 六、速查表

| 项 | 内容 |
|---|---|
| `PrivateDefinitions` 定义 | `ModuleRules.cs:1397`，`List<string>` |
| `PublicDefinitions` 定义 | `ModuleRules.cs:1402`，`List<string>` |
| 转 HashSet | `UEBuildModule.cs:224` |
| 合并进编译环境 | `UEBuildModuleCPP.cs:2466~2468` |
| 格式规范化 | `UEBuildModule.cs:603~628`（`AddDefinitions`） |
| 转 `/D` 参数 | `VCToolChain.cs:2630~2636` → `AddDefinition`（`:203~219`） |
| 最终命令行 | `cl.exe ... /DNAME=VALUE ...` |
| C++ 等价 | `#define NAME VALUE` |

**口诀**：**`Add` 往清单写字符串 → UBT 收集 → 工具链拼成 `/D` → 编译器当 `#define` 用。`Private` 只自己用，`Public` 传给所有依赖方。**

---

## 七、`Core.Build.cs` 里所有 `Definitions.Add` 一览

### 7.1 `PrivateDefinitions`（16 处）

| 行 | 宏 |
|---|---|
| 69 | `PLATFORM_BUILDS_LIBPAS=1` |
| 80 | `PLATFORM_BUILDS_MIMALLOC=1` |
| 88 | `USE_BUNDLED_DBGHELP=1` |
| 90 | `USE_BUNDLED_DBGHELP=0` |
| 94 | `YIELD_BETWEEN_TASKS=1` |
| 264 | `IS_CLIENT_TARGET=1` |
| 268 | `IS_CLIENT_TARGET=0` |
| 281 | `UE_CALLSTACK_TRACE_ANDROID_USE_STACK_FRAMES_WALKING=1` |
| 283 | `UE_MEMORY_TRACE_ANDROID_ENABLE_SCUDO_TRACING_SUPPORT=1` |
| 285 | `PLATFORM_BUILDS_MIMALLOC=1` |
| 287 | `PLATFORM_SUPPORTS_BINARYCONFIG=...` |
| 289 | `PLATFORM_COMPILER_OPTIMIZATION_LTCG=...` |
| 290 | `PLATFORM_COMPILER_OPTIMIZATION_PG=...` |
| 291 | `PLATFORM_COMPILER_OPTIMIZATION_PG_PROFILING=...` |
| 293 | `UE_DEFINE_LEGACY_MATH_CONSTANT_MACRO_NAMES=0` |
| 295 | `PLATFORM_BUILDS_MIMALLOC=1`（Unix 分支） |

### 7.2 `PublicDefinitions`（9 处）

| 行 | 宏 |
|---|---|
| 105 | `UE_MEMORY_TAGS_TRACE_ENABLED=1` |
| 224 | `UE_ENABLE_ICU=...` |
| 238 | `WITH_ADDITIONAL_CRASH_CONTEXTS=1` |
| 272 | `IS_RUNNING_GAMETHREAD_ON_EXTERNAL_THREAD=1` |
| 276 | `UE_WITH_IRIS=...` |
| 284 | `UE_MEMORY_TRACE_AVAILABLE=1` |
| 290 | `WITH_MALLOC_STOMP=...` |
| 298 | `UE_MERGED_MODULES=...` |
| 311 | `UE_MERGED_MODULES=...` |

---

## 待确认

1. 看懂「Add 字符串 → `/D` 参数 → `#define`」这条链路了吗？
2. 重点记：`Private` 只自己用，`Public` 传给所有依赖方。
3. 看完说「下一步」，回主线继续 `Core.Build.cs` 验收 → `Core_APL.xml`（73 行）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（330 行，第 302~315 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs`（`PrivateDefinitions` 1397、`PublicDefinitions` 1402）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\UEBuildModule.cs`（`PublicDefinitions` 属性 71、赋值 224、`AddDefinitions` 603~628）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\UEBuildModuleCPP.cs`（合并 2466~2468）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Platform\Windows\VCToolChain.cs`（`AddDefinition` 203~219、循环 2630~2636）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\ToolChain\ClangToolChain.cs`（`-D` 版本）