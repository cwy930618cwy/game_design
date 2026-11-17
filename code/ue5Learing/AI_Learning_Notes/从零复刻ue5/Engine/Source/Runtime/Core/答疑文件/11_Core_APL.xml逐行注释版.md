# 11 — `Core_APL.xml` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core_APL.xml`（**73 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Core_APL.xml`
> **✅ 本节可独立编译**——XML 脚本，不参与编译，只在 Android 打包时被 UBT 读取执行。
> **注意**：这份是**讲解版**（每行都有详细注释），交付版在 `02_Core_APL.xml_完整代码.md`。

---

## 一、这个文件解决什么问题（为什么）

上一课 `Core.Build.cs` 第 30 行有这么一句：

```csharp
AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "Core_APL.xml"));
```

它把本文件**注册进 Android 打包流程**。打包时 UBT 会读这个 XML，按里面的指令往 Android 工程里**插 Java 代码 / 改 gradle / 拷文件**。

**说个场景**：Android 上要跑 Android Studio 的 Memory Profiler（内存分析器），需要往 `GameActivity.java` 里插一行 `import`、在 `onCreate` 里调一次 `ProfilerAccessor.Init(this)`、还要把 `ProfilerAccessor.java` 拷进工程、并在 gradle 里加依赖。这些「往别人工程里插东西」的活，就是 UPL（Unreal Plugin Language）干的。

> 🔑 **一句话**：73 行 XML，写着「**如果编译 SDK ≥ 35，就往 Android 工程里插 5 处东西，让 Memory Profiler 能用**」。

---

## 二、UPL 是什么（先搞清语言）

**UPL = Unreal Plugin Language**，UBT 内置的一种 **XML 脚本语言**，专门用来「往打包产物里插东西」。

**源码**：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\System\UnrealPluginLanguage.cs`（3552 行）

**核心机制**（`UnrealPluginLanguage.cs:17~26` 注释原文）：

> UPL 是一种简单的、基于 XML 的语言，用于操作 XML 并返回字符串。它包含一个 `<init>` 段，该段在每个架构下、在任何其他段之前执行一次。状态会被保留并传递到下一个被执行的段，因此各段的执行顺序很重要。
>
> 虽然 UPL 是一个通用的、用于修改和查询 XML 的系统，但它专门用于让插件影响其所属包的全局配置。例如，这允许插件修改 Android APK 的 `AndroidManifest.xml` 文件，或 iOS IPA 的 plist 文件。UBT 还会查询插件的 UPL XML 文件，以获取需要写入包内公共文件（例如 Android 上的某些 `.java` 文件）的字符串。
>
> 如果你想在插件上下文中查看实际执行的指令，可以加上下面这行来开启追踪：
>
> `<trace enable="true"/>`
>
> 加上之后，你的上下文中所有实际执行的节点都会被写入日志，直到你执行 `<trace enable="false"/>`。你也可以用下面这条命令导出上下文中的所有变量：
>
> `<dumpvars/>`
>
> 支持 Bool、Int、String 三种变量类型。任何属性都可以引用变量，并在求值前用下面的语法替换成对应的字符串：
>
> - `$B(name)` = 布尔变量 `name` 的值
> - `$I(name)` = 整数变量 `name` 的值
> - `$S(name)` = 字符串变量 `name` 的值
> - `$E(name)` = 元素变量 `name` 的值

**变量语法**（`UnrealPluginLanguage.cs:42~45`）：

| 语法 | 含义 |
|---|---|
| `$B(name)` | 布尔变量 |
| `$I(name)` | 整数变量 |
| `$S(name)` | 字符串变量 |
| `$E(name)` | XML 元素变量 |

**自动初始化的变量**（`UnrealPluginLanguage.cs:49~61`）：

| 变量 | 含义 |
|---|---|
| `$S(Output)` | 本段输出（初始化为 Input） |
| `$S(UnrealArch)` | 目标 UE 架构（arm32 / arm64 / x86 / x64） |
| `$S(Architecture)` | 目标 Android 架构（armeabi-armv7a / arm64-v8a / x86 / x86_64） |
| `$S(PluginDir)` | XML 文件所在目录 |
| `$S(EngineDir)` | 引擎目录 |
| `$S(BuildDir)` | 项目平台构建目录（Intermediate 下） |
| `$S(Configuration)` | 配置（Debug / DebugGame / Development / Test / Shipping） |
| `$B(Distribution)` | 是否分发构建 |
| `$I(EngineMajorVersion)` | 引擎主版本号 |

> 本文件用到的只有 `$S(CompileSDK)`（编译 SDK 版本）、`$S(EngineDir)`、`$S(BuildDir)`。

---

## 三、完整代码（每行都有详细注释）

```xml
<?xml version="1.0" encoding="utf-8"?>
<!-- XML 声明：版本 1.0，编码 UTF-8。所有 XML 文件第一行都是它。 -->

