# 03 — `UnrealTargetPlatform` 与 `Target` 答疑（为什么它们底下有属性）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs` 第 18~31 行（Android 块）
> 对应真实源码：`UEBuildTarget.cs`（6352 行）、`ReadOnlyTargetRules.cs`（748 行）、`TargetRules.cs`
> **✅ 本节可独立编译**——纯 C# 类型答疑，不涉及 C++。

---

## 一、先看这段代码在问什么

```csharp
if (Target.Platform == UnrealTargetPlatform.Android)
{
	PrivateDependencyModuleNames.Add("GoogleGameSDK");
	PrivateIncludePathModuleNames.Add("VulkanRHI");   // AndroidPlatformMisc 需要

	if (Target.Configuration != UnrealTargetConfiguration.Shipping)
	{
		PrivateDependencyModuleNames.Add("libGPUCounters");
		PrivateDependencyModuleNames.Add("heapprofd");
	}
	string ModulePath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
	AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "Core_APL.xml"));
}
```

这段里有 4 个「点」：

| 写法 | 左边是什么 | 右边是什么 |
|---|---|---|
| `Target.Platform` | `Target`（对象） | `Platform`（它的属性） |
| `UnrealTargetPlatform.Android` | `UnrealTargetPlatform`（**类型**） | `Android`（它的**静态**属性） |
| `Target.Configuration` | `Target`（对象） | `Configuration`（它的属性） |
| `Target.RelativeEnginePath` | `Target`（对象） | `RelativeEnginePath`（它的属性） |

> 🔑 **一句话**：`UnrealTargetPlatform` 是**类型**（点后面跟静态成员）；`Target` 是**对象**（点后面跟实例成员）。两者都是「有成员的类型」，所以底下都有属性。

---

## 二、`UnrealTargetPlatform` 是什么

### 2.1 它不是 `enum`，是 `struct`

```csharp
// UEBuildTarget.cs:30~31
[Serializable, TypeConverter(typeof(UnrealTargetPlatformTypeConverter))]
public partial struct UnrealTargetPlatform : ISerializable
{
	private int Id;                                    // 内部就是一个整数
	private static UniqueStringRegistry? StringRegistry;  // 名字↔ID 的注册表
```

| 事实 | 说明 |
|---|---|
| 声明 | `public partial struct`（**不是** `enum`） |
| 内部 | 只有一个 `private int Id` |
| 名字来源 | `UniqueStringRegistry`（字符串注册表），不是编译期固定的枚举值 |
| 为什么不用 enum | 平台可以由**插件/平台扩展**在运行时注册（Switch、PS5 等不在引擎主干里），enum 做不到 |

> 类比：`enum` 是「印在纸上的固定菜单」；`UnrealTargetPlatform` 是「一块白板 + 一支笔」——引擎先写 8 个，平台扩展插件还能往上加。

### 2.2 它底下的静态属性（8 个内置平台）

```csharp
// UEBuildTarget.cs:252~287
public static UnrealTargetPlatform Win64      { get; } = FindOrAddByName("Win64");
public static UnrealTargetPlatform Mac        { get; } = FindOrAddByName("Mac");
public static UnrealTargetPlatform IOS        { get; } = FindOrAddByName("IOS");
public static UnrealTargetPlatform Android    { get; } = FindOrAddByName("Android");
public static UnrealTargetPlatform Linux      { get; } = FindOrAddByName("Linux");
public static UnrealTargetPlatform LinuxArm64 { get; } = FindOrAddByName("LinuxArm64");
public static UnrealTargetPlatform TVOS       { get; } = FindOrAddByName("TVOS");
public static UnrealTargetPlatform VisionOS   { get; } = FindOrAddByName("VisionOS");
```

| 属性 | 行号 | 说明 |
|---|---|---|
| `Win64` | 252 | 64 位 Windows |
| `Mac` | 257 | macOS |
| `IOS` | 262 | iOS |
| `Android` | 267 | Android ← **本段代码用的就是这个** |
| `Linux` | 272 | Linux x64 |
| `LinuxArm64` | 277 | Linux ARM64 |
| `TVOS` | 282 | tvOS |
| `VisionOS` | 287 | VisionOS |

> 这 8 个是**引擎主干里写死的**；`Win32`、`Switch`、`PS5` 等由平台扩展在运行时 `FindOrAddByName` 注册进来。

### 2.3 它底下的实例方法（`IsInGroup`）

