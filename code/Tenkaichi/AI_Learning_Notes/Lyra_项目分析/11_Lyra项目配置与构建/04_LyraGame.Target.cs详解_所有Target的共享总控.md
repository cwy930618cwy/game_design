# 04 — `LyraGame.Target.cs` 详解：所有 Target 的"共享总控"

> **定位**：上一篇 `03` 用最短的 `LyraClient.Target.cs`（17 行）入门。现在啃全工程最长的 `LyraGame.Target.cs`（283 行）。它不只是一个 Target，而是 Lyra 构建体系的"**总控台**"——其他 9 个 Target 全都直接或间接跟它挂钩。
>
> **一句话记住它的三副面孔**：
> 1. **它自己** = 一个 `Type=Game` 的普通目标（单机/测试用的游戏进程）
> 2. **共享配置源头** = `ApplySharedLyraTargetSettings()`，全 Lyra 编译规矩都在这里
> 3. **玩法包开关器** = `ConfigureGameFeaturePlugins()`，打包时自动决定带/不带哪些 GameFeature

---

## 一、先看整体结构：283 行其实就 4 块

```
 LyraGame.Target.cs（283 行）                  │ 各自干嘛
 ══════════════════════════════════════════════┼════════════════════
 L1-9   using 头                               │ 引入 UBT/IO/日志库
 ──────────────────────────────────────────────┼────────────────────
 L11-20 class LyraGameTarget : TargetRules     │ ① 构造函数（很短）
         + 构造函数                             │  Type=Game
                                              │  带 LyraGame 模块
 ──────────────────────────────────────────────┼────────────────────
 L22     bHasWarnedAboutShared                 │ ② 只警告一次的标志
 ──────────────────────────────────────────────┼────────────────────
 L24-95 ApplySharedLyraTargetSettings()        │ ③ 共享配置 ★核心★
         （所有 Target 都调它）                 │  编译版本/警告等级/
                                              │  安全加固/瘦身/开关玩法包
 ──────────────────────────────────────────────┼────────────────────
 L97-116  ShouldEnableAllGameFeaturePlugins()  │ ④ GameFeature 决策
 L118-282 ConfigureGameFeaturePlugins()        │  扫描并启用/禁用
 ══════════════════════════════════════════════┴════════════════════
 看明白了吗：真正"为它自己"写的代码只有头 20 行，
 剩下 260 多行全是给"整个工程所有 Target"服务的。
```

> 💡 先记住这个反直觉的点：**一个文件，绝大部分代码不是给自己用的**。这是工程化里常见的"总控/配置集中"写法。

---

## 二、① 构造函数（L11-20）：跟 `LyraClient` 长得一模一样

```csharp
public LyraGameTarget(TargetInfo Target) : base(Target)
{
	Type = TargetType.Game;                        // 这是 Game 目标
	ExtraModuleNames.AddRange(new string[] { "LyraGame" });
	LyraGameTarget.ApplySharedLyraTargetSettings(this);   // 调"自己"的静态方法
}
```

跟 `03` 篇的 `LyraClient` 相比只差一行 `Type`：

| Target | Type | 差别 |
|---|---|---|
| `LyraGameTarget` | `Game` | 单进程 = 客户端逻辑 + 可开 listen 服务器（单机测试常用） |
| `LyraClientTarget` | `Client` | 纯客户端，只能连别人开的服 |

> 💡 **场景记忆**：在编辑器里直接"Play"单人试玩，背后用的是 **Game** 目标的能力；正式发给玩家的安装包才是 **Client**。

---

## 二点五、一个"奇怪的 C# 细节"：为什么方法要传 `this`

```csharp
LyraGameTarget.ApplySharedLyraTargetSettings(this);   // this = 正在构造的 Target
```

静态方法里没有 `this`，所以调用时把**当前目标对象自己**传进去。好处：这个共享方法可以被 `LyraClientTarget`、`LyraServerTarget`…任何目标调用，把"谁"传进去，就替"谁"配置。这就是"写一处、服务十个文件"的机制。

