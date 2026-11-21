# 答疑 — `TargetInfo` vs `ReadOnlyTargetRules` 有什么区别？

> 提问：`public LyraGame(ReadOnlyTargetRules Target) : base(Target)` 和 `public LyraEditorTarget(TargetInfo Target) : base(Target)` 有什么区别？
> 依据：`D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\UnrealBuildTool\Configuration\`（UE 5.4 实查）

---

## 一、一句话答案

| 类型 | 给谁用 | 能不能改目标配置 |
|---|---|---|
| `TargetInfo` | `TargetRules` 的构造函数（`*.Target.cs`） | ❌ 只读字段，是「原始参数包」 |
| `TargetRules` | `*.Target.cs` 里的 `this` | ✅ 可写（`Type` / `Logger` / `BuildEnvironment` …） |
| `ReadOnlyTargetRules` | `ModuleRules` 的构造函数（`*.Build.cs`） | ❌ 只读包装，模块不许改目标 |

🔑 **一句话**：`TargetInfo` 是「命令行递进来的报名表」，`TargetRules` 是「填好的可改档案」，`ReadOnlyTargetRules` 是「复印一份只许看」。

---

## 二、三个类型的真实定义

### 1. `TargetInfo` —— 原始参数包（只读）

出处：`TargetInfo.cs:12`

```csharp
public class TargetInfo
{
	// 目标名（如 "LyraGame" / "LyraEditor"）
	public readonly string Name;                                    // L17
	// 目标平台（Win64 / Android / IOS …）
	public readonly UnrealTargetPlatform Platform;                  // L22
	// 编译配置（Debug / Development / Shipping …）
	public readonly UnrealTargetConfiguration Configuration;        // L27
	// CPU 架构（x64 / Arm64 …）
	public readonly UnrealArchitectures Architectures;              // L32
	// 工程文件路径（.uproject）
	public readonly FileReference? ProjectFile;                     // L42
	// 当前构建版本
	public ReadOnlyBuildVersion Version => ReadOnlyBuildVersion.Current;  // L47
	// 额外命令行参数
	public CommandLineArguments? Arguments;                         // L52
}
```

**特点**：
- 全部是 `readonly` 字段 —— **一个都不能改**
- 只有 7 个成员 —— 就是命令行传进来的那点信息
- **没有** `Type`（Game/Editor/Server）、**没有** `Logger`、**没有** `BuildEnvironment`

> 所以 `Target.cs` 构造函数里拿到 `TargetInfo` 后，**第一件事就是 `: base(Target)` 把它交给 `TargetRules`**，自己不用它。

---

### 2. `TargetRules` —— 可写的目标规则

出处：`TargetRules.cs:2853`（构造函数）、`TargetRules.cs:633`（`Type`）

```csharp
public TargetRules(TargetInfo target)
{
	// 把 TargetInfo 的字段抄进自己身上
	DefaultName = target.Name;
	Platform = target.Platform;
	Configuration = target.Configuration;
	Architectures = target.Architectures;
	IntermediateEnvironment = target.IntermediateEnvironment;
	// ……
}
```

**可写成员（节选）**：

| 成员 | 行号 | 说明 |
|---|---|---|
| `Type` | `TargetRules.cs:633` | `TargetType.Game` / `Editor` / `Server` / `Client`，**可 set** |
| `Logger` | `TargetRules.cs:584` | UBT 日志器 |
| `Platform` | `TargetRules.cs:597` | 平台（`init`，构造时可设） |
| `Configuration` | `TargetRules.cs:602` | 配置（`init`） |
| `ProjectFile` | `TargetRules.cs:623` | 工程文件（`init`） |
| `Version` | `TargetRules.cs:628` | 构建版本（`init`） |
| `BuildEnvironment` | `TargetRules.cs:2450` | `Unique` / `Shared` |

**这就是 `*.Target.cs` 里 `this` 的类型** —— 所以你能写：

```csharp
Type = TargetType.Game;                          // 改 Type
ExtraModuleNames.AddRange(new string[] { "LyraGame" });
Target.bUseLoggingInShipping = true;             // 改 Shipping 日志
```

---

### 3. `ReadOnlyTargetRules` —— 只读包装

出处：`ReadOnlyTargetRules.cs:16`

```csharp
public partial class ReadOnlyTargetRules
{
	// 内部包着一个可写的 TargetRules
	readonly TargetRules Inner;                  // L21

