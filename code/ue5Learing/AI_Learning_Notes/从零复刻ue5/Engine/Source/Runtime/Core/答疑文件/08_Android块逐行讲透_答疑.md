# 08 — 逐行讲透 Android 块（`Core.Build.cs` 第 18~31 行）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs` 第 18~31 行
> **✅ 本节可独立编译**——纯 C# 讲解。

---

## 一、先看整段（14 行）

```csharp
if (Target.Platform == UnrealTargetPlatform.Android)
{
	PrivateDependencyModuleNames.Add("GoogleGameSDK");
	
	PrivateIncludePathModuleNames.Add("VulkanRHI"); 				// AndroidPlatformMisc 需要

	if (Target.Configuration != UnrealTargetConfiguration.Shipping)
	{
		PrivateDependencyModuleNames.Add("libGPUCounters");        // ARM CPU 与 ARM Mali GPU 的性能计数器
		PrivateDependencyModuleNames.Add("heapprofd");      // 把自定义分配器暴露给 Google 的内存分析器
	}
	string ModulePath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
	AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "Core_APL.xml"));
}
```

**一句话**：**「如果这次编的是 Android，就额外挂 4 个东西 + 注册 1 个 xml」**。

---

## 二、第 1 行：`if (Target.Platform == UnrealTargetPlatform.Android)`

```csharp
if (Target.Platform == UnrealTargetPlatform.Android)
```

### 2.1 拆开看

| 部分 | 是什么 | 从哪来 |
|---|---|---|
| `Target` | `ReadOnlyTargetRules` 对象（只读的目标规则） | `ModuleRules.cs:667` |
| `Target.Platform` | 属性，返回 `UnrealTargetPlatform`（当前平台） | `ReadOnlyTargetRules.cs:56` |
| `UnrealTargetPlatform` | **类型**（`partial struct`，不是 enum） | `UEBuildTarget.cs:31` |
| `UnrealTargetPlatform.Android` | 它的**静态属性**，代表 Android 平台 | `UEBuildTarget.cs:267` |
| `==` | 比较两个 `UnrealTargetPlatform` 的 `Id` | `UEBuildTarget.cs:107` |

### 2.2 为什么能 `==`

`UnrealTargetPlatform` 内部只有一个 `int Id`，并且重载了 `operator ==`：

```csharp
// UEBuildTarget.cs:107~110
public static bool operator ==(UnrealTargetPlatform A, UnrealTargetPlatform B)
{
	return A.Id == B.Id;
}
```

### 2.3 白话

> **「这次编的是不是 Android？」** 是 → 进大括号；不是 → 整段跳过。

---

## 三、第 3 行：`PrivateDependencyModuleNames.Add("GoogleGameSDK")`

```csharp
PrivateDependencyModuleNames.Add("GoogleGameSDK");
```

### 3.1 拆开看

| 部分 | 是什么 | 从哪来 |
|---|---|---|
| `PrivateDependencyModuleNames` | `List<string>`，私有依赖模块清单 | `ModuleRules.cs:1284` |
| `.Add(...)` | `List<T>` 的标准方法，往末尾加一项 | .NET |
| `"GoogleGameSDK"` | 模块名（字符串） | — |

### 3.2 `GoogleGameSDK` 是什么（实查）

```
D:\ue5\Epic Games\UE_5.6\Engine\Source\ThirdParty\GoogleGameSDK\GoogleGameSDK.Build.cs
```

```csharp
public class GoogleGameSDK : ModuleRules
{
	public GoogleGameSDK(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;   // 第三方模块
		string GoogleGameSDKBasePath = Target.UEThirdPartySourceDirectory + "GoogleGameSDK/gamesdk";
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			// 按 Debug / Release 选库路径，按 arm64-v8a / x86_64 选架构
			...
		}
	}
}
```

**它是 Google 的 Android 游戏 SDK**（含 `libswappy_static.a` 等），只在 Android 平台有实际内容。

### 3.3 「私有依赖」是什么意思

| 项 | 含义 |
|---|---|
| **加头路径** | Core 的 `.cpp` 能 `#include` GoogleGameSDK 的头 |
| **链接库** | 链接时把 GoogleGameSDK 的 `.a` / `.so` 链进来 |
| **不对外传递** | 别的模块 include Core 的头时，**拿不到** GoogleGameSDK |

### 3.4 白话

> **「Core 自己要调 Google 游戏 SDK 的函数，所以把它的库链进来；但这是 Core 内部的事，不告诉别人。」**

---

## 四、第 5 行：`PrivateIncludePathModuleNames.Add("VulkanRHI")`