> 💡 **类比**：像一张**公用检查表**（方法），每个部门（Target）报到时都拿着自己的名字来核销（传 this），表上的每条规矩（配置项）就作用在这个部门头上。

---

## 三、② `ApplySharedLyraTargetSettings`（L24-95）：共享配置逐块拆

函数开头先取了个 `Logger`（打印构建日志用），然后算三个标志：

```csharp
bool bIsTest      = 配置 == Test;        // 测试包？
bool bIsShipping  = 配置 == Shipping;    // 正式发布包？
bool bIsDedicatedServer = Type == Server; // 是不是专服？
```

下面整个函数按这个"大前提"分组：

### 块 A（L28-29）：引擎编译规范

```csharp
Target.DefaultBuildSettings = BuildSettingsVersion.V5;
Target.IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
```

让所有目标用同一套"最新编译规范"。**所有 Target 都遵守 → 不会出现 A 目标能用新版语法、B 目标不行**的混乱。

> 💡 **场景**：万一升级引擎后头文件 include 顺序规则变了，改这一处，全部目标同步跟上。

### 块 B（L34-36）：代码质量红线

```csharp
if (BuildEnvironment == Unique)  // "独构建"环境才执行，原因见下
{
	Target.CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Error;
```

`ShadowVariable` = **变量遮蔽**（内层变量盖住了外层同名变量，极易写错变量）。Lyra 直接把这种警告升成**编译错误**——宁可编译不过，不让带隐患的代码混进游戏。

> 💡 **场景**：你曾在回调里写了 `int Damage = ...` 把外面同名 `Damage` 遮住了，运行时数字神秘不对。Lyra 这样设就是让你**编译阶段就爆错**，而不是上线后踩坑。

### 块 C（L41-54）：正式包（Shipping）加固

```csharp
if (bIsShipping && !bIsDedicatedServer)
{
	Target.bDisableUnverifiedCertificates = true;   // HTTPS 必须验证证书
}
```

`bDisableUnverifiedCertificates = true`：**只信任验证通过的 HTTPS 证书**，防中间人劫持下载/更新。后面被注释掉的几行是"命令行白名单"开关示例（想锁死命令行参数再打开）。

> 💡 **场景**：你的游戏要热更新/下载补丁，没有证书校验，坏人就可能伪造服务器下发恶意内容——正式包默认开着校验。

### 块 D（L56-61）：Test/Shipping 包禁止读本地 ini

```csharp
Target.bAllowGeneratedIniWhenCooked = false;
Target.bAllowNonUFSIniWhenCooked = false;
```

打包后玩家不能随便塞一份修改过的 ini 进来改你配置（防作弊/防乱改）。

### 块 E（L63-70）：非编辑器目标"瘦身 + 省内存"

```csharp
if (Target.Type != TargetType.Editor)
{
	Target.DisablePlugins.Add("OpenImageDenoise");          // 去噪插件只拍电影用
	Target.GlobalDefinitions.Add("UE_ASSETREGISTRY_INDIRECT_ASSETDATA_POINTERS=1");
}
```

- 禁掉 `OpenImageDenoise`（光线追踪去噪库，体积大，只在"美宣图"用）
- 打开省内存宏（AssetRegistry 少驻留数据）

> 💡 **场景**：打包给玩家时包体小几 GB，就是这么一处一处"抠"出来的。

### 块 F（L72）：Unique 分支的收尾——自动配置玩法包

```csharp
LyraGameTarget.ConfigureGameFeaturePlugins(Target);
```

见第五节。

### 块 G（L74-94）：另一个环境（Shared）怎么处理

`Unique` 和 `Shared` 是两种构建环境（不深究，记结论）：
- **Unique（独构建）**：每个目标单独完整编译 → 能自由改插件开关/配置 → 上面 A-F 全执行。
- **Shared（共享编译）**：复用引擎预编译好的共享二进制（装好版引擎打包时常见）→ **不能改插件开关**，否则影响共享 PCH 缓存。

