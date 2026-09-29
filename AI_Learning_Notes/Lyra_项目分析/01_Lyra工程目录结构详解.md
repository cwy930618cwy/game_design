# 01 — Lyra 工程目录结构详解

> **定位**：分析 Lyra 实际工程（`LyraStarterGame`）的目录结构，说明每个文件夹是干嘛的。
>
> **一句话**：Lyra 工程分 **Source（C++ 源码）、Content（资产）、Plugins（插件）、Config（配置）** 几大部分。Source 是代码核心，Content 是资源，Plugins 是扩展。
>
> **文件**：`e:\code\lyra_fifty_six\LyraStarterGame/`（Lyra 实际工程）

---

## 一、LyraStarterGame 顶层目录（先看全貌）

```
LyraStarterGame/
├── Source/           ← ★ C++ 源码（代码核心）
├── Content/          ← ★ 资产（蓝图/材质/地图）
├── Plugins/          ← ★ 插件（功能扩展）
├── Config/           ← 配置文件（ini）
├── Build/            ← 构建脚本
├── Binaries/         ← 编译产物（.dll/.exe）
├── Intermediate/     ← 编译中间文件（自动生成）
├── Saved/            ← 运行时临时文件（日志/缓存）
├── DerivedDataCache/ ← 派生数据缓存（DDC）
└── Platforms/        ← 平台特定文件
```

---

## 二、重点目录逐个说明

### 1. Source/ —— C++ 源码（最重要）

**这是代码核心**。我实际看了结构：

```
Source/
├── LyraGame/        ← ★ 游戏运行时代码（最大，450 文件）
│   ├── 228 个 .h
│   └── 219 个 .cpp
├── LyraEditor/      ← 编辑器工具代码（26 文件）
├── LyraGame.Target.cs      ← 目标配置
├── LyraClient.Target.cs    ← 客户端目标
├── LyraServer.Target.cs    ← 服务器目标
├── LyraEditor.Target.cs    ← 编辑器目标
├── LyraGameEOS.Target.cs   ← EOS 联机版
├── LyraGameSteam.Target.cs ← Steam 版
└── ...（多个平台版本）
```

| 目录/文件 | 干嘛 |
|----------|------|
| `LyraGame/` | **游戏运行时** C++ 代码（Experience/GAS/角色等全在这） |
| `LyraEditor/` | **编辑器工具**代码（验证器、自定义编辑器） |
| `*.Target.cs` | **构建目标**（客户端/服务器/编辑器/Steam/EOS 等） |

### 2. Content/ —— 资产（蓝图/材质/地图）

**所有游戏资源**都在这里：蓝图、材质、贴图、地图、数据资产。

```
Content/
├── 蓝图（角色、UI、GameplayAbility）
├── 材质 / 贴图
├── 地图（Levels）
├── 数据资产（Experience、InputAction）
└── 各种资源
```

> 编辑器里看到的 Content Browser，对应的就是这个文件夹。

### 3. Plugins/ —— 插件（功能扩展）

**Lyra 大量用插件**（GameFeature 拆玩法）。这是 Lyra 和普通项目最大的不同。

```
Plugins/
├── GameFeatures/     ← 玩法插件（把玩法拆成模块）
├── CommonUI/         ← UI 框架
├── GameplayMessageRouter ← 消息路由
├── AsyncMixin        ← 异步工具
└── ...（各种插件）
```

### 4. Config/ —— 配置文件

```
Config/
├── DefaultEngine.ini    ← 引擎配置
├── DefaultGame.ini      ← 游戏配置
├── DefaultInput.ini     ← 输入配置
└── ...（各种 ini）
```

### 5. Build/ —— 构建脚本

打包、构建用的脚本文件。

### 6. Binaries/ —— 编译产物

编译出来的 `.dll` / `.exe`。**不需要看，是生成的。**

### 7. Intermediate/ —— 编译中间文件

编译过程的临时文件（UHT 生成的 .generated.h 也在这）。**自动生成，不用管。**

### 8. Saved/ —— 运行时临时文件

运行时的日志、崩溃报告、缓存。**看日志从这里看（Logs 文件夹）。**

### 9. DerivedDataCache/ —— 派生数据缓存

DDC，缓存着色器/材质编译结果，加速打开工程。**不用管。**

### 10. Platforms/ —— 平台特定文件

特定平台（如 Windows/Android）的额外文件。**一般不用管。**

---

## 三、你最该关注的两个目录

| 目录 | 为什么重要 |
|------|-----------|
| **Source/LyraGame/** | 所有 C++ 游戏逻辑（Experience/GAS/角色/装备） |
| **Content/** | 所有游戏资产（蓝图/地图/数据资产） |

**学习 Lyra 源码，主要看 `Source/LyraGame/`**。

---

## 四、Source/ 里那些 Target.cs 是干嘛的

Lyra 有多个 `.Target.cs`，是因为它支持**多种平台/联机**：

| Target.cs | 干嘛 |
|-----------|------|
| `LyraGame.Target.cs` | 基础目标 |
| `LyraClient.Target.cs` | 纯客户端（联机用） |
| `LyraServer.Target.cs` | 专用服务器 |
| `LyraEditor.Target.cs` | 编辑器 |
| `LyraGameEOS.Target.cs` | EOS 联机版 |
| `LyraGameSteam.Target.cs` | Steam 版 |

**理解**：Lyra 展示了 UE5 的**多目标构建**——同一个游戏，可以编译成客户端、服务器、Steam 版、EOS 版。

---

## 五、总结速查

```
LyraStarterGame/
├── Source/       ★ 代码（LyraGame 运行时 + LyraEditor 编辑器 + Target.cs）
├── Content/      ★ 资产（蓝图/材质/地图/数据资产）
├── Plugins/      ★ 插件（GameFeature/CommonUI/消息路由）
├── Config/         配置（ini）
├── Build/          构建脚本
├── Binaries/       编译产物（.dll，不用看）
├── Intermediate/   编译中间（自动生成，不用看）
├── Saved/          运行时日志（Logs 在这看）
├── DerivedDataCache/  DDC 缓存（不用管）
└── Platforms/      平台文件

学习重点：Source/LyraGame/（代码）+ Content/（资产）
```

**一句话**：Lyra 工程的核心是 **`Source/`（C++ 代码）、`Content/`（资产）、`Plugins/`（插件）**。学 Lyra 源码主要看 `Source/LyraGame/`，运行时日志在 `Saved/Logs` 看，Binaries/Intermediate 等是生成的不用管。

---

## 六、下一步

理解了目录结构，下一步深入 **`Source/LyraGame/` 内部的子目录**（Experience、GAS、Character 等系统代码怎么组织）。
