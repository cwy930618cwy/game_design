# 05b — LyraGame 的 Private 依赖逐个简介：这 26 个模块都是干嘛的

> **定位**：`05a` 把 Public 名单（22 个，露给下游）讲完了。这篇讲另一半——**Private 名单（26 个）**：同样逐个过一遍"底层是啥、Lyra 拿它干嘛"，但记住它们和 Public 的性质不同：**只有 LyraGame 自己内部能用，不会传导给引用 LyraGame 的模块**。
>
> **先认清单**：对照 `LyraGame.Build.cs` L51-76，Private 依赖共 **26 个**。

---

## 一、为什么这批"藏起来"的反而更贴近玩法

回顾 `05` 的判断口诀：**头文件里 include 的 → Public；只有 .cpp 里 include 的 → Private。**

所以 Private 名单的画像通常是：
- UI、输入这类"**实现时才碰**"的东西（玩家 UI 代码不会在 LyraGame 公共头里出现）
- **项目自己写的小模块**（CommonGame 等，不用暴露给外人）
- 纯工具类（JSON、音频混合……）

> 💡 **场景**：你写了一个血条 Widget 只在你自己的 .cpp 里用 → 放 Private，别人引用你模块时编译器不用白加载 UMG 一大堆东西（编译更快）。

---

## 二、逐个简介：26 个分 6 组

### 组 1：UI 与输入（人机交互这一挂）

| 模块 | 底层是啥（核心类/本质） | Lyra 拿它干嘛 |
|---|---|---|
| `SlateCore` | Slate UI 框架底层（布局/样式/渲染原语） | 给上层 UI 打底 |
| `Slate` | Slate 上层：编辑器、原生 UI 控件 | 编辑器 UI、自定义原生控件 |
| `UMG` | **运行时 UI**：`UUserWidget`、控件蓝图 | 血条、技能栏、菜单界面 |
| `CommonUI` | 通用 UI 控件库（输入提示自动切键鼠/手柄） | 跨平台 UI 统一风格 |
| `CommonInput` | 记录当前输入设备（键鼠/手柄/触屏） | 决定 UI 显示哪套按键提示 |
| `InputCore` | 输入基础：`EKeys`（按键枚举、轴类型） | 判断"按的是哪个键" |
| `EnhancedInput` | 新版增强输入：`UInputAction/UInputMappingContext` | 移动/瞄准/开火的输入映射 |

### 组 2：渲染底层（不动它，但被拖拽依赖）

| 模块 | 底层是啥 | Lyra 拿它干嘛 |
|---|---|---|
| `RenderCore` | 渲染框架（绘制命令、着色器参数） | 一切画面前提 |
| `RHI` | **渲染硬件抽象**：同一套代码跑 DX12/Vulkan | 换显卡 API 不改上层 |

### 组 3：网络与回放（联机这挂）

| 模块 | 底层是啥 | Lyra 拿它干嘛 |
|---|---|---|
| `NetCore` | 网络核心（网络版本、`FName` 序列化基础） | 联机底层设施 |
| `DTLSHandlerComponent` | 数据包 **DTLS 加密**组件 | 传输加密（前面插件篇讲过） |
| `NetworkReplayStreaming` | 录像回放流式读写 | 观战/回放功能 |

### 组 4：Lyra 自家 UI/账号全家桶（项目自带，都在 `Lyra/Plugins/`）

| 模块 | 来自哪个项目插件 | 干嘛的 |
|---|---|---|
| `CommonGame` | `CommonGame` | 游戏层 UI 管理：激活窗口、UI 分层 |
| `CommonUser` | `CommonUser` | 平台账号：登录、选用户 |
| `GameSettings` | `GameSettings` | 画质/音量/按键设置存取 |
| `GameSubtitles` | `GameSubtitles` | 对话字幕 |
| `GameplayMessageRuntime` | `GameplayMessageRouter` | 全局消息总线（模块名=Runtime） |
| `UIExtension` | `UIExtension` | 扩展槽：其他模块往 Lyra UI 插按钮 |