	public ReadOnlyTargetRules(TargetRules inner) // L27
	{
		Inner = inner;
		// ……
	}

	// 下面全是「只读转发」—— 只有 get，没有 set
	public string Name => Inner.Name;                                    // L43
	public UnrealTargetPlatform Platform => Inner.Platform;              // L55
	public UnrealTargetConfiguration Configuration => Inner.Configuration; // L57
	public UnrealArchitectures Architectures => Inner.Architectures;     // L59
	public FileReference? ProjectFile => Inner.ProjectFile;              // L65
	public ReadOnlyBuildVersion Version => Inner.Version;                // L67
	public TargetType Type => Inner.Type;                                // L69
	public ILogger Logger => Inner.Logger;                               // L72
	public BuildSettingsVersion DefaultBuildSettings => Inner.DefaultBuildSettings; // L74
	public EngineIncludeOrderVersion IncludeOrderVersion => Inner.IncludeOrderVersion; // L78
	public TargetBuildEnvironment BuildEnvironment => Inner.BuildEnvironment; // L531
}
```

**特点**：
- 全部是 `=> Inner.XXX` 的**只读转发**，**一个 set 都没有**
- 成员比 `TargetInfo` 多得多（`Type` / `Logger` / `BuildEnvironment` / `DefaultBuildSettings` …）
- 内部持有 `readonly TargetRules Inner` —— 真正的数据在 `TargetRules` 里

---

## 三、为什么 `Build.cs` 只给只读版？

出处：`ModuleRules.cs:1440`

```csharp
public ModuleRules(ReadOnlyTargetRules target)
```

**设计意图**（`ReadOnlyTargetRules.cs:14` 的注释原文）：

> Read-only wrapper around an existing TargetRules instance. This exposes target settings to modules without letting them to modify the global environment.

翻译：**「把目标设置暴露给模块，但不让模块修改全局环境。」**

**为什么必须这样**：

| 场景 | 如果模块能改目标配置 | 后果 |
|---|---|---|
| 100 个模块都改 `Type` | 谁最后执行谁说了算 | 编译结果不确定 |
| 模块改 `Configuration` | 一个模块把 Development 改成 Shipping | 整个工程配置错乱 |
| 模块改 `Platform` | 交叉编译时平台被篡改 | 产物跑不起来 |

所以 UBT 用「只读包装」做**权限隔离**：模块只能**读**目标配置（决定自己依赖哪些模块），不能**写**。

---

## 四、调用链全景

```
命令行：UnrealBuildTool.exe TenkaichiEditor Win64 Development -Project=Tenkaichi.uproject
   ↓
UBT 解析出 TargetInfo（Name / Platform / Configuration / Architectures / ProjectFile / Arguments）
   ↓
new LyraEditorTarget(TargetInfo)          ← *.Target.cs 构造函数收到 TargetInfo
   ↓  : base(Target)
new TargetRules(TargetInfo)               ← 把 TargetInfo 字段抄进自己
   ↓
Target.cs 里改 this.Type / this.Logger / this.BuildEnvironment …   ← 可写
   ↓
new ReadOnlyTargetRules(TargetRules)      ← 包一层只读壳
   ↓
new LyraGame(ReadOnlyTargetRules)         ← *.Build.cs 构造函数收到只读壳
   ↓