Shared 分支里：`Editor` 目标仍可配置玩法包；其它目标只打一条**警告**（"装好的引擎没法给 EOS 等动态选项做定制"），且用 `bHasWarnedAboutShared` 保证这警告**只打一次**（L88-92）。

> 💡 **场景**：用 Launcher 装的引擎打包时，若你发现 GameFeature/EOS 相关选项"没生效"，多半就是这个 Shared 分支在起作用——不是代码错了，是环境限制。

---

## 四、④ `ConfigureGameFeaturePlugins`（L118-282）：自动开关玩法包

这是全文件最长的函数。目的：**不用人肉勾选，打包时自动决定带哪些 GameFeature**。

### 步骤 1：找到所有玩法包（L132-138）

```csharp
Unreal.GetExtensionDirs(项目目录, "Plugins/GameFeatures")
```

去 `项目/Plugins/GameFeatures/` 找所有 `.uplugin`（就是 `01a` 里说的 ShooterCore、TopDownArena 那些）。`GetExtensionDirs` 还会兼顾引擎扩展目录。

### 步骤 2：对每个 `.uplugin` 读 JSON，按规则裁决（L144-250）

一个 `.uplugin` 描述文件里可能有这些字段，代码逐个检查：

| 字段（写在玩法包 .uplugin 里） | 代码如何处理 |
|---|---|
| `EnabledByDefault` | 应写 `false`，否则打警告（内置玩法包默认关，等代码来开） |
| `ExplicitlyLoaded` | 应写 `true`，否则警告（玩法包要"显式加载"，不能开机就全载） |
| `EditorOnly: true` | 非 Editor 目标直接禁用（编辑器专用玩法包不进游戏包） |
| `RestrictToBranch: "xxx"` | 分支名不符 → 禁用（某功能只准在 main 分支编） |
| `NeverBuild: true` | 直接禁用（这包永远别编译） |
| （以上都没命中，且 `bBuildAll` 开启） | 启用 |

### 步骤 3：优先级 + 落锤（L258-275）

```csharp
// 强制禁用 优先于 启用
if (bForceDisabled) { bEnabled = false; }
...
if (bEnabled)        Target.EnablePlugins.Add(名字);
else if (bForceDisabled) Target.DisablePlugins.Add(名字);
```

读取 JSON 万一抛异常（L252-256），也会按"禁用"处理，保证**构建不会因为一个坏玩法包就崩**。

### 步骤 4：`ShouldEnableAllGameFeaturePlugins`（L97-116）

默认返回 `false`（一切以编辑器里插件浏览器的勾选为准）。留了两个"特殊通道"：
- Editor 目标里把 `return true` 放开 → 编辑器编全部玩法包
- 构建机设环境变量 `IsBuildMachine=1` → 同理可全开

> 💡 **场景记忆**：发布 1.0 版前，你把"新地图玩法包"标了 `RestrictToBranch` 只给 main 分支。Release 分支打包时，这个函数自动把它禁用——**不需要任何人记得去编辑器里取消勾选**。

---

## 五、它是怎么被其他 9 个 Target 用起来的

```
                          LyraGameTarget（本文件）
        ┌──────────────────────┼───────────────────────┐
        │                      │                       │
   ★调用共享方法             ★调用共享方法            ★继承类（连 Type 也抄）
   LyraClient.Target       LyraServer.Target         LyraGameEOSTarget
   LyraEditor.Target      (+变体还要再继承)          LyraGameSteamTarget
                                                     LyraGameSteamEOSTarget
                                                     各自加 CustomConfig
                                                     见 02 篇
```

- **Client / Server / Editor**：`Type` 自己定，只调 `ApplySharedLyraTargetSettings` 蹭共享配置。
- **Steam / EOS 变体**：直接**继承** `LyraGameTarget` 这个类，所以构造里那三行 + 共享设置自动全有了，只需再加一行 `CustomConfig="Steam"`。

