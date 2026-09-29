# Lyra 为什么分成 LyraGame 和 LyraEditor 两个模块

> **定位**：讲清你新拉的 Lyra 5.6 源码里，`Source/` 下为什么有**两个 C++ 模块**——`LyraGame`（游戏本体）和 `LyraEditor`（编辑器扩展）。这是理解 UE 项目"模块分层"的第一课。
>
> **一句话**：`LyraGame` 是**纯游戏运行时代码**（打进最终玩家包），`LyraEditor` 是**只在编辑器里跑的辅助代码**（资产校验、菜单扩展、命令行工具）。**分开是为了让正式包不带编辑器垃圾、编译更快、职责更清。**

---

## 一、先看真实的目录结构

```
Source/
├── LyraGame/                 ← 【核心】游戏运行时代码（450 个文件）
│   ├── Character/  AbilitySystem/  Weapons/  Inventory/ ...
│   ├── LyraGame.Build.cs
│   └── LyraGameModule.cpp
│
├── LyraEditor/               ← 【辅助】只在编辑器里跑的代码（26 个文件）
│   ├── Commandlets/  Private/  Utilities/  Validation/
│   ├── LyraEditor.Build.cs
│   └── LyraEditor.cpp / LyraEditorEngine.cpp
│
├── LyraClient.Target.cs      ← 客户端打包目标
├── LyraEditor.Target.cs      ← 编辑器目标
├── LyraGame.Target.cs        ← Game 打包目标
├── LyraServer.Target.cs      ← 服务器打包目标
└── ...（EOS/Steam 变体）
```

> 关键数字对比：`LyraGame` ≈ **450 文件**（真正的游戏逻辑），`LyraEditor` ≈ **26 文件**（少量编辑器辅助）。**主体永远在 LyraGame。**

---

## 二、本质区别：一个能进包，一个只能编辑器里跑

| | **LyraGame** | **LyraEditor** |
|---|---|---|
| 角色 | 游戏本体（玩法全在这） | 编辑器扩展工具 |
| 能否打进玩家包 | ✅ 能（Shipping 发布） | ❌ 不能（发布时被剥离） |
| 依赖的引擎模块 | 运行时模块（Engine/Core/GAS...） | **额外依赖编辑器模块**（UnrealEd/EditorFramework...） |
| 典型内容 | 角色、武器、技能、UI、输入 | 资产校验、菜单、命令行工具(Commandlet) |
| 谁用 | 玩家 + 开发者 | 只有开发者（在编辑器里） |

### 看 Build.cs 的证据

**LyraEditor.Build.cs** 依赖了一堆"**只有编辑器才有**"的模块：

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "EditorFramework",   // ← 编辑器框架，打包时不存在
    "UnrealEd",          // ← 编辑器核心，打包时不存在
    "GameplayTagsEditor",// ← 编辑器专用
    "GameplayAbilitiesEditor", // ← 编辑器专用
    ...
    "LyraGame",          // ← 注意：它依赖 LyraGame！
});
```

**LyraGame.Build.cs** 全是**运行时模块**，没有任何 `*Editor` 模块：

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core", "Engine", "GameplayAbilities", "ModularGameplay",
    "EnhancedInput", "AIModule", "ReplicationGraph", ...
    // 没有一个带 "Editor" 后缀的
});
```

> **判断一个模块是不是"编辑器专用"的最简单方法**：看它的 `.Build.cs` 里有没有依赖 `UnrealEd` / `EditorFramework` / `*Editor` 这类模块。有 = 编辑器专用。

---

## 三、为什么要分开（4 个核心理由）

### ① 正式包不带编辑器垃圾

`UnrealEd`、`EditorFramework` 这些模块**体积大、只在编辑器里有**。如果把它们塞进游戏本体，玩家下载的安装包会白白变大几十上百 MB，而且这些代码在玩家机器上根本跑不起来（没有编辑器环境）。

**分开后**：打包玩家版（Game/Client/Server Target）时，`LyraEditor` 模块**根本不参与编译**，干干净净。

### ② 编译更快

编辑器代码和游戏代码解耦：
- 改 UI 菜单 → 只重编 `LyraEditor`（26 文件，秒级）
- 不用每次都把整个项目重新编译一遍

### ③ 职责清晰，防止误用

- `LyraGame` = 会出现在玩家手里的逻辑 → 写这里要谨慎
- `LyraEditor` = 开发期辅助 → 随便折腾不影响玩家

把"资产校验、命令行工具"这类东西放到独立模块，也避免它们污染游戏主逻辑。

### ④ 符合 UE 官方范式

