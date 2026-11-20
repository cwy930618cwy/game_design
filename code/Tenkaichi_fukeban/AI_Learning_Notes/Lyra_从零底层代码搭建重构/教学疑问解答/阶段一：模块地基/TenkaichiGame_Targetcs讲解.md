# TenkaichiGame.Target.cs 讲解（283 行完整还原）

> **所属**：阶段一（L1 模块地基）第 1 步。答疑记录。
> **对应 Lyra**：`Source/LyraGame.Target.cs`（283 行）。
> **核对结论**：你的 `TenkaichiGame.Target.cs` 已 283 行完整还原，与 Lyra 源码逐行一致（仅换前缀），✅ 正确。

---

## 一、这个文件是干嘛的（先讲为什么）

`Target.cs` 是 **「编译目标」的定义文件**，它回答一个问题：

> 「我要编译出一个什么样的程序？编译时开哪些开关、启用哪些插件？」

`TenkaichiGame.Target.cs` 是**最核心、最复杂的一个 Target**，因为它不只是一个目标，还**承载了所有其他目标（Editor/Client/Server 及各种变体）共享的公共设置**。

关键设计：Lyra 把「一大堆通用的编译设置」抽成了一个方法 `ApplySharedTenkaichiTargetSettings`，让 Game / Editor / Client / Server 四个目标都调用它，避免复制粘贴 280 行代码。

---

## 二、文件结构全景（4 个成员）

| 成员 | 类型 | 作用 | 行号 |
|------|------|------|------|
| `TenkaichiGameTarget(...)` | 构造函数 | 定义「这是 Game 目标」，挂 `TenkaichiGame` 模块，调共享设置 | 13~20 |
| `ApplySharedTenkaichiTargetSettings` | 静态方法 | **共享编译设置**，别的 Target 都调它 | 24~95 |
| `ShouldEnableAllGameFeaturePlugins` | 静态方法 | 判断「要不要全部启用 GameFeature 插件」 | 97~116 |
| `ConfigureGameFeaturePlugins` | 静态方法 | 扫描 `Plugins/GameFeatures/`，逐个决定启用/禁用 | 124~282 |

---

## 三、逐段讲解

### 3.1 构造函数（13~20 行）

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

- `Type = TargetType.Game`：这个目标是「游戏」类型（单机/本地运行，逻辑+渲染都在本地）。
- `ExtraModuleNames.AddRange(...)`：编译这个目标时，把 `TenkaichiGame` 模块编进去。
- `ApplySharedTenkaichiTargetSettings(this)`：把共享设置应用到「当前这个目标」上。

### 3.2 共享设置方法（24~95 行）

```csharp
internal static void ApplySharedTenkaichiTargetSettings(TargetRules Target)
{
	ILogger Logger = Target.Logger;
	Target.DefaultBuildSettings = BuildSettingsVersion.V5;
	Target.IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

	bool bIsTest = Target.Configuration == UnrealTargetConfiguration.Test;
	bool bIsShipping = Target.Configuration == UnrealTargetConfiguration.Shipping;
	bool bIsDedicatedServer = Target.Type == TargetType.Server;
```

先算三个布尔量，用来区分「当前是测试/发布/专用服务器」：

- `bIsTest`：是不是 Test 配置。
- `bIsShipping`：是不是 Shipping（发布）配置。
- `bIsDedicatedServer`：是不是专用服务器目标。

然后进入一个**大分支**：

```csharp
if (Target.BuildEnvironment == TargetBuildEnvironment.Unique)
{
	... 一堆精细设置 ...
	TenkaichiGameTarget.ConfigureGameFeaturePlugins(Target);
}
else
{
	... 共享/安装引擎分支，只能部分配置 ...
}
```

**为什么要分这个支？**

- `TargetBuildEnvironment.Unique`：**每个目标独立编译**（源码版引擎，能随便改插件和选项）。
- 非 Unique（共享/安装引擎）：**复用引擎预编译二进制**，不能随便启停插件、改选项（否则要重编引擎）。