```csharp
PrivateIncludePathModuleNames.Add("VulkanRHI"); 				// AndroidPlatformMisc 需要
```

### 4.1 和上一行的区别（关键）

| 项 | `PrivateDependencyModuleNames` | `PrivateIncludePathModuleNames` |
|---|---|---|
| 加头路径 | ✅ | ✅ |
| **链接库** | ✅ **链接** | ❌ **不链接** |
| 引擎注释原话 | *modules that our private code depends on* | *we don't need to "import" or link against* |

### 4.2 `VulkanRHI` 是什么（实查）

```
D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\VulkanRHI\VulkanRHI.Build.cs
```

**Vulkan 图形 API 的 RHI（渲染硬件接口）实现**。

### 4.3 为什么只要头不连库

注释写得很清楚：**`AndroidPlatformMisc` 需要**。

`AndroidPlatformMisc` 是 Core 里的一个类（`AndroidPlatformMisc.cpp`），它要读 Vulkan 的一些**类型定义 / 枚举 / 宏**（比如 `VK_...` 常量），但**不调用 Vulkan 的函数**——所以只要头文件能编译过就行，不需要链接 Vulkan 的库。

> 类比：`Dependency` 是「连人带设备一起借」，`IncludePath` 是「只借说明书，不借设备」。这里 Core 只是要查 Vulkan 的说明书（头文件），不真的用它的机器（库）。

### 4.4 白话

> **「Core 的 AndroidPlatformMisc 要 include VulkanRHI 的头（查点常量），但不调它的函数，所以不链接它的库。」**

---

## 五、第 7~11 行：非 Shipping 才加的两个模块

```csharp
if (Target.Configuration != UnrealTargetConfiguration.Shipping)
{
	PrivateDependencyModuleNames.Add("libGPUCounters");        // ARM CPU 与 ARM Mali GPU 的性能计数器
	PrivateDependencyModuleNames.Add("heapprofd");      // 把自定义分配器暴露给 Google 的内存分析器
}
```

### 5.1 拆开看

| 部分 | 是什么 | 从哪来 |
|---|---|---|
| `Target.Configuration` | 属性，返回 `UnrealTargetConfiguration`（当前配置） | `ReadOnlyTargetRules.cs:58` |
| `UnrealTargetConfiguration` | `enum`（6 个值） | `UEBuildTarget.cs:1066` |
| `UnrealTargetConfiguration.Shipping` | 枚举值，发布配置 | `UEBuildTarget.cs:1096` |
| `!=` | 不等于 | C# |

### 5.2 `UnrealTargetConfiguration` 的 6 个值

| 值 | 含义 |
|---|---|
| `Unknown` | 未知 |
| `Debug` | 调试（全不优化） |
| `DebugGame` | 调试游戏（引擎优化、游戏不优化） |
| `Development` | 开发（默认） |
| `Test` | 测试 |
| `Shipping` | **发布**（正式包，关掉所有调试功能） |

### 5.3 `libGPUCounters` 是什么（实查）

```
D:\ue5\Epic Games\UE_5.6\Engine\Source\ThirdParty\libGPUCounters\libGPUCounters.Build.cs
```

```csharp
public class libGPUCounters : ModuleRules
{
	public libGPUCounters(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		string BasePath = Target.UEThirdPartySourceDirectory + "libGPUCounters/";
		PublicSystemIncludePaths.Add(BasePath + "Public");

		if (Target.Platform == UnrealTargetPlatform.Android && Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			PublicAdditionalLibraries.Add(BasePath + "lib/arm64-v8a/libGPUCounters.so");
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "libGPUCounters_UPL.xml"));
		}
	}
}
```

**ARM CPU / ARM Mali GPU 的性能计数器库**（`.so` 文件）。

### 5.4 `heapprofd` 是什么（实查）

```
D:\ue5\Epic Games\UE_5.6\Engine\Source\ThirdParty\heapprofd\heapprofd.Build.cs
```

```csharp
public class heapprofd : ModuleRules
{
	public heapprofd(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		string BasePath = Target.UEThirdPartySourceDirectory + "heapprofd";
		PublicSystemIncludePaths.Add(BasePath);

		if (Target.Platform == UnrealTargetPlatform.Android && Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "heapprofd_UPL.xml"));
			PublicDefinitions.Add("ANDROID_HEAP_PROFILING_SUPPORTED=1");
		}
	}
}
```

**Google 的堆内存分析器**（`heapprofd` = heap profiler daemon），把 UE 的自定义分配器暴露给 Android Studio 的 Memory Profiler。

