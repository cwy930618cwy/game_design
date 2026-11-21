# 02 — `Source\LyraGame.Target.cs` —— 总 md

> 依据：`E:\ue5\LyraStarterGame\Source\LyraGame.Target.cs`（**279 行**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraGame.Target.cs`
> **✅ 本节可独立被 UBT 识别**（纯编译配置脚本，不参与运行时）

---

## 一、这个文件解决什么问题（为什么）

`*.Target.cs` 是**编译目标的说明书**。UBT（Unreal Build Tool）编译前先读它，回答三个问题：

1. **编什么** —— `Type = TargetType.Game`，编一个「游戏客户端」目标
2. **编哪些模块** —— `ExtraModuleNames.AddRange(new string[] { "LyraGame" })`
3. **用什么编译设置** —— `BuildSettingsVersion.V5`、`IncludeOrderVersion.Latest`、Shipping 下要不要日志

🔑 **一句话比喻**：`Target.cs` 是「施工图纸的封面」—— 说明这栋楼是住宅还是写字楼（Game / Editor / Server）、包含哪几个单元（模块）、按哪版规范施工（BuildSettings）。

**为什么 Lyra 的 Target.cs 有 279 行这么长**：它把「所有 Lyra 目标共用的设置」抽成了两个静态方法：

- `ApplySharedLyraTargetSettings(TargetRules Target)` —— 4 个目标（Game / Editor / Client / Server）共用
- `ConfigureGameFeaturePlugins(TargetRules Target)` —— 扫描 `Plugins\GameFeatures\` 下的 `.uplugin`，决定哪些玩法插件参与编译

---

## 二、内部结构全景

```
LyraGame.Target.cs
├─ using × 7                                    [3-9]
├─ class LyraGameTarget : TargetRules           [11-279]
│  ├─ LyraGameTarget(TargetInfo)                [13-20]   构造函数
│  │  ├─ Type = TargetType.Game                 [15]
│  │  ├─ ExtraModuleNames.AddRange              [17]
│  │  └─ ApplySharedLyraTargetSettings(this)    [19]
│  ├─ bHasWarnedAboutShared (static bool)       [22]      只警告一次的标志位
│  ├─ ApplySharedLyraTargetSettings(TargetRules)[24-91]   共用设置
│  │  ├─ DefaultBuildSettings = V5              [28]
│  │  ├─ IncludeOrderVersion = Latest           [29]
│  │  ├─ bIsTest / bIsShipping / bIsDedicatedServer [31-33]
│  │  ├─ if (BuildEnvironment == Unique)        [34-69]   独立构建分支
│  │  │  ├─ ShadowVariableWarningLevel = Error  [36]
│  │  │  ├─ bUseLoggingInShipping = true        [38]
│  │  │  ├─ if (Shipping && !Server)            [40-53]   证书校验 + 命令行安全示例
│  │  │  ├─ if (Shipping || Test)               [55-60]   禁读外部 ini
│  │  │  ├─ if (Type != Editor)                 [62-66]   禁用 OpenImageDenoise
│  │  │  └─ ConfigureGameFeaturePlugins         [68]
│  │  └─ else（Shared 分支）                     [70-90]
│  │     ├─ if (Type == Editor) → 配置插件       [77-80]
│  │     └─ else → 只警告一次                    [81-89]
│  ├─ ShouldEnableAllGameFeaturePlugins         [93-112]  是否全编译插件
│  ├─ AllPluginRootJsonObjectsByName (static)   [114]     .uplugin JSON 缓存
│  └─ ConfigureGameFeaturePlugins(TargetRules)  [120-278] 扫描并决定插件开关
│     ├─ 收集 .uplugin 路径                      [128-134]
│     ├─ foreach 每个插件                        [140-273]
│     │  ├─ 读 JSON（带缓存）                    [148-153]
│     │  ├─ 校验 EnabledByDefault                [158-162]
│     │  ├─ 校验 ExplicitlyLoaded                [166-170]
│     │  ├─ 全编译模式 → 全启用                  [179-183]
│     │  ├─ EditorOnly → 非编辑器目标强制禁用     [186-198]
│     │  ├─ RestrictToBranch → 分支不符强制禁用   [201-214]
│     │  ├─ NeverBuild → 强制禁用                [217-223]
│     │  ├─ 记录插件引用关系                     [226-246]
│     │  ├─ catch 解析异常 → 强制禁用            [248-252]
│     │  └─ 输出决定 → EnablePlugins/DisablePlugins [255-271]
│     └─ 预留：发布版本引用校验                   [275-276]
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|---|---|---|---|
| `LyraGame.Target.cs` | ❌ 无（只依赖 UBT 库） | ✅ UBT 直接识别 | ❌ 不需要 |

**关键依赖表**

| 项 | 作用 |
|---|---|
| `TargetRules` / `TargetInfo` / `TargetType` | UBT 基类与枚举，来自 `UnrealBuildTool` |
| `EpicGames.Core` | 提供 `JsonObject` / `FileReference` / `DirectoryReference` |
| `UnrealBuildBase` | 提供 `Unreal.GetExtensionDirs` / `PluginsBase.EnumeratePlugins` |
| `Microsoft.Extensions.Logging` | 提供 `ILogger`（`LogWarning` / `LogDebug`） |
| `LyraEditor.Target.cs` | 会调用本文件的 `ApplySharedLyraTargetSettings`（第 3 轮） |

> ⚠️ **本文件没有渐进式偏差**，一次到位，与 Lyra 完全一致。

---

## 四、怎么教（推进节奏）

本轮三件套：

| # | 产物 | 路径 |
|---|---|---|
| ① | 总 md（本文件） | `S00工程骨架\02_LyraGame.Target.cs_总md.md` |
| ② | 复制文件（代码 + 每行上方中文注释） | `S00工程骨架\02_LyraGame.Target.cs复制文件.md` |
| ③ | 还原文件 | `Source\LyraGame.Target.cs` |

---

**下一步**：第 3 轮 `Source\LyraEditor.Target.cs`（24 行）。