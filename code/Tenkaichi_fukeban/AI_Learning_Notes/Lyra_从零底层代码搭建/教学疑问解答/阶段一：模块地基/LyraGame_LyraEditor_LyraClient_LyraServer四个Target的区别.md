# LyraGame / LyraEditor / LyraClient / LyraServer 四个 Target 的区别

> **所属**：阶段一（L1 模块地基）第 1 步。答疑记录。
> **问题**：这 4 个 Target 文件到底有什么区别？为什么同一个项目要建 4 个目标？

---

## 一、先讲概念：Target 是什么

UE 的「Target」= **一次编译/打包的产物定义**。一个 `.Target.cs` 文件描述「我要编译出一个什么样的可执行程序」。

同一个项目（同一个 `.uproject`），可以有多个 Target，因为它们对应**不同的运行形态**：

| 运行形态 | 通俗理解 |
|---------|---------|
| 编辑器 | 你在 UE 编辑器里点播放、改资产的那个程序 |
| 游戏客户端 | 玩家双击启动的那个游戏程序 |
| 专用服务器 | 部署在服务器机房、没人看画面、只算逻辑的那个程序 |
| 瘦客户端 | 只渲染画面、逻辑都听服务器的那个程序（联网模式） |

一句话：**Target 决定了「这个程序是拿来干嘛的」。**

---

## 二、四个 Target 的核心区别（源码依据）

| Target | `Type` | 加载的模块 | 关键特征 |
|--------|--------|-----------|---------|
| `LyraGame` | `TargetType.Game` | `LyraGame` | 单机/本地运行的游戏程序，**同时含逻辑 + 渲染** |
| `LyraEditor` | `TargetType.Editor` | `LyraGame` + `LyraEditor` | 编辑器程序，**额外挂 `LyraEditor` 模块** |
| `LyraClient` | `TargetType.Client` | `LyraGame` | 联网的**瘦客户端**（只渲染，逻辑在服务器） |
| `LyraServer` | `TargetType.Server` | `LyraGame` | **专用服务器**（不渲染，只算逻辑） |

### 逐个拆解（贴 Lyra 真实源码）

#### 1. `LyraGame.Target.cs`（283 行，最核心）

```csharp
public class LyraGameTarget : TargetRules
{
	public LyraGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.AddRange(new string[] { "LyraGame" });
		LyraGameTarget.ApplySharedLyraTargetSettings(this);
	}
	...
}
```

- `Type = TargetType.Game`：标准游戏程序。
- 它**不只有上面几行**——后面还有 200 多行的 `ApplySharedLyraTargetSettings`（共享设置）和 `ConfigureGameFeaturePlugins`（GameFeature 插件配置）。
- **关键点**：`LyraGameTarget` 是**基类**，另外 3 个 Target 都调用了它的 `ApplySharedLyraTargetSettings(this)`，共享同一套编译设置。

#### 2. `LyraEditor.Target.cs`（24 行）

```csharp
public class LyraEditorTarget : TargetRules
{
	public LyraEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.AddRange(new string[] { "LyraGame", "LyraEditor" });
		if (!bBuildAllModules)
		{
			NativePointerMemberBehaviorOverride = PointerMemberBehavior.Disallow;
		}
		LyraGameTarget.ApplySharedLyraTargetSettings(this);
		EnablePlugins.Add("RemoteSession");
	}
}
```

区别点：
- `Type = TargetType.Editor`：编辑器。
- **多挂了一个 `LyraEditor` 模块**（编辑器专用模块，如编辑器 UI、工具）。
- 多开了 `RemoteSession` 插件（配合「Unreal Remote 2」手机触屏开发）。
- 多一个 `NativePointerMemberBehaviorOverride` 设置（非全模块构建时禁用指针成员行为，编辑器反射安全相关）。

#### 3. `LyraClient.Target.cs`（16 行）

```csharp
public class LyraClientTarget : TargetRules
{
	public LyraClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;
		ExtraModuleNames.AddRange(new string[] { "LyraGame" });
		LyraGameTarget.ApplySharedLyraTargetSettings(this);
	}
}
```

- `Type = TargetType.Client`：**瘦客户端**。联网游戏里，客户端只负责「收服务器数据 → 渲染画面」，**不做游戏逻辑裁决**。

#### 4. `LyraServer.Target.cs`（19 行）

```csharp
[SupportedPlatforms(UnrealPlatformClass.Server)]
public class LyraServerTarget : TargetRules
{
	public LyraServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		ExtraModuleNames.AddRange(new string[] { "LyraGame" });
		LyraGameTarget.ApplySharedLyraTargetSettings(this);
		bUseChecksInShipping = true;
	}
}
```

- `Type = TargetType.Server`：**专用服务器**。**不渲染画面**，只跑游戏逻辑（权威服务器）。
- `[SupportedPlatforms(UnrealPlatformClass.Server)]`：只支持服务器平台（Windows/Linux 服务器），不能编成普通桌面程序。
- `bUseChecksInShipping = true`：即便 Shipping 发布版也保留 check 断言（服务器要稳，宁可崩也要及时暴露逻辑错误，而不是带病运行）。

---

## 三、一句话总结区别

| 目标 | 一句话 |
|------|--------|
| **Game** | 单机游戏（逻辑 + 渲染都在本地） |
| **Editor** | 编辑器（开发用，多挂编辑器模块和插件） |
| **Client** | 联网瘦客户端（只渲染，逻辑听服务器） |
| **Server** | 专用服务器（只算逻辑，不渲染，最稳） |

---

## 四、为什么都要建？因为 Lyra 是联机游戏

Lyra 是**多人在线**游戏，需要「客户端 + 服务器」分离的架构：

```
玩家电脑 ──(瘦客户端 LyraClient)──┐
                                  ├──→ 权威服务器 (LyraServer)
另一台玩家电脑 ──(瘦客户端)───────┘      ↑ 只算逻辑、不渲染
```

- 单机调试时用 `LyraGame`（本地同时跑逻辑 + 画面）。
- 开发时用 `LyraEditor`（在编辑器里点播放）。
- 正式联机时：玩家用 `Client`，机房用 `Server`。

---

## 五、⚠️ 重要提醒：1.5 的教学代码是简化版，不符合铁律 23

对照 Lyra 真实源码后发现：

- **教学 md 1.5 里给的 `TenkaichiGame.Target.cs` 只有 15 行**（`Type` + `DefaultBuildSettings` + `IncludeOrderVersion` + `ExtraModuleNames`）。
- **Lyra 真实 `LyraGame.Target.cs` 是 283 行**，包含：
  1. `ApplySharedLyraTargetSettings`（共享编译设置，约 74 行）
  2. `ShouldEnableAllGameFeaturePlugins`（是否启用全部 GameFeature 插件）
  3. `ConfigureGameFeaturePlugins`（GameFeature 插件自动配置，约 160 行）

按铁律 23「一比一完整还原、不做任何简化」，这 283 行**都应该完整还原**成 `TenkaichiGame.Target.cs`，而不是 15 行的简化版。

> 这属于之前教学里「做了减法」的遗留问题。**需要单独处理**——建议下一步专门补一个 md，把 `TenkaichiGame.Target.cs` 的 283 行完整内容一比一还原讲解。