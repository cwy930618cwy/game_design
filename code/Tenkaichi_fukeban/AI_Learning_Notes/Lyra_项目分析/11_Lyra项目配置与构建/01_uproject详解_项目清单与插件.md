# 01 — `.uproject` 详解：项目清单与插件

> **定位**：上一篇 `00` 给了三件套全局地图。这一篇单独把 `.uproject` 拎出来，逐字段拆干净——它是 UE 引擎"认不认识这个项目、用什么引擎版本、加载哪些模块和插件"的第一道关卡。

---

## 一、一句话：`.uproject` 是项目的"身份证 + 启动清单"

UE 双击一个项目时，引擎**第一步读的就是 `.uproject`**。它是一份 JSON，回答四个问题：

| 问题 | 对应字段 |
|---|---|
| 这是不是个 UE 项目、清单格式多新 | `FileVersion` |
| 用哪一版引擎打开 | `EngineAssociation` |
| 要编译哪些代码模块 | `Modules` |
| 额外启用哪些插件 | `Plugins` |

> **大白话**：`.uproject` 好比一个游戏"楼盘"的**总立项书**。物业（引擎）一看立项书，才知道该派哪个版本的施工队（引擎版本）、先盖哪几栋楼（模块）、预装哪些市政设施（插件）。

---

## 二、真实文件拆解：以本仓库 `code.uproject` 为例

打开 `code/code.uproject`：

```json
{
	"FileVersion": 3,
	"EngineAssociation": "{2A415D17-44CA-408E-46CC-B5BC3CA623F1}",
	"Category": "",
	"Description": "",
	"Modules": [
		{
			"Name": "code",
			"Type": "Runtime",
			"LoadingPhase": "Default"
		}
	],
	"Plugins": [
		{
			"Name": "ModelingToolsEditorMode",
			"Enabled": true,
			"TargetAllowList": [
				"Editor"
			]
		}
	]
}
```

### 字段逐个讲

**① `FileVersion: 3`** —— JSON 清单格式版本号。
- 由 UE 自己维护，升级引擎时偶尔自动抬升。
- 不需要手工写，知道它"存在、别乱改"即可。

**② `EngineAssociation: "{GUID}"`** —— 用哪个引擎打开。
- 两种写法：`"5.6"`（Launcher 版，写引擎版本号）或 `"{GUID}"`（源码版，写引擎注册 GUID）。
- 本地这份是 `{2A415D17-...}`，说明关联的是**源码版 UE5.6**（Custom build）。

> 💡 **使用场景：报错"该引擎关联的版本无效"**——多半是换电脑/换引擎后 GUID 对不上。右键 `.uproject` → *Switch Unreal Engine version* 重新选一次引擎即可，就是改这个字段。

**③ `Category` / `Description`** —— 纯备注（分类/描述），不参与编译。留空不影响运行，写上是给团队看的。

**④ `Modules: [...]`** —— 声明**本项目的 C++ 模块**（代码模块）。
- `"Name": "code"`：模块名。对应 `Source/code/` 目录 + `code.Build.cs`。
- `"Type": "Runtime"`：运行时模块（游戏打包后要跑的）。其他常见类型见下表。
- `"LoadingPhase": "Default"`：加载时机，`Default` = 引擎初始化到通用阶段就加载。

| `Type` | 含义 | 典型用途 |
|---|---|---|
| `Runtime` | 进游戏包，玩家运行时加载 | 游戏逻辑主模块 |
| `RuntimeNoCommandlet` | 运行时但不进命令行工具 | 含仅游戏侧逻辑的模块 |
| `Editor` | 只在编辑器里编 | 编辑器工具、Cook 辅助 |
| `Developer` | 开发期工具，不进发布包 | 调试辅助逻辑 |
| `Program` | 独立工具程序 | 命令行小工具 |
| `ClientOnly` / `ServerOnly` | 只随客户端/服务器打包 | 分流加载逻辑 |

> 💡 **使用场景：想加一个"游戏核心"模块**——新建 `Source/GameCore/GameCore.Build.cs`，然后在 `Modules` 里补一段 `{ "Name": "GameCore", "Type": "Runtime", "LoadingPhase": "Default" }`，引擎下次启动就会编它。

**⑤ `Plugins: [...]`** —— 为项目**额外启用插件**（引擎/第三方插件），与 `Modules` 是两码事：

| | Modules | Plugins |
|---|---|---|
| 本质 | 你自己/项目内的 C++ 代码模块 | 引擎或外部提供的功能包 |
| 目录 | `Source/` | `Plugins/`、引擎自带 |
| 作用 | 声明"要编哪些代码" | 声明"要开哪些现成功能" |

- `"Name": "ModelingToolsEditorMode"`：本地例子只启用了建模工具。
- `"Enabled": true`：开还是关。
- `"TargetAllowList": ["Editor"]`：**限定哪些构建目标才启用**该插件。这里意思是：只有编辑器目标允许它加载，打包客户端/服务器不会带。

---

## 三、对照：Lyra 的 `.uproject` 长什么样

Lyra（`LyraStarterGame.uproject`）是同一个 JSON 结构，只是更"满"：

```jsonc
{
	"FileVersion": 3,
	"EngineAssociation": "5.6",          // Launcher 版写法：直接写版本号
	"Modules": [
		{ "Name": "LyraGame",   "Type": "Runtime", "LoadingPhase": "Default" },
		{ "Name": "LyraEditor", "Type": "Editor",  "LoadingPhase": "Default" }
	],
	"Plugins": [
		{ "Name": "GameplayAbilities", "Enabled": true },              // GAS
		{ "Name": "GameFeatures",      "Enabled": true },              // 模块化玩法
		{ "Name": "ModularGameplay",   "Enabled": true },              // ModularActor 等
		// ... 一长串其他插件
	]
}
```

