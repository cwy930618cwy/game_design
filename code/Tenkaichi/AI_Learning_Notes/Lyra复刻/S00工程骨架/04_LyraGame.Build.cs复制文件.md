# 04 — `LyraGame.Build.cs` 复制文件

> 依据：`E:\ue5\game_design\code\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs`（**106 行**，逐字一比一）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraGame.Build.cs`
> **用法**：照着下面代码逐行敲。**注释在每行上方，敲的时候整行 `//` 注释都不要抄**。
> ⚠️ 本文件已按源全文重写（旧"精简版"作废）。代码行与源逐字一致（92 行代码）。

---

```csharp
// 版权声明：Epic Games 保留所有权利

// 引入 UnrealBuildTool 命名空间；ModuleRules / ReadOnlyTargetRules / PCHUsageMode 都在这里
using UnrealBuildTool;

// 定义 LyraGame 模块规则类；类名必须与 .uproject 中 Modules 的 Name 一致
public class LyraGame : ModuleRules
{
	// 构造函数；UBT 编译本模块时调用，Target 携带平台 / 配置 / 是否编辑器
	public LyraGame(ReadOnlyTargetRules Target) : base(Target)
	{
		// PCH 使用模式：优先共享 PCH，没有则用显式 PCH；UE5 默认值，加快编译
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 公共头文件搜索路径：把模块根目录加入，其他模块可省略前缀写 #include
		PublicIncludePaths.AddRange(
			// 字符串数组开始
			new string[] {
				// 模块根目录名；对应 Source\LyraGame\
				"LyraGame"
			// 数组结束
			}
		// 添加公共头文件路径结束
		);

		// 私有头文件搜索路径：本模块内部专用，不对外暴露；S00 与 Lyra 一致为空
		PrivateIncludePaths.AddRange(
			// 字符串数组开始（空）
			new string[] {
			// 数组结束
			}
		// 添加私有头文件路径结束
		);

		// 公共依赖模块：本模块公共头文件里 include 了它们的头文件；依赖会向下传染
		PublicDependencyModuleNames.AddRange(
			// 字符串数组开始
			new string[] {
				// 最底层：FString / TArray / 容器 / 数学库
				"Core",
				// 在线子系统公共接口
				"CoreOnline",
				// UObject 系统：UCLASS / UPROPERTY / 反射 / GC
				"CoreUObject",
				// 应用核心层：平台抽象 / 时间
				"ApplicationCore",
				// 引擎主体：AActor / UActorComponent / UWorld / 渲染 / 物理
				"Engine",
				// 物理核心：碰撞 / 物理材质
				"PhysicsCore",
				// GameplayTag 系统：FGameplayTag / FGameplayTagContainer 用于标签匹配
				"GameplayTags",
				// GameplayTask：UAbilityTask 的基类
				"GameplayTasks",
				// GAS 主体：ASC / GameplayAbility / GameplayEffect
				"GameplayAbilities",
				// AI 模块：导航 / 感知
				"AIModule",
				// 模块化玩法基础：组件式 Pawn / GameMode 扩展点
				"ModularGameplay",
				// 模块化玩法 Actor：AModularPawn / AModularCharacter 等
				"ModularGameplayActors",
				// 数据注册表：DataRegistry 资产
				"DataRegistry",
				// 复制图：网络复制优化
				"ReplicationGraph",
				// GameFeature：玩法模块化加载
				"GameFeatures",
				// 重要性管理：网络/性能裁剪
				"SignificanceManager",
				// 热修复：运行时选项 / 文本热更
				"Hotfix",
				// 通用加载界面：ILoadingProcessInterface
				"CommonLoadingScreen",
				// Niagara 特效系统
				"Niagara",
				// 异步 Mixin：AsyncTask 的组合工具
				"AsyncMixin",
				// 控制流：ControlFlows 节点
				"ControlFlows",
				// 属性路径：FPropertyPath 反射访问
				"PropertyPath"
			// 数组结束
			}
		// 添加公共依赖结束
		);

		// 私有依赖模块：只在本模块 .cpp 里用到，公共头文件不暴露
		PrivateDependencyModuleNames.AddRange(
			// 字符串数组开始
			new string[] {
				// 输入核心：FKey / EKeys
				"InputCore",
				// Slate UI 基础
				"Slate",
				// Slate 核心
				"SlateCore",
				// 渲染核心：渲染资源
				"RenderCore",
				// 开发者设置：UDeveloperSettings 基类
				"DeveloperSettings",
				// 增强输入：UInputAction / UInputMappingContext
				"EnhancedInput",
				// 网络核心：联机底层
				"NetCore",
				// 渲染硬件接口
				"RHI",
				// 工程与插件管理：FProjectManager
				"Projects",
				// Gauntlet 自动化测试框架
				"Gauntlet",
				// UMG：UUserWidget / 控件
				"UMG",
				// CommonUI 控件层
				"CommonUI",
				// CommonInput：通用输入抽象
				"CommonInput",
				// 设置系统插件
				"GameSettings",
				// 通用游戏层：GameInstance / UI 管理
				"CommonGame",
				// 通用用户系统：登录 / 会话
				"CommonUser",
				// 字幕系统
				"GameSubtitles",
				// 消息路由运行时：GameplayMessageSubsystem
				"GameplayMessageRuntime",
				// 音频混音
				"AudioMixer",
				// 网络回放流：录像 / 回放
				"NetworkReplayStreaming",
				// UI 扩展点：UIExtensionPointWidget
				"UIExtension",
				// 客户端试验框架
				"ClientPilot",
				// 音频调制
				"AudioModulation",
				// 引擎设置模块
				"EngineSettings",
				// DTLS 加密组件
				"DTLSHandlerComponent",
				// JSON 解析
				"Json",
			// 数组结束（注意 Json 后保留源文件的尾逗号，不要删）
			}
		// 添加私有依赖结束
		);

		// 动态加载模块：运行时通过 FModuleManager 加载；S00 与 Lyra 一致为空
		DynamicallyLoadedModuleNames.AddRange(
			// 字符串数组开始（空）
			new string[] {
			// 数组结束
			}
		// 添加动态加载模块结束
		);

		// 若测试 / 发行版里出现 DrawDebug 调用，直接编译报错，防止调试绘制漏进包
		PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");

		// 外部 RPC 框架基础配置；框架功能在 Shipping 里会被剥离以减小漏洞面
		// 注册外部 RPC 框架模块
		PrivateDependencyModuleNames.Add("ExternalRpcRegistry");
		// HTTPServer 是 ExternalRpcRegistry 的依赖
		PrivateDependencyModuleNames.Add("HTTPServer");
		// 如果是 Shipping 配置
		if (Target.Configuration == UnrealTargetConfiguration.Shipping)
		{
			// 发行版关闭外部 RPC 注册表
			PublicDefinitions.Add("WITH_RPC_REGISTRY=0");
			// 发行版关闭 HTTP 监听
			PublicDefinitions.Add("WITH_HTTPSERVER_LISTENERS=0");
		}
		// 非 Shipping 配置
		else
		{
			// 开发版开启外部 RPC 注册表
			PublicDefinitions.Add("WITH_RPC_REGISTRY=1");
			// 开发版开启 HTTP 监听
			PublicDefinitions.Add("WITH_HTTPSERVER_LISTENERS=1");
		}

		// 启用 GameplayDebugger 支持；编辑器里按 ' 键打开 GAS 调试面板
		SetupGameplayDebuggerSupport(Target);
		// 启用 Iris 网络复制支持；UE5 新一代复制系统
		SetupIrisSupport(Target);
	// 构造函数结束
	}
// 类结束
}
```