### 5.5 为什么只在非 Shipping 加

| 原因 | 说明 |
|---|---|
| 性能开销 | 性能计数器 + 堆分析都会拖慢运行 |
| 包体大小 | 多两个 `.so` |
| 安全 | 发布包不该暴露内部内存布局 |

> 白话：**「这两个是开发期用的诊断工具，正式发布包不要。」**

---

## 六、第 12 行：`Utils.MakePathRelativeTo(...)`

```csharp
string ModulePath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
```

### 6.1 拆开看

| 部分 | 是什么 | 从哪来 |
|---|---|---|
| `Utils` | UBT 的静态工具类 | `System\Utils.cs` |
| `Utils.MakePathRelativeTo` | 静态方法，算相对路径 | `System\Utils.cs:740` |
| `ModuleDirectory` | 本模块目录**绝对路径** | `ModuleRules.cs:1550` |
| `Target.RelativeEnginePath` | 引擎根**相对当前目录**的路径 | `ReadOnlyTargetRules.cs:652` |
| `string ModulePath` | 局部变量，存结果 | — |

### 6.2 方法签名

```csharp
public static string MakePathRelativeTo(string SourcePath, string RelativeToDirectory, bool AlwaysTreatSourceAsDirectory = false)
```

| 参数 | 传的是什么 |
|---|---|
| `SourcePath` | `ModuleDirectory` = `D:\...\UE_5.6\Engine\Source\Runtime\Core` |
| `RelativeToDirectory` | `Target.RelativeEnginePath` = `D:\...\UE_5.6\Engine\` |
| 返回 | `Source\Runtime\Core` |

### 6.3 为什么不用绝对路径

| 原因 | 说明 |
|---|---|
| 可移植 | 换台机器引擎装在别处，绝对路径就废了 |
| 回执要求 | `.target` 回执里存相对路径，打包工具按引擎根解析 |

### 6.4 白话

> **「算出『从引擎根怎么走到 Core 模块』——答案是 `Source\Runtime\Core`。」**

---

## 七、第 13 行：`AdditionalPropertiesForReceipt.Add(...)`

```csharp
AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "Core_APL.xml"));
```

### 7.1 拆开看

| 部分 | 是什么 | 从哪来 |
|---|---|---|
| `AdditionalPropertiesForReceipt` | `ReceiptPropertyList`，构建回执附加属性 | `ModuleRules.cs:1438` |
| `.Add(name, value)` | 往列表加一个键值对 | `ModuleRules.cs:438` |
| `"AndroidPlugin"` | **key**（固定字符串） | — |
| `Path.Combine(ModulePath, "Core_APL.xml")` | **value**（xml 相对路径） | `System.IO` |

### 7.2 `ReceiptPropertyList` 是什么

```csharp
// ModuleRules.cs:413~452
[Serializable]
public class ReceiptPropertyList
{
	readonly List<ReceiptProperty> _inner = new();

	public void Add(string name, string value)
	{
		_inner.Add(new ReceiptProperty(name, value));
	}
}
```

就是一个「键值对列表」，`Add` 往里塞一对 `(name, value)`。

### 7.3 数据流（实查，4 步）

```
① Core.Build.cs:30
   AdditionalPropertiesForReceipt.Add("AndroidPlugin", "Source\Runtime\Core\Core_APL.xml")
        ↓
② UEBuildTarget.cs:2407
   Receipt.AdditionalProperties.AddRange(Module.Rules.AdditionalPropertiesForReceipt.Inner)
        ↓
③ 写入 .target 回执文件（JSON）
        ↓
④ UEDeployAndroid.cs:5953
   Receipt.AdditionalProperties.Where(x => x.Name == "AndroidPlugin")
   → 拿到 xml 路径 → 打包时读它
```

### 7.4 `Core_APL.xml` 是什么（实查）

```
D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core_APL.xml（73 行）
```

```xml
<?xml version="1.0" encoding="utf-8"?>
<!--Core module additions-->
<root xmlns:android="http://schemas.android.com/apk/res/android">
	<init>
		<log text="Core UPL init"/>
	</init>
	<gameActivityImportAdditions>
		<setBoolIsGreaterEqual result="bAndroidProfilerAPIAvailable" arg1="$S(CompileSDK)" arg2="35"/>
		<if condition="bAndroidProfilerAPIAvailable">
			<true>
				<insert>
					import com.epicgames.unreal.androidprofiling.ProfilerAccessor;
				</insert>
			</true>
		</if>
	</gameActivityImportAdditions>
	...