可对比出几个规律：

1. **Module 拆多份**：主游戏模块（`LyraGame`，Runtime）+ 编辑器专用模块（`LyraEditor`，Type=Editor），分工明确。
2. **插件靠 Plugins 开**：GAS、GameFeatures、ModularGameplay 全是**引擎插件**，Lyra 只是"启用 + 引用"，源码不在自己仓库里。
3. 版本写法取决于引擎来源（`5.6` vs `{GUID}`），两种都能被正确打开。

---

## 四、到底谁先加载：引擎启动读文件的顺序

```
双击 .uproject
      │
      ▼
① 引擎按 EngineAssociation 选版本
      │
      ▼
② 读 Modules ──► 找到 Source/xx/xx.Build.cs 组织编译
      │
      ▼
③ 读 Plugins ──► 挂载插件（插件也有自己的 .uplugin）
      │
      ▼
④ 编译/启动游戏主模块（LoadingPhase 决定先后）
```

> 💡 **使用场景：为什么我明明在编辑器里启用了插件却还是编译报 include 错误**——因为 `Plugins` 启用 ≠ 你的代码模块能 include 它。include 权限由对应 `*.Build.cs` 的 `PublicDependencyModuleNames` 决定（这是 `05_Build.cs详解` 篇的内容）。

---

## 五、日常最常见的四个"改 uproject"场景

| 场景 | 改哪里 | 为什么 |
|---|---|---|
| 换引擎版本/换电脑 | `EngineAssociation` | 右键 Switch Version 自动改 |
| 新增一个游戏模块 | `Modules` 加一项 | 让引擎去编新 `Source/xx` |
| 想用某引擎插件（如 GAS） | `Plugins` 加一项 | 挂载现成功能 |
| 插件只想在编辑器用 | 加 `TargetAllowList: ["Editor"]` | 限制发布包体积/权限 |

---

## 六、踩坑清单

| 坑 | 表现 | 应对 |
|---|---|---|
| 编辑器开着时改 `.uproject` | 改动不生效甚至被还原 | **先关编辑器**再改，改完右键 Generate + 重开 |
| 手写 GUID/版本号打错 | 提示"项目找不到引擎" | 用 Switch Unreal Engine version，别手敲 |
| Modules 写的模块在 Source 里不存在 | 编译报找不到模块 | 先建好对应 `Source/目录/Build.cs` 再填 |
| 以为开了插件就能 include | include 报红 | 去 `*.Build.cs` 加依赖，见 05 篇 |
| 拖入的插件没 `Enabled: true` | 编辑器里看不到 | 确认 `"Enabled": true` |

---

## 七、总结图：code.uproject 解剖（左：写了什么 / 右：引擎拿去干嘛）

```
        code.uproject —— 一段 JSON，引擎启动的第一道门（以本仓库真实内容为准）
 ════════════════════════════════════════════════════════════════════
  左：文件里写了什么                        │ 右：引擎拿它去干什么
 ───────────────────────────────────────────┼───────────────────────
 {                                         │
   "FileVersion": 3,                       │ 清单格式版本
                                           │ → 引擎自己维护，别手改
                                           │
   "EngineAssociation":                    │ 决定"用哪个 UE 打开"
     "{2A415D17-44CA-...}"                 │ → 写 "5.6"   用 Launcher 版
                                           │ → 写 "{GUID}" 用源码注册版
                                           │   (本仓库这条 = 源码引擎 5.6)
                                           │
   "Category": "", "Description": "",      │ 纯备注，不参与编译
                                           │
   "Modules": [                            │ ① 编"项目自己的 C++ 模块"
     {                                      │
       "Name": "code",                     │ → 按名字找 Source/code/ 目录
       "Type": "Runtime",                  │   └─ code.Build.cs（依赖清单）
       "LoadingPhase": "Default"           │       读 GAS 三件套 → 编成模块
     }                                     │
   ],                                      │ (Lyra 对照：写两条
                                           │  LyraGame + LyraEditor)
                                           │
   "Plugins": [                            │ ② 挂"现成的引擎/外部功能"
     {                                      │
       "Name": "ModelingToolsEditorMode",  │ → 按名字去引擎插件目录找
       "Enabled": true,                    │   └─ .../ModelingToolsEditorMode
       "TargetAllowList": ["Editor"]       │      .uplugin → 挂载
     }                                     │   └ 仅编辑器目标启用，不进游戏包
   ]                                       │
 }                                         │
 ───────────────────────────────────────────┼───────────────────────
  双击后引擎依次干的事：                      │
  ① 看 EngineAssociation 选对引擎           │
  ② 读 Modules → 找到同名的 .Build.cs      │ → 编译出模块
  ③ 读 Plugins → 找到同名的 .uplugin       │ → 挂上功能
  ④ 启动，各模块按 LoadingPhase 先后加载     │
  ⑤ 编辑器打开 / 游戏跑起来                 │
 ════════════════════════════════════════════════════════════════════
  一句话：.uproject 只回答 4 个问题——
       是不是项目(FileVersion)、用哪个引擎(EngineAssociation)、
       编哪些模块(Modules)、开哪些插件(Plugins)。
```

**记忆锚点**：`.uproject` 只管"**清单**"，不管"模块能 include 什么"（那是 `Build.cs` 的事）、也不管"要编出几种可执行程序"（那是 `Target.cs` 的事）——这两个已经分别在 `02` 篇讲过了。