<!--Core module additions-->
<!-- 注释：这是 Core 模块的附加内容（给人看的，不影响执行）。 -->

<!-- 根节点。xmlns:android 声明 Android 命名空间，
     让下面可以用 android:xxx 形式的属性（本文件其实没用到，但 UPL 要求保留）。 -->
<root xmlns:android="http://schemas.android.com/apk/res/android">

	<!-- init section is always evaluated once per architecture -->
	<!-- init 段：每个架构执行一次，且在所有其他段之前执行。
	     执行入口在 UnrealPluginLanguage.cs:2696（ProcessPluginNode(Arch, "init", "")）。 -->
	<init>
		<!-- 往 UBT 日志打一行 "Core UPL init"。
		     实现：UnrealPluginLanguage.cs:1310（case "log"）→ Logger.LogInformation。 -->
		<log text="Core UPL init"/>
		<!-- we cant set bAndroidProfilerAPIAvailable $S(CompileSDK) -->
		<!-- 注释：这里不能设置 bAndroidProfilerAPIAvailable，因为 $S(CompileSDK) 在 init 阶段还没准备好。
		     所以每个段里都重新算一次（见下面 5 个段）。 -->
	</init>

	<!-- 段 1：往 GameActivity.java 的 import 区插内容。
	     消费方：UEDeployAndroid.cs:6235（替换 "//$${gameActivityImportAdditions}$$" 占位符）。 -->
	<gameActivityImportAdditions>
		<!-- 设置布尔变量 bAndroidProfilerAPIAvailable = (CompileSDK >= 35)。
		     实现：UnrealPluginLanguage.cs:2086（case "setBoolIsGreaterEqual"）
		           → CurrentContext.BoolVariables[Result] = (StringToInt(Arg1) >= StringToInt(Arg2))。
		     $S(CompileSDK) = 编译用的 Android SDK 版本号（如 34 / 35 / 36）。 -->
		<setBoolIsGreaterEqual result="bAndroidProfilerAPIAvailable" arg1="$S(CompileSDK)" arg2="35"/>
		<!-- 条件判断：如果 bAndroidProfilerAPIAvailable 为真。
		     实现：UnrealPluginLanguage.cs:1234（case "if"）→ 取 <true> 或 <false> 子节点执行。 -->
		<if condition="bAndroidProfilerAPIAvailable">
			<!-- 条件为真时执行 -->
			<true>
				<!-- 打日志：编译 SDK 是 35+，加 Android Profiler 访问权限。 -->
				<log text="Compile SDK is 35+, adding Android Profiler access."/>
				<!-- 插入内容。
				     实现：UnrealPluginLanguage.cs:1721（case "insert"）→ 把子节点文本追加到 $S(Output)。 -->
				<insert>
					import com.epicgames.unreal.androidprofiling.ProfilerAccessor;
				</insert>
			</true>
			<!-- 条件为假时执行 -->
			<false>
				<!-- 打日志：编译 SDK 低于 35，Android Profiler 访问不可用。 -->
				<log text="Compile SDK is below 35, Android Profiler access will not available."/>
			</false>
		</if>
	</gameActivityImportAdditions>
	
	<!-- 段 2：往 GameActivity.java 的 onCreate() 方法里插内容。
	     消费方：UEDeployAndroid.cs:6245（替换 "//$${gameActivityOnCreateAdditions}$$" 占位符）。 -->
	<gameActivityOnCreateAdditions>
		<!-- 同样先算 bAndroidProfilerAPIAvailable = (CompileSDK >= 35)。
		     为什么重复算：UPL 的变量是「段内上下文」，跨段不保证保留（见 UnrealPluginLanguage.cs:63~65 注释）。 -->
		<setBoolIsGreaterEqual result="bAndroidProfilerAPIAvailable" arg1="$S(CompileSDK)" arg2="35"/>
		<!-- 如果 SDK >= 35 -->
		<if condition="bAndroidProfilerAPIAvailable">
			<true>
				<!-- 插入 Java 代码到 onCreate() 里 -->
				<insert>
					<!-- CDATA 段：里面的内容原样输出，不解析 XML 特殊字符（< > & 等）。
					     这里插的是 Java 语句：调用 ProfilerAccessor.Init(this)。 -->
					<![CDATA[
					ProfilerAccessor.Init(this);
					]]>
				</insert>
			</true>

		</if>
	</gameActivityOnCreateAdditions>

	<!-- 段 3：往 proguard 规则文件里插内容（防止混淆器删掉 ProfilerAccessor 类）。
	     消费方：UEDeployAndroid.cs:3300（ProcessPluginNode(Arch, "proguardAdditions", ...)）。 -->
	<proguardAdditions>
		<!-- 同样先算 bAndroidProfilerAPIAvailable -->
		<setBoolIsGreaterEqual result="bAndroidProfilerAPIAvailable" arg1="$S(CompileSDK)" arg2="35"/>
		<if condition="bAndroidProfilerAPIAvailable">
			<true>
				<insert>
					<!-- ProGuard 规则：保留 com.epicgames.unreal.androidprofiling.ProfilerAccessor 类的所有 public 成员。
					     为什么：ProGuard 会删掉「看起来没用到」的类，但 ProfilerAccessor 是被反射调用的，
					     不写这条规则会被删掉，导致运行时崩溃。 -->
					-keep class com.epicgames.unreal.androidprofiling.ProfilerAccessor {
					public *;
					}
				</insert>
			</true>
		</if>
	</proguardAdditions>

	<!-- 段 4：拷贝资源文件到构建目录。
	     消费方：UEDeployAndroid.cs:5294（ProcessPluginNode(NDKArch, "resourceCopies", "")）。 -->
	<resourceCopies>
		<!-- 同样先算 bAndroidProfilerAPIAvailable -->
		<setBoolIsGreaterEqual result="bAndroidProfilerAPIAvailable" arg1="$S(CompileSDK)" arg2="35"/>
		<if condition="bAndroidProfilerAPIAvailable">
			<true>
				<!-- 拷贝文件。
				     实现：UnrealPluginLanguage.cs:1786（case "copyFile"）→ File.Copy(Src, Dst, true)。
				     src = 引擎里的 ProfilerAccessor.java 源文件
				           $S(EngineDir) = 引擎根目录
				     dst = 构建目录里的目标位置
				           $S(BuildDir) = Intermediate/Android/APK 之类的构建目录
				     为什么要拷：这个 .java 文件不在 Android 工程里，必须拷进去才能编译。 -->
				<copyFile src="$S(EngineDir)/Source/Runtime/Core/Private/Android/Java/ProfilerAccessor.java" dst="$S(BuildDir)/src/com/epicgames/unreal/androidprofiling/ProfilerAccessor.java" />
			</true>
		</if>
	</resourceCopies>
	
	<!-- 段 5：往 app 的 build.gradle 里插依赖。
	     消费方：UEDeployAndroid.cs:4213（ProcessPluginNode(NDKArch, "buildGradleAdditions", ...)）。 -->
	<buildGradleAdditions>
		<!-- 同样先算 bAndroidProfilerAPIAvailable -->
		<setBoolIsGreaterEqual result="bAndroidProfilerAPIAvailable" arg1="$S(CompileSDK)" arg2="35"/>
		<if condition="bAndroidProfilerAPIAvailable">
			<true>
				<insert>
				<!-- Gradle 依赖块：引入 androidx.core 1.15.0。
				     为什么：ProfilerAccessor.java 里用到了 androidx.core 的 API（如 Profiler 相关类），
				     不加这个依赖会编译失败。 -->
				dependencies {
					implementation('androidx.core:core:1.15.0')
				}
				</insert>
			</true>
		</if>
	</buildGradleAdditions>