UE 官方所有示例项目（包括引擎自带）都是这个套路：**一个 Runtime 模块 + 一个 Editor 模块**。Lyra 只是遵循了这个约定。

---

## 四、Target.cs 是怎么把它们组合起来的（关键机制）

"模块"是代码单位，"Target"是**打包目标**——决定"这次编译要带上哪些模块"。看真实源码：

### LyraEditor.Target.cs（编辑器目标）

```csharp
Type = TargetType.Editor;
ExtraModuleNames.AddRange(new string[] { "LyraGame", "LyraEditor" });
//                          ↑ 编辑器目标：两个都要！因为编辑器里既要跑游戏逻辑，也要跑编辑器工具
```

### LyraGame.Target.cs（Game 打包目标）

```csharp
Type = TargetType.Game;
ExtraModuleNames.AddRange(new string[] { "LyraGame" });
//                          ↑ 只带 LyraGame！LyraEditor 不进包
```

### 总结成一张表

| Target（打包目标） | 包含的模块 | 用途 |
|---|---|---|
| `LyraEditor` | **LyraGame + LyraEditor** | 在编辑器里开发/调试 |
| `LyraGame` | 只有 **LyraGame** | 打成玩家版（Windows/Mac 安装包） |
| `LyraClient` | 只有 **LyraGame** | 打成客户端 |
| `LyraServer` | 只有 **LyraGame** | 打成专用服务器(DS) |

> **记忆口诀**：**编辑器目标带两个，打包目标只带 LyraGame。**

---

## 五、LyraEditor 模块里具体放了什么

看它的目录结构，都是"开发期工具"：

```
LyraEditor/
├── Commandlets/      ← 命令行工具（不用开编辑器就能跑的批处理，如生成数据）
├── Private/          ← 编辑器相关的私有实现
├── Utilities/        ← 编辑器小工具
└── Validation/       ← 资产校验（检查美术资源合不合规，提交前自动查错）
```

**典型场景**：
- 美术提交资产前，`Validation/` 自动检查"这个材质命名对不对、贴图尺寸合不合规"
- `Commandlets/` 提供一键生成配置数据的命令（CI 流水线用）
- 在编辑器菜单里加一些 Lyra 专用的快捷按钮

**这些东西玩家永远看不到，也不该打进包里。**

---

## 六、对你学习的影响（重要）

| 你会遇到的情况 | 该怎么做 |
|---|---|
| 学玩法、改角色/武器/技能 | 只碰 **LyraGame/** |
| 看到 `LyraEditor/` 的代码 | 初学阶段**可以先跳过**，它是编辑器工具 |
| 想给编辑器加个菜单/校验 | 才需要动 **LyraEditor/** |
| 编译很慢 | 确认你是不是误改了导致全量重编 |

> **学习建议**：前期 95% 的时间你都在 `LyraGame/` 里转。`LyraEditor/` 等你项目做大了、需要自动校验资产或写编辑器工具时再回头看。

---

## 七、常见误区

| 误区 | 正确理解 |
|------|---------|
| "两个模块功能重复了" | 不重复：一个是游戏本体，一个是编辑器工具 |
| "LyraEditor 会被打进玩家包" | ❌ 打包目标不含它，进不了包 |
| "我应该把工具代码写进 LyraGame" | ❌ 工具放 LyraEditor，保持 LyraGame 干净 |
| "LyraGame 依赖 LyraEditor" | ❌ 反过来：LyraEditor 依赖 LyraGame |
| "多一个模块会拖慢游戏" | 不会，打包时它根本不参与 |

---

## 八、总结速查

```
Lyra 分两个模块：
  LyraGame    = 游戏本体（450 文件，纯运行时，打进玩家包）
  LyraEditor  = 编辑器工具（26 文件，依赖 UnrealEd，只在编辑器跑）

关系：LyraEditor 依赖 LyraGame，反之不行

Target 组合：
  LyraEditor 目标 → LyraGame + LyraEditor
  LyraGame/Client/Server 目标 → 只有 LyraGame

记住：编辑器目标带两个，打包目标只带 LyraGame
```

**一句话**：`LyraGame` 是会出现在玩家手里的**游戏本体**，`LyraEditor` 是只在开发者编辑器里跑的**辅助工具**（资产校验/菜单/命令行）。分开是为了**正式包更干净、编译更快、职责更清**——这也是 UE 官方的标准范式。

---

## 九、下一步

- 深入 `LyraGame/` 内部结构（Character/AbilitySystem/Weapons 等子系统）
- 理解 `Target.cs` 里的 `ConfigureGameFeaturePlugins`（GameFeature 插件如何按目标启用）
- 了解 Runtime 模块 vs Editor 模块的边界最佳实践