</root>
```

**UPL = Unreal Plugin Language**，一种 XML 脚本，Android 打包时用它往 `AndroidManifest.xml` / `gradle` / Java 代码里**插内容**。

### 7.5 白话

> **「把 `Core_APL.xml` 这个说明书塞进 Android 打包机的插槽（key = `AndroidPlugin`），打包时打包机会读它，往 Android 工程里插 Java 代码。」**
>
> 没有这一行，xml 写了也没人读。

---

## 八、整段串起来（白话版）

```csharp
if (Target.Platform == UnrealTargetPlatform.Android)   // ① 这次编 Android 吗？
{
	PrivateDependencyModuleNames.Add("GoogleGameSDK");  // ② 是 → 链 Google 游戏 SDK
	PrivateIncludePathModuleNames.Add("VulkanRHI");     // ③ 只要 Vulkan 的头（查常量）
	if (Target.Configuration != UnrealTargetConfiguration.Shipping)  // ④ 不是发布包吗？
	{
		PrivateDependencyModuleNames.Add("libGPUCounters");  // ⑤ 是 → 加 GPU 性能计数器
		PrivateDependencyModuleNames.Add("heapprofd");       // ⑥ 加堆内存分析器
	}
	string ModulePath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);  // ⑦ 算 Core 相对引擎根的路径
	AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "Core_APL.xml"));  // ⑧ 把 xml 注册进打包流程
}
```

**一句话总结**：

> **「编 Android 时：链 Google 游戏 SDK、借 Vulkan 的头、开发版再加两个诊断工具、最后把 Core_APL.xml 注册进 Android 打包流程。」**

---

## 九、速查表

| 行 | 代码 | 干什么 | 出处 |
|---|---|---|---|
| 1 | `Target.Platform == UnrealTargetPlatform.Android` | 判断是不是 Android | `ReadOnlyTargetRules.cs:56` / `UEBuildTarget.cs:267` |
| 3 | `PrivateDependencyModuleNames.Add("GoogleGameSDK")` | 链 Google 游戏 SDK | `ModuleRules.cs:1284` |
| 5 | `PrivateIncludePathModuleNames.Add("VulkanRHI")` | 只要 Vulkan 头 | `ModuleRules.cs:1278` |
| 7 | `Target.Configuration != UnrealTargetConfiguration.Shipping` | 判断是不是发布包 | `ReadOnlyTargetRules.cs:58` / `UEBuildTarget.cs:1096` |
| 9 | `PrivateDependencyModuleNames.Add("libGPUCounters")` | GPU 性能计数器 | `ModuleRules.cs:1284` |
| 10 | `PrivateDependencyModuleNames.Add("heapprofd")` | 堆内存分析器 | `ModuleRules.cs:1284` |
| 12 | `Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath)` | 算相对路径 | `System\Utils.cs:740` |
| 13 | `AdditionalPropertiesForReceipt.Add("AndroidPlugin", ...)` | 注册 xml 进打包流程 | `ModuleRules.cs:1438` |

---

## 十、四个模块的真实位置（实查）

| 模块 | 路径 | 类型 |
|---|---|---|
| `GoogleGameSDK` | `Engine\Source\ThirdParty\GoogleGameSDK\GoogleGameSDK.Build.cs` | 第三方 |
| `VulkanRHI` | `Engine\Source\Runtime\VulkanRHI\VulkanRHI.Build.cs` | 运行时 |
| `libGPUCounters` | `Engine\Source\ThirdParty\libGPUCounters\libGPUCounters.Build.cs` | 第三方 |
| `heapprofd` | `Engine\Source\ThirdParty\heapprofd\heapprofd.Build.cs` | 第三方 |

---

## 待确认

1. 这段看懂了吗？重点记：`Dependency` 连库 / `IncludePath` 只要头；`AdditionalPropertiesForReceipt` 是「把 xml 塞进打包机」。
2. 看完说「下一步」，回主线继续 `Core.Build.cs` 验收 → `Core_APL.xml`（73 行）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（330 行，第 18~31 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs`（2119 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ReadOnlyTargetRules.cs`（748 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\UEBuildTarget.cs`（6352 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\System\Utils.cs`（`MakePathRelativeTo` 740）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Platform\Android\UEDeployAndroid.cs`（`AndroidPlugin` 消费 5953）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\ThirdParty\GoogleGameSDK\GoogleGameSDK.Build.cs`
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\ThirdParty\libGPUCounters\libGPUCounters.Build.cs`
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\ThirdParty\heapprofd\heapprofd.Build.cs`
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\VulkanRHI\VulkanRHI.Build.cs`
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core_APL.xml`（73 行）