> 一句话：源码版引擎走 `Unique` 分支（能完全控制），安装版引擎走 `else` 分支（受限制）。

#### Unique 分支里的关键设置（挑几个重要的讲，其余后面用到再展开）

| 代码 | 作用 |
|------|------|
| `ShadowVariableWarningLevel = WarningLevel.Error` | 变量遮蔽（shadowing）警告升级为错误，逼你改掉隐患 |
| `bUseLoggingInShipping = true` | 发布版也保留日志（方便线上排查） |
| `bDisableUnverifiedCertificates = true` | 发布版强制校验 HTTPS 证书（安全） |
| `bAllowGeneratedIniWhenCooked = false` | 发布/测试时禁止读运行时生成的 ini（锁定配置） |
| `DisablePlugins.Add("OpenImageDenoise")` | 非编辑器禁掉光追降噪插件（DLL 太大，运行时用不到） |
| `UE_ASSETREGISTRY_INDIRECT_ASSETDATA_POINTERS=1` | 省内存（用间接指针存资产数据） |

> 这些开关具体为什么这么设、影响什么，**等后面阶段真正打到包、做联网、上 GameFeature 时再逐个展开**（铁律 27：先写全，详细后置）。

### 3.3 `ShouldEnableAllGameFeaturePlugins`（97~116 行）

```csharp
static public bool ShouldEnableAllGameFeaturePlugins(TargetRules Target)
{
	if (Target.Type == TargetType.Editor)
	{
		// return true;  ← 默认注释掉
	}
	bool bIsBuildMachine = (Environment.GetEnvironmentVariable("IsBuildMachine") == "1");
	if (bIsBuildMachine)
	{
		// return true;  ← 默认注释掉
	}
	return false;
}
```

**作用**：决定「要不要把**所有** GameFeature 插件都编译进来」。

- 默认返回 `false`（走编辑器里插件浏览器手动勾选的规则）。
- 两个 `return true` 都被注释掉，是留给「编辑器全编」或「构建机全编」的开关。

### 3.4 `ConfigureGameFeaturePlugins`（124~282 行，最长的部分）

**作用**：扫描 `Plugins/GameFeatures/` 目录下的所有 GameFeature 插件（`.uplugin` 文件），逐个读它们的描述，按规则决定「启用还是禁用」。

流程：

1. `Unreal.GetExtensionDirs(...)` 找到 GameFeature 插件根目录。
2. `PluginsBase.EnumeratePlugins(...)` 枚举所有 `.uplugin`。
3. 对每个插件，读它的 `.uplugin` JSON，检查这些字段：
   - `EditorOnly` → 非编辑器目标禁用
   - `RestrictToBranch` → 限定分支，不匹配就禁用
   - `NeverBuild` → 永不编译
   - `ExplicitlyLoaded` → 必须是 true（GameFeature 要求启动后手动加载）
4. 最后把结果写进 `Target.EnablePlugins` / `Target.DisablePlugins`。

> 这是 GameFeature（Lyra 的核心玩法模块化机制）的**编译期自动配置**。详细原理等阶段七（业务层/GameFeatures）再深入，本步先知道「它在扫描并决定哪些玩法插件要编译」即可。

---

## 四、总结

`TenkaichiGame.Target.cs` 干两件事：

1. **定义自己**：我是 Game 目标，编 `TenkaichiGame` 模块。
2. **当公共工具箱**：提供 `ApplySharedTenkaichiTargetSettings`（共享编译设置）+ `ConfigureGameFeaturePlugins`（GameFeature 插件自动配置），供 Editor/Client/Server 及所有变体复用。

所以它是 10 个 Target 里的**基类 + 公共配置中心**，必须最先补、且一字不差。

---

## 五、核对结果

✅ 你的 283 行已完整还原，与 Lyra 逐行一致（仅 `Lyra`→`Tenkaichi` 前缀替换 + 日志字符串 `"LyraGameEOS"`→`"TenkaichiGameEOS"`），无需再改。