</root>
```

---

## 四、逐段拆解（本段新元素 + 这段在干什么）

### 4.1 XML 声明与根节点（第 1~3 行）

**本段新元素**

- `<?xml version="1.0" encoding="utf-8"?>` —— XML 声明，版本 1.0 + UTF-8 编码
- `<root>` —— UPL 的根节点，所有段都挂在它下面
- `xmlns:android` —— XML 命名空间声明（本文件没实际用到，UPL 要求保留）

**这段在干什么**：声明这是一个 XML 文件，根节点叫 `root`。

> 类比：`<root>` 是「一个文件夹」，下面 6 个段是「6 个子文件夹」，每个子文件夹装一类指令。

### 4.2 `<init>` 段（第 4~8 行）

**本段新元素**

- `<init>` —— **初始化段**，每个架构执行一次，且在所有其他段之前（`UnrealPluginLanguage.cs:2696`）
- `<log text="..."/>` —— 往 UBT 日志打一行（`UnrealPluginLanguage.cs:1310`）

**这段在干什么**：打一行日志 `Core UPL init`，证明这个 UPL 脚本被加载了。

> 注意第 7 行注释：`we cant set bAndroidProfilerAPIAvailable $S(CompileSDK)` —— 在 `init` 阶段 `$S(CompileSDK)` 还没准备好，所以每个段里都重新算一次。

### 4.3 `<gameActivityImportAdditions>` 段（第 10~23 行）

**本段新元素**

- `<gameActivityImportAdditions>` —— 往 `GameActivity.java` 的 **import 区**插内容（`UEDeployAndroid.cs:6235`）
- `<setBoolIsGreaterEqual result="" arg1="" arg2=""/>` —— 设置布尔变量 = (arg1 >= arg2)（`UnrealPluginLanguage.cs:2086`）
- `$S(CompileSDK)` —— 编译用的 Android SDK 版本号
- `<if condition="">` —— 条件判断（`UnrealPluginLanguage.cs:1234`）
- `<true>` / `<false>` —— `if` 的真 / 假分支
- `<insert>` —— 插入内容到输出（`UnrealPluginLanguage.cs:1721`）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 11 | 算 `bAndroidProfilerAPIAvailable = (CompileSDK >= 35)` |
| 12 | 如果为真 |
| 14 | 打日志「SDK 35+，加 Profiler 访问」 |
| 15~17 | 插入 `import com.epicgames.unreal.androidprofiling.ProfilerAccessor;` |
| 19~21 | 否则打日志「SDK < 35，不可用」 |

> 类比：这是「往 Java 文件的 import 区塞一行 import」——就像你在 C++ 文件顶部加 `#include`。