```csharp
// UEBuildTarget.cs:234~237
public bool IsInGroup(UnrealPlatformGroup Group)
{
	return UEBuildPlatform.IsPlatformInGroup(this, Group);
}

// UEBuildTarget.cs:244~247
public bool IsInGroup(string Group)
{
	return UnrealPlatformGroup.TryParse(Group, out UnrealPlatformGroup ActualGroup) && UEBuildPlatform.IsPlatformInGroup(this, ActualGroup);
}
```

| 方法 | 用途 | `Core.Build.cs` 用在哪 |
|---|---|---|
| `IsInGroup(UnrealPlatformGroup)` | 判断「这个平台属不属于某个平台组」 | 第 73、116、134、180、201、235、241、253 行 |
| `IsInGroup(string)` | 同上，但组名用字符串 | 未用 |

> 所以 `Target.Platform.IsInGroup(UnrealPlatformGroup.Windows)` 读法是：**取 Target 的平台 → 问它「你在 Windows 组里吗」**。

### 2.4 配套的 `UnrealPlatformGroup`（也是 struct）

```csharp
// UEBuildTarget.cs:361
public partial struct UnrealPlatformGroup
{
	// 520~570 行：11 个静态属性
	public static UnrealPlatformGroup Windows     { get; } = FindOrAddByName("Windows");
	public static UnrealPlatformGroup Microsoft   { get; } = FindOrAddByName("Microsoft");
	public static UnrealPlatformGroup Apple       { get; } = FindOrAddByName("Apple");
	public static UnrealPlatformGroup IOS         { get; } = FindOrAddByName("IOS");
	public static UnrealPlatformGroup Unix        { get; } = FindOrAddByName("Unix");
	public static UnrealPlatformGroup Linux       { get; } = FindOrAddByName("Linux");
	public static UnrealPlatformGroup Android     { get; } = FindOrAddByName("Android");
	public static UnrealPlatformGroup Desktop     { get; } = FindOrAddByName("Desktop");
	public static UnrealPlatformGroup SDLPlatform { get; } = FindOrAddByName("SDLPlatform");
	public static UnrealPlatformGroup ThirtyHz    { get; } = FindOrAddByName("30Hz");
	public static UnrealPlatformGroup PosixOS     { get; } = FindOrAddByName("PosixOS");
}
```

| 组 | 行号 | 包含哪些平台 |
|---|---|---|
| `Windows` | 520 | Win32 + Win64（注释原话：*just to lump Win32 and Win64 into Windows directories*） |
| `Microsoft` | 525 | 微软系 |
| `Apple` | 530 | 苹果系 |
| `IOS` | 535 | iOS + tvOS（注释：*making IOS a group allows TVOS to compile IOS code*） |
| `Unix` | 540 | Unix 系 |
| `Linux` | 545 | Linux 系 |
| `Android` | 550 | Android 系 |
| `Desktop` | 555 | 桌面（Win/Mac/Linux） |
| `SDLPlatform` | 560 | 用 SDL 的平台 |
| `ThirtyHz` | 565 | 30Hz 平台（Android、Switch） |
| `PosixOS` | 570 | POSIX 兼容 |

> ⚠️ 注意：`UnrealTargetPlatform.Android`（平台）和 `UnrealPlatformGroup.Android`（组）**同名但不同类**。`Core.Build.cs` 第 180 行用的是**组**（`IsInPlatformGroup(UnrealPlatformGroup.Android)`），第 18 行用的是**平台**（`== UnrealTargetPlatform.Android`）。

---

## 三、`Target` 是什么

### 3.1 它是 `ReadOnlyTargetRules` 对象

```csharp
// ModuleRules.cs:664~667
/// <summary>
/// 此模块所属目标的规则
/// </summary>
public ReadOnlyTargetRules Target { get; init; }
```

```csharp
// ReadOnlyTargetRules.cs:13~21
/// <summary>
/// Read-only wrapper around an existing TargetRules instance. This exposes target settings to modules
/// without letting them to modify the global environment.
/// </summary>
public partial class ReadOnlyTargetRules
{
	/// <summary>
	/// The writeable TargetRules instance
	/// </summary>
	readonly TargetRules Inner;
```

| 事实 | 说明 |
|---|---|
| 类型 | `ReadOnlyTargetRules`（**class**，不是 struct） |
| 本质 | 一个**只读包装器**，里面包着可写的 `TargetRules Inner` |
| 为什么只读 | 模块（`*.Build.cs`）不该改全局编译环境，只能读 |
| 怎么来的 | `Core.Build.cs` 构造函数参数 `ReadOnlyTargetRules Target` → `: base(Target)` → 存进 `ModuleRules.Target` |