> 这些是 Lyra 自己写的"**私有家具**"——只给自己家用，更不需要暴露给下游，放 Private 理所当然。

### 组 5：设置 / 工程 / 工具（引擎）

| 模块 | 底层是啥 | Lyra 拿它干嘛 |
|---|---|---|
| `Projects` | 项目/插件描述读取（`FPluginManager`） | 运行时问"某插件开了没" |
| `DeveloperSettings` | `UDeveloperSettings` | 让自定义设置出现在 Project Settings 面板 |
| `EngineSettings` | 引擎级设置类 | 读引擎默认配置 |
| `Gauntlet` | 自动化测试框架 | CI/夜里自动跑游戏测试 |
| `ClientPilot` | 客户端行为引导框架 | 自动化/新手引导数据采集 |
| `Json` | JSON 读写库 | 解析网络/配置文件 |

### 组 6：音频（引擎）

| 模块 | 底层是啥 | Lyra 拿它干嘛 |
|---|---|---|
| `AudioMixer` | 音频混合低层 | 声音最终混音输出 |
| `AudioModulation` | 音频动态调制 | 环境音随状态渐变 |

---

## 三、底层源码画图：这 26 个从哪来、被谁"藏"着

```
          ┌─────────────────────────────────────────────┐
          │   下游模块（引用 LyraGame 的，如 LyraEditor）   │
          │   只能看到 Public 22 个 → 看不到下面这些 ↓      │
          └─────────────────────────────────────────────┘
                     ▲  头文件边界（Public 露头）
                     │
  LyraGame 模块内部 ············································
   Private 26 个 = "外套内侧口袋里的东西"：

   引擎自带（约 20 个）                    项目自带（约 6 个）
   Engine/Source/Runtime + 引擎插件        Lyra/Plugins/
   ┌────────────────────────────┐        ┌─────────────────────┐
   │ UI/输入：SlateCore / Slate │        │ CommonGame          │
   │   / UMG / CommonUI /      │        │ CommonUser          │
   │   CommonInput / InputCore │        │ GameSettings        │
   │   / EnhancedInput         │        │ GameSubtitles       │
   ├────────────────────────────┤        │ GameplayMessageRouter│
   │ 渲染：RenderCore / RHI     │        │   (模块名 Runtime)   │
   ├────────────────────────────┤        │ UIExtension         │
   │ 网络：NetCore / DTLS加密 / │        └─────────────────────┘
   │   NetworkReplayStreaming  │          ← 全部是 Lyra 自写
   ├────────────────────────────┤
   │ 设置工具：Projects /       │
   │   DeveloperSettings /     │
   │   EngineSettings / Gauntlet│
   │   / ClientPilot / Json    │
   ├────────────────────────────┤
   │ 音频：AudioMixer /        │
   │   AudioModulation         │
   └────────────────────────────┘
  ·······························································
  一句话：Private = 只给自己用的模块；
        引擎自带的做"实现细节"，Lyra 自带的做"私有家具"。
```

**读图要点**：
- Private 名单 = LyraGame "**外套内侧的口袋**"：下游看不见、也不该看见；
- 引擎自带的约 20 个负责**实现细节**（UI 渲染、加密、音频）；
- 项目自带的约 6 个是 **Lyra 自己写的小模块**（账号/设置/消息总线/UI 扩展），更没理由外传。

---

## 四、本篇一句话

26 个 Private 依赖 = LyraGame 说"**这些模块只有我自己实现时用，别传导给我的调用方**"。它和 Public 名单的分界线只有一条：**你的公共头文件 include 谁 → 谁进 Public**。这张 Private 名单里 UI/输入/账号占了一大半，正是"玩法里最常碰、但最不该外漏"的那批东西。Public + Private 两篇加起来，就是 `LyraGame.Build.cs` 全部 48 个依赖（22 + 26）的家谱。