Build.cs 里只能读 Target.Platform / Target.Type / Target.bBuildAllModules …   ← 只读
```

---

## 五、对照表（背这个就够）

| 问题 | `TargetInfo` | `TargetRules` | `ReadOnlyTargetRules` |
|---|---|---|---|
| 出现在哪个文件 | `*.Target.cs` 构造函数参数 | `*.Target.cs` 里的 `this` | `*.Build.cs` 构造函数参数 |
| 能改 `Type` 吗 | ❌ 没这个字段 | ✅ `Type = TargetType.Game` | ❌ 只有 get |
| 能改 `bUseLoggingInShipping` 吗 | ❌ | ✅ | ❌ |
| 能读 `Platform` 吗 | ✅ | ✅ | ✅ |
| 能读 `bBuildAllModules` 吗 | ❌ | ✅ | ✅ |
| 成员数量 | 7 个 | 数百个 | 数百个（全只读） |
| 本质 | 命令行参数包 | 真正的规则对象 | `TargetRules` 的只读视图 |

---

## 六、回到你的两个文件

### `LyraGame.Target.cs`（第 02 轮）

```csharp
public LyraGameTarget(TargetInfo Target) : base(Target)
{
	Type = TargetType.Game;                    // ← 改 this.Type（TargetRules 的可写属性）
	ExtraModuleNames.AddRange(new string[] { "LyraGame" });
	LyraGameTarget.ApplySharedLyraTargetSettings(this);   // ← this 是 TargetRules
}
```

- 参数 `TargetInfo` 只是「过路」，`: base(Target)` 交给父类后就不用了
- `this` 是 `TargetRules`，所以能改 `Type`

### `LyraGame.Build.cs`（第 04 轮）

```csharp
public LyraGame(ReadOnlyTargetRules Target) : base(Target)
{
	PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	// 只能读 Target.XXX，不能改
}
```

- 参数 `ReadOnlyTargetRules` 是只读壳
- 想读 `Target.bBuildAllModules`（`LyraEditor.Target.cs:14` 用过）也能读，但改不了

### `LyraEditor.Target.cs`（第 03 轮）

```csharp
public LyraEditorTarget(TargetInfo Target) : base(Target)
{
	Type = TargetType.Editor;
	ExtraModuleNames.AddRange(new string[] { "LyraGame", "LyraEditor" });
	if (!bBuildAllModules)                     // ← bBuildAllModules 是 TargetRules 的属性
	{
		NativePointerMemberBehaviorOverride = PointerMemberBehavior.Disallow;
	}
	LyraGameTarget.ApplySharedLyraTargetSettings(this);   // ← this 是 TargetRules
	EnablePlugins.Add("RemoteSession");
}
```

---

## 七、一句话总结

**`TargetInfo` = 报名表（只读、7 个字段）；`TargetRules` = 档案（可写、几百个字段）；`ReadOnlyTargetRules` = 档案的复印件（只读、几百个字段）。`*.Target.cs` 用前两个，`*.Build.cs` 用第三个 —— 因为模块不许改目标配置。**

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\UnrealBuildTool\Configuration\TargetInfo.cs:12-52`（`TargetInfo` 类与 7 个成员）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\UnrealBuildTool\Configuration\TargetRules.cs:2853-2860`（`TargetRules(TargetInfo)` 构造函数）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\UnrealBuildTool\Configuration\TargetRules.cs:633`（`Type` 可 set）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\UnrealBuildTool\Configuration\TargetRules.cs:584,597,602,623,628,2450`（`Logger` / `Platform` / `Configuration` / `ProjectFile` / `Version` / `BuildEnvironment`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\UnrealBuildTool\Configuration\ReadOnlyTargetRules.cs:14-35`（类注释 + `readonly TargetRules Inner` + 构造函数）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\UnrealBuildTool\Configuration\ReadOnlyTargetRules.cs:43-78,531`（只读转发属性）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs:103,1440`（`ModuleRules` 类 + `ModuleRules(ReadOnlyTargetRules)` 构造函数）