### 4.4 `<gameActivityOnCreateAdditions>` 段（第 25~37 行）

**本段新元素**

- `<gameActivityOnCreateAdditions>` —— 往 `GameActivity.java` 的 **`onCreate()` 方法**里插内容（`UEDeployAndroid.cs:6245`）
- `<![CDATA[...]]>` —— XML 的 CDATA 段，里面的内容**原样输出**，不解析 `<` `>` `&` 等特殊字符

**这段在干什么**：如果 SDK ≥ 35，往 `onCreate()` 里插一行 Java 代码 `ProfilerAccessor.Init(this);`。

> 类比：`onCreate()` 是 Android Activity 的「启动时执行」，插这一行 = 「App 一启动就初始化 Profiler」。

### 4.5 `<proguardAdditions>` 段（第 39~50 行）

**本段新元素**

- `<proguardAdditions>` —— 往 ProGuard 规则文件里插内容（`UEDeployAndroid.cs:3300`）

**这段在干什么**：如果 SDK ≥ 35，插入 ProGuard 规则 `-keep class ...ProfilerAccessor { public *; }`。

> 为什么：ProGuard 会删掉「看起来没用到」的类，但 `ProfilerAccessor` 是被反射调用的，不写这条规则会被删掉，导致运行时崩溃。

### 4.6 `<resourceCopies>` 段（第 52~59 行）

**本段新元素**

- `<resourceCopies>` —— 拷贝资源文件到构建目录（`UEDeployAndroid.cs:5294`）
- `<copyFile src="" dst=""/>` —— 拷贝文件（`UnrealPluginLanguage.cs:1786`）
- `$S(EngineDir)` —— 引擎根目录
- `$S(BuildDir)` —— 构建目录

**这段在干什么**：如果 SDK ≥ 35，把引擎里的 `ProfilerAccessor.java` 拷到构建目录的 `src/com/epicgames/unreal/androidprofiling/` 下。

> 为什么：这个 `.java` 文件不在 Android 工程里，必须拷进去才能编译。

### 4.7 `<buildGradleAdditions>` 段（第 61~72 行）

**本段新元素**

- `<buildGradleAdditions>` —— 往 app 的 `build.gradle` 里插内容（`UEDeployAndroid.cs:4213`）

**这段在干什么**：如果 SDK ≥ 35，插入 Gradle 依赖 `implementation('androidx.core:core:1.15.0')`。

> 为什么：`ProfilerAccessor.java` 用到了 `androidx.core` 的 API，不加依赖会编译失败。

---

## 五、五个段的关系（一张图）

```
Core_APL.xml
├─ <init>                          打日志 "Core UPL init"
├─ <gameActivityImportAdditions>   往 GameActivity.java 的 import 区插 import
├─ <gameActivityOnCreateAdditions> 往 GameActivity.java 的 onCreate() 插 ProfilerAccessor.Init(this)
├─ <proguardAdditions>             往 ProGuard 规则插 -keep class ProfilerAccessor
├─ <resourceCopies>                拷 ProfilerAccessor.java 到构建目录
└─ <buildGradleAdditions>          往 build.gradle 插 androidx.core 依赖

全部条件：CompileSDK >= 35
```