## 敲完自查

| 检查项 | 通过标准 |
|---|---|
| 文件路径 | `Source\LyraGame\LyraGame.Build.cs`（不是 `Source\Tenkaichi\`） |
| 类名 | `LyraGame`，与模块名一致 |
| 公共依赖 | **22 个**，与源文件 24-45 行逐字一致（不是 6 个！） |
| 私有依赖 | **26 个**，与源文件 51-76 行逐字一致；`"Json",` 的尾逗号保留 |
| ExternalRpc | 有 `ExternalRpcRegistry` / `HTTPServer` / `if Shipping` 整段（源 88-101） |
| 宏定义 | `PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");` |
| 两个 Setup | `SetupGameplayDebuggerSupport` + `SetupIrisSupport` |
| 注释 | 全部删掉 `//` 开头的行；**零英文注释残留** |
| 缩进 | Tab（不是空格） |
| 代码行 | 剔注释后 **92 行**，与源逐字一致 |
| 旧模块 | 删掉 `Source\Tenkaichi\` 整个目录（`.uproject` 只声明 `LyraGame`） |

## 源码依据

- `E:\ue5\game_design\code\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs:1-106`（源全文；文档里写的 `E:\ue5\LyraStarterGame` 不存在，这是实查到的 Lyra 5.4 源根）
- 第 9 行：`PCHUsage`
- 第 11-15 行：`PublicIncludePaths`
- 第 22-47 行：22 个公共依赖
- 第 49-78 行：26 个私有依赖
- 第 86 行：`SHIPPING_DRAW_DEBUG_ERROR`
- 第 88-101 行：外部 RPC + Shipping 分支
- 第 103-104 行：两个 Setup 调用