> 类比：`TargetRules` 是「总包工头手里的施工总表」（可改）；`ReadOnlyTargetRules` 是「贴在工地墙上的复印件」（只能看）。模块拿到的是复印件。

### 3.2 它底下的属性（全部转发给 `Inner`）

```csharp
// ReadOnlyTargetRules.cs:56~70
public UnrealTargetPlatform Platform => Inner.Platform;
public UnrealTargetConfiguration Configuration => Inner.Configuration;
public UnrealArchitectures Architectures => Inner.Architectures;
public UnrealIntermediateEnvironment IntermediateEnvironment => Inner.IntermediateEnvironment;
public UnrealArch Architecture => Inner.Architecture;
public FileReference? ProjectFile => Inner.ProjectFile;
public ReadOnlyBuildVersion Version => Inner.Version;
public TargetType Type => Inner.Type;
```

| 属性 | 行号 | 类型 | `Core.Build.cs` 用在哪 |
|---|---|---|---|
| `Platform` | 56 | `UnrealTargetPlatform` | 第 18、67、73、116、134、180、201、235、241、253、279 行 |
| `Configuration` | 58 | `UnrealTargetConfiguration` | 第 24、99、104、183、241、279 行 |
| `Architectures` | 60 | `UnrealArchitectures` | 未用 |
| `Architecture` | 64 | `UnrealArch` | 未用（用的是 `Target.WindowsPlatform.Architecture`） |
| `Type` | 70 | `TargetType` | 第 104、183、260 行 |
| `Logger` | 73 | `ILogger` | 未用 |
| `DefaultBuildSettings` | 75 | `BuildSettingsVersion` | 未用 |
| `IncludeOrderVersion` | 79 | `EngineIncludeOrderVersion` | 未用 |
| `bBuildEditor` | 见 `TargetRules` | `bool` | 第 61 行 |
| `bCompileICU` | 见 `TargetRules` | `bool` | 第 222 行 |
| `bUseIris` | 见 `TargetRules` | `bool` | 第 270 行 |
| `bMergeModules` | 见 `TargetRules` | `bool` | 第 311 行 |
| `RelativeEnginePath` | 652 | `string` | 第 29 行 |

### 3.3 `RelativeEnginePath` 是什么（第 29 行用的）

```csharp
// ReadOnlyTargetRules.cs:650~652
/// Provide access to the RelativeEnginePath property for code referencing ModuleRules.BuildConfiguration.
public string RelativeEnginePath => Unreal.EngineDirectory.MakeRelativeTo(DirectoryReference.GetCurrentDirectory()) + Path.DirectorySeparatorChar;
```

| 部分 | 含义 |
|---|---|
| `Unreal.EngineDirectory` | 引擎根目录（绝对路径） |
| `DirectoryReference.GetCurrentDirectory()` | 当前工作目录 |
| `MakeRelativeTo(...)` | 算出「引擎根相对当前目录」的相对路径 |
| `+ Path.DirectorySeparatorChar` | 末尾补一个分隔符 |

> 举例：引擎在 `D:\ue5\Epic Games\UE_5.6`，当前目录在 `D:\ue5\Epic Games\UE_5.6`，结果就是 `.\`（或 `./`）。

### 3.4 第 29~30 行在干什么

```csharp
string ModulePath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "Core_APL.xml"));
```

| 步骤 | 代码 | 结果 |
|---|---|---|
| ① | `ModuleDirectory` | `ModuleRules.cs:1550` → `Directory.FullName`，即 `...\Engine\Source\Runtime\Core` |
| ② | `Target.RelativeEnginePath` | 引擎根相对当前目录的路径 |
| ③ | `Utils.MakePathRelativeTo(①, ②)` | 算出「Core 模块相对引擎根」的路径 → `Engine/Source/Runtime/Core` |
| ④ | `Path.Combine(③, "Core_APL.xml")` | `Engine/Source/Runtime/Core/Core_APL.xml` |
| ⑤ | `AdditionalPropertiesForReceipt.Add("AndroidPlugin", ④)` | 把 xml 路径登记进 Android 打包回执 |

> `Utils.MakePathRelativeTo` 定义在 `System\Utils.cs`：`public static string MakePathRelativeTo(string SourcePath, string RelativeToDirectory, bool AlwaysTreatSourceAsDirectory = false)`。
> `AdditionalPropertiesForReceipt` 是 `ReceiptPropertyList`（`ModuleRules.cs:1438`），`Add(name, value)` 在 `ModuleRules.cs:438`。

---

## 四、为什么「他们底下有属性」（核心答疑）

C# 里 `A.B` 的 `B` 可以是三种东西，取决于 `A` 是**类型**还是**对象**：

| 写法 | `A` 是什么 | `B` 是什么 | 例子 |
|---|---|---|---|
| `类型.静态成员` | 类型 | 静态属性 / 静态方法 / 嵌套类型 | `UnrealTargetPlatform.Android` |
| `对象.实例成员` | 对象 | 实例属性 / 实例方法 | `Target.Platform` |
| `对象.属性.方法` | 链式 | 先取属性（返回对象），再调它的方法 | `Target.Platform.IsInGroup(...)` |

**本段代码的完整拆解**：

```
Target.Platform == UnrealTargetPlatform.Android
└─┬──┘ └──┬───┘    └────────┬─────────┘ └──┬──┘
  │       │                 │              │
  │       │                 │              └─ UnrealTargetPlatform 的【静态属性】Android
  │       │                 └─ 类型名（struct）
  │       └─ Target 的【实例属性】Platform，返回 UnrealTargetPlatform
  └─ Target 对象（ReadOnlyTargetRules 实例）