**为什么 5 个段都要重算 `bAndroidProfilerAPIAvailable`**：UPL 的变量是「段内上下文」，跨段不保证保留（`UnrealPluginLanguage.cs:63~65` 注释原文：*除上述变量外，其余变量都在插件上下文中，以避免命名空间冲突；尝试给上述变量（`Output` 除外）赋新值，只会影响当前上下文*）。

---

## 六、我们这版 vs 引擎的差异

| 项 | 引擎原文 | 我们这版 | 说明 |
|----|---------|---------|------|
| 文件名 | `Core_APL.xml` | `Core_APL.xml` | **完全一致** |
| 路径 | `Engine\Source\Runtime\Core\` | `ue_5.6\Engine\Source\Runtime\Core\` | 只换根 |
| 行数 | 73 行 | 73 行 | **行数一致** |
| 英文注释 | 6 处 | 0 处 | 全部译成中文 |
| 补注释 | 无 | 每段上方 + 关键行上方 | 允许的例外 |
| Tab 缩进 | 有 | 原样保留 | 照抄 |
| 第 24 / 60 行带 Tab 的空行 | 有 | 原样保留 | 照抄 |

> 除注释外，**XML 内容一字不差**。

---

## 七、收尾

到这里 `Core_APL.xml` 就完了（73 行）。

**结构**：1 个 `<root>` + 6 个段（`init` / `gameActivityImportAdditions` / `gameActivityOnCreateAdditions` / `proguardAdditions` / `resourceCopies` / `buildGradleAdditions`）。

**关键概念**：
- UPL = Unreal Plugin Language，XML 脚本，往打包产物里插东西
- `$S(xxx)` = 字符串变量，`$B(xxx)` = 布尔变量
- `<if>` + `<true>` / `<false>` = 条件分支
- `<insert>` = 插入内容，`<copyFile>` = 拷文件，`<log>` = 打日志
- 5 个段都重算 `bAndroidProfilerAPIAvailable`，因为变量不跨段保留

---

## 八、速查表 · 口诀

| 三要素 | 值 |
|---|---|
| 读什么 | `Engine\Source\Runtime\Core\Core_APL.xml`（73 行，XML） |
| 讲义 | `从零复刻ue5\Engine\Source\Runtime\Core\02_Core_APL.xml_完整代码.md` |
| 还原到哪 | `ue_5.6\Engine\Source\Runtime\Core\Core_APL.xml` |

**口诀**：**`Core_APL.xml` 是 Android 打包的「插件说明书」；6 个段各管一处（import / onCreate / proguard / 拷文件 / gradle）；全部条件都是 `CompileSDK >= 35`；变量不跨段，所以每段都重算。**

---

## 九、XML / UPL 语法小抄

| 语法 | 含义 | 例子 |
|---|---|---|
| `<?xml ...?>` | XML 声明 | `<?xml version="1.0" encoding="utf-8"?>` |
| `<!-- ... -->` | XML 注释 | `<!--Core module additions-->` |
| `<tag attr="value"/>` | 自闭合标签 | `<log text="Core UPL init"/>` |
| `<tag>...</tag>` | 成对标签 | `<init>...</init>` |
| `<![CDATA[...]]>` | 原样输出段 | `<![CDATA[ProfilerAccessor.Init(this);]]>` |
| `$S(name)` | UPL 字符串变量 | `$S(CompileSDK)` |
| `$B(name)` | UPL 布尔变量 | `$B(Distribution)` |
| `$I(name)` | UPL 整数变量 | `$I(EngineMajorVersion)` |

---

## 待确认

1. 73 行看懂了吗？重点记：6 个段各管一处 + 全部条件 `CompileSDK >= 35`。
2. 看完说「下一步」，回主线继续 `Public\` 选一个（7 个散头 / `Containers` / `Math` / `HAL` / `Misc`）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core_APL.xml`（73 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\System\UnrealPluginLanguage.cs`（3552 行；UPL 说明 17~26、变量语法 42~45、自动变量 49~61、段清单 343~440、`ProcessPluginNode` 1109、`if` 1234、`log` 1310、`insert` 1721、`copyFile` 1786、`setBoolIsGreaterEqual` 2086、`init` 执行 2696）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Platform\Android\UEDeployAndroid.cs`（`proguardAdditions` 3300、`buildGradleAdditions` 4213、`resourceCopies` 5294、`gameActivityImportAdditions` 6235、`gameActivityOnCreateAdditions` 6245）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（330 行，第 30 行注册本文件）