> 💡 区分两个概念：
> - **调用共享方法**（Client/Server/Editor）= "你的规矩我照抄，但我类型不同"
> - **继承类**（Steam/EOS 变体）= "我完全是你，再补个小标签"

---

## 六、总结解剖图：直接对着源码结构看

```
     LyraGame.Target.cs（283行）真实骨架：左=代码本身 │ 右=一句批注
══════════════════════════════════════════════════════════════════
 class LyraGameTarget : TargetRules                 │ 类名=文件名
 {                                                 │
   LyraGameTarget(TargetInfo Target)               │ 构造函数(UBT来调用)
   {                                               │
     Type = TargetType.Game;                       │ 我是 Game 目标
     ExtraModuleNames.AddRange(                    │ 打包要带模块：
       new string[]{ "LyraGame" });                │   LyraGame
     LyraGameTarget.ApplySharedLyraTargetSettings( │ 调下面的共享方法
       this);                                      │   (把自己传进去)
   }                                               │
  ─────────────────────────────────────────────────┼────────────────
   static ApplySharedLyraTargetSettings(Target)    │ ★所有其他 Target
   {                                               │   都来调这个方法
     Target.DefaultBuildSettings = V5;             │ 统一编译规范
     Target.IncludeOrderVersion = Latest;          │ 统一头文件顺序
     if (Unique 构建环境) {                         │ "独构建"才可调配置
       ShadowVariableWarningLevel = Error;         │ 变量遮蔽=编译错误
       if (Shipping && !专服)                      │ 正式发布包:
         bDisableUnverifiedCertificates = true;    │   HTTPS强制验证书
       if (Shipping || Test)                       │ 玩家不可读 ini：
         bAllowGeneratedIniWhenCooked = false;     │   禁Generated ini
         bAllowNonUFSIniWhenCooked = false;        │   禁NonUFS ini
       if (Type != Editor) {                       │ 非编辑器瘦身：
         DisablePlugins.Add("OpenImageDenoise");   │   禁光追去噪(省体积)
         GlobalDefinitions.Add("UE_ASSETREGISTRY_  │   省内存宏
           INDIRECT_ASSETDATA_POINTERS=1");        │
       }                                           │
       ConfigureGameFeaturePlugins(Target);        │ 收尾:自动开关玩法包↓
     }                                             │
     else if (Editor) { ... } else { 警告一次 }     │ Shared环境分支
   }                                               │
  ─────────────────────────────────────────────────┼────────────────
   static ConfigureGameFeaturePlugins(Target)      │ ★玩法包开关器
   {                                               │
     枚举 Plugins/GameFeatures/*.uplugin           │ 先找出所有玩法包
     for (每个玩法包) {                             │ 逐个读它.json
       if (EnabledByDefault 没写false)  警告;      │ 内置包应默认关闭
       if (ExplicitlyLoaded 没写true)   警告;      │ 应显式加载
       if (bBuildAll)                   启用;      │ 全构建开关
       if (EditorOnly && 非Editor目标)  强制禁用;   │ 编辑器包不进游戏
       if (RestrictToBranch 不符)       强制禁用;   │ 分支限制(如仅main)
       if (NeverBuild)                  强制禁用;   │ 永远别编
       解析异常(ParseException)          强制禁用;   │ 坏包不炸构建
     }                                           │
     if (bForceDisabled) bEnabled = false;        │ 强制禁用优先
     Target.EnablePlugins.Add(名字) / DisablePlugins  │ 最后落锤
   }                                               │
══════════════════════════════════════════════════════════════════
  一句话：20 行为自己（Type=Game），260 行为全 Lyra——
         Client/Server/Editor 调它的共享方法(ApplyShared)，
         Steam/EOS 变体继承它的类(class X : LyraGameTarget)。
```

**本篇一句话**：`LyraGame.Target.cs` 是 Lyra 所有 Target 的**公共源头**——它的构造函数定义"Game 类型"模板，共享方法统一所有目标的编译规矩，GameFeature 函数负责打包时自动取舍玩法包。看懂它，Lyra 的构建体系就通了 80%。