```

```
Target.Platform.IsInGroup(UnrealPlatformGroup.Windows)
└─┬──┘ └──┬───┘ └──┬───┘  └──────────┬──────────────┘
  │       │        │                 └─ UnrealPlatformGroup 的【静态属性】Windows
  │       │        └─ UnrealTargetPlatform 的【实例方法】IsInGroup
  │       └─ 返回 UnrealTargetPlatform（struct 值）
  └─ Target 对象
```

> 🔑 **一句话**：`UnrealTargetPlatform` 是**类型**（所以点后面是静态的 `Android`）；`Target` 是**对象**（所以点后面是实例的 `Platform`）。两者都是「有成员的类型」，所以底下都有属性——只是一个是静态成员、一个是实例成员。

---

## 五、速查表

| 名字 | 是什么 | 定义位置 | 关键成员 |
|---|---|---|---|
| `UnrealTargetPlatform` | `partial struct`（**不是 enum**） | `UEBuildTarget.cs:31` | 8 个静态属性（`Win64`/`Mac`/`IOS`/`Android`/`Linux`/`LinuxArm64`/`TVOS`/`VisionOS`）+ `IsInGroup()` |
| `UnrealPlatformGroup` | `partial struct` | `UEBuildTarget.cs:361` | 11 个静态属性（`Windows`/`Microsoft`/`Apple`/`IOS`/`Unix`/`Linux`/`Android`/`Desktop`/`SDLPlatform`/`ThirtyHz`/`PosixOS`） |
| `UnrealTargetConfiguration` | `enum` | `UEBuildTarget.cs:1066` | `Unknown`/`Debug`/`DebugGame`/`Development`/`Test`/`Shipping` |
| `TargetType` | `enum` | `TargetRules.cs:20` | `Game`/`Editor`/`Client`/`Server`/`Program` |
| `ReadOnlyTargetRules` | `class`（只读包装器） | `ReadOnlyTargetRules.cs:16` | 包着 `TargetRules Inner`，所有属性转发 |
| `TargetRules` | `class`（可写总表） | `TargetRules.cs` | `Platform`（644 行）/ `Configuration`（649 行）等 |

**口诀**：**`UnrealTargetPlatform` 是「白板上的平台名」（struct，可被插件追加）；`Target` 是「墙上那张只读施工总表」（ReadOnlyTargetRules）；`Target.Platform` 从总表里读平台名，`UnrealTargetPlatform.Android` 从白板上取 Android 这个名字，两者相等就进 Android 分支。**

---

## 待确认

1. 看懂「类型.静态成员」vs「对象.实例成员」的区别了吗？
2. 重点记：`UnrealTargetPlatform` **不是 enum 而是 struct**（因为平台可被插件运行时注册）。
3. 看完说「下一步」，回主线继续 `Core.Build.cs` 验收 → `Core_APL.xml`（73 行）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（330 行，第 18~31 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\UEBuildTarget.cs`（6352 行；`UnrealTargetPlatform` 31~288、`UnrealPlatformGroup` 361~572、`UnrealTargetConfiguration` 1066~1097）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ReadOnlyTargetRules.cs`（748 行；`Platform` 56、`Configuration` 58、`Type` 70、`RelativeEnginePath` 652）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\TargetRules.cs`（`TargetType` 20~46、`Platform` 644、`Configuration` 649）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs`（2119 行；`Target` 667、`ModuleDirectory` 1550、`AdditionalPropertiesForReceipt` 1438）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\System\Utils.cs`（`MakePathRelativeTo`）