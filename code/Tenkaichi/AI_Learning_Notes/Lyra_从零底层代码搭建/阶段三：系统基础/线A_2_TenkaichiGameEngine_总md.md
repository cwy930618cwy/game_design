# 线A-2 TenkaichiGameEngine —— 总 md

> **定位**：阶段三（L3 系统基础）**线 A 第 2 项**的**总入口 + 全景**。
> 目标：理解并一比一还原 Lyra 的 `System\LyraGameEngine.h/.cpp`——一个"自定义引擎类"。
> 源码依据：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\System\LyraGameEngine.h/.cpp`
> 命名：`ULyraGameEngine` → **`UTenkaichiGameEngine`**；文件 `LyraGameEngine` → **`TenkaichiGameEngine`**（只换前缀，一字不砍）。

---

## 一、为什么有它（解决什么问题）

UE 引擎启动时，会创建一个"引擎对象"（`UGameEngine`）负责整个引擎的生命周期。默认是引擎自带的 `UGameEngine`。

但游戏想**在引擎启动的时机里插一脚做自定义事**（比如初始化全局模块、注册回调、改引擎行为），就需要一个**继承自 `UGameEngine` 的自定义引擎类**。

Lyra 的 `ULyraGameEngine` 就是干这个的——**它是"游戏自己的引擎类"**，目前 `Init()` 里只是调了 `Super::Init()`（没加自定义逻辑，是个空壳占位），为将来引擎启动时挂自定义代码留了个钩子。

> 一句话：**TenkaichiGameEngine = 游戏自定义的引擎类，用来在引擎启动时插入自定义逻辑（现在是空壳，留钩子）。**

---

## 二、它很小，但有个关键的"接入方式"

### 为什么它必须存在，即使现在啥都没干

因为它是"**游戏要替换默认引擎**"的入口。光建这个类还不够，还必须在配置里**告诉 UE"用我自定义的引擎类"**——否则建了白建。

### 关键的接入点：`Config/DefaultEngine.ini`

Lyra 在 `Config\DefaultEngine.ini` 配置了（真实源码第 20~22 行）：

```ini
[/Script/Engine.Engine]
DurationOfErrorsAndWarningsOnHUD=3.0
GameEngine=/Script/LyraGame.LyraGameEngine
```

**关键点：这行 `GameEngine=...` 必须放在 `[/Script/Engine.Engine]` 这个 section 底下**，不能裸放在文件里。`[/Script/Engine.Engine]` 是"引擎配置"的分区，`GameEngine` 字段属于它。

你工程里要对应写成（19 号铁律：能用配置就别动编辑器）：

```ini
[/Script/Engine.Engine]
GameEngine=/Script/TenkaichiGame.TenkaichiGameEngine
```

> 这是"**用代码/配置指定全局引擎**"的标准做法，不用在编辑器里点。
> 路径格式：纯 C++ 类用 `/Script/模块名.类名`（**不带 `_C`**），这里模块名是 `TenkaichiGame`，类名 `TenkaichiGameEngine`。
> 注意：你工程的 `DefaultEngine.ini` 里可能已经有 `[/Script/Engine.Engine]` section 了，只需把 `GameEngine=` 这一行加进**那个已有 section 里**，不要重复开一个新 section。

---

## 三、全景清单（这一步要还原的）

依据 Lyra 真实源码，`LyraGameEngine.h/.cpp` 极其精简：

| 文件 | 内容 | 规模 |
|------|------|------|
| `TenkaichiGameEngine.h` | 类 `UTenkaichiGameEngine : public UGameEngine`，声明构造函数 + 重写 `Init` | 25 行 |
| `TenkaichiGameEngine.cpp` | 实现构造函数 + `Init()`（调用 `Super::Init`） | 19 行 |
| `Config\DefaultEngine.ini` | 加一行 `GameEngine=/Script/TenkaichiGame.TenkaichiGameEngine` | 1 行 |

- 继承：`UGameEngine`（引擎自带）。
- 重写的方法：`Init(IEngineLoop* InEngineLoop)`——引擎初始化的钩子。
- 依赖：只有引擎模块，**完全独立**，不依赖阶段四/五任何类。

---

## 四、关键概念预告

- **`UGameEngine`**：引擎的"运行时引擎类"（编辑器里用的是 `UEditorEngine`）。
- **`IEngineLoop`**：引擎主循环接口，`Init` 的参数，代表整个引擎循环实例。
- **`Init()`**：引擎初始化时被调用的钩子，重写它就能在引擎启动时插代码。
- **`Config/DefaultEngine.ini` 的 `GameEngine` 字段**：指定"用哪个类当引擎"，是让自定义引擎生效的开关。

> 这几个概念的具体原理，教到 `.h`/`.cpp`/配置那步再展开。现在先知道"它是空壳 + 靠配置接入"即可。

---

## 五、教学节奏（铁律 20/29）

按铁律串行走，一步一反馈：
1. ✅ 本总 md（为什么 + 全景）——**现在在这里，等你确认**
2. ⏳ 教 `.h`（建 `..._h.md`）
3. ⏳ 教 `.cpp`（建 `..._cpp.md`）
4. ⏳ 教 `DefaultEngine.ini` 的接入配置（这步比较特殊，是配置不是代码，会在 `.cpp` 后或一起讲）

> **注意**：本文件带 `Lyra` 前缀，前缀替换为 `Tenkaichi`。注释会中文化（铁律 21/28）。

---

## 六、待确认

1. **从 `TenkaichiGameEngine` 开始**，OK 吗？
2. 确认后，我先教 `.h`（建 md），你写完说"下一步"（我会先读代码确认），再教 `.cpp`。