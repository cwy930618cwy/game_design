# 05c — Slate 是什么：UE 自己的 C++ UI 框架

> **定位**：`05b` 里出现了 `Slate` / `SlateCore` 两个模块。你可能好奇：UE 做界面不是用蓝图 Widget（UMG）吗？怎么还有"C++ UI 框架"？这一篇讲清 Slate 是什么、和 UMG 什么关系、你什么时候会碰它。

---

## 一、一句话定义

**Slate 是 UE 自己写的 C++ 声明式 UI 框架**——用它可以用纯代码"拼"出界面。**UE 编辑器的所有界面**（工具栏、面板、属性窗口…）都是 Slate 写的；你在游戏里常用的 **UMG 控件，底层也是建立在 Slate 之上**。

> 💡 **场景锚点**：你现在看到的整个 Unreal Editor 窗口本身就是 Slate 做的。游戏里那些能拖拽做出来的 UI（UMG）是它上面的"亲民层"。

---

## 二、先建立 UI 分层观：UE 界面是四层汉堡

```
 ④ 你的游戏界面（血条/技能栏/主菜单）
    用 UMG 蓝图控件拼 → 存成 .uasset
 ③ CommonUI 等控件库（Lyra 用的通用 UI 组件）
    仍是 UMG 控件，只是"预制件更全"
 ② UMG（可蓝图化 UI）
    UUserWidget = 一个可拖拽的"画布 + 控件树"
    ← 游戏开发者主要待在这一层
 ① Slate / SlateCore（C++ UI 底层）
    SWidget 控件树、布局、绘制
    ← ①② 全部跑在 Slate 上；编辑器 UI 直接住在这层
```

| 层 | 用什么写 | 谁能用 |
|---|---|---|
| Slate | 纯 C++（`SNew(...)` 拼控件树） | 引擎/编辑器、需原生 UI 时 |
| UMG | 蓝图可视化 + C++ 基类 | 游戏界面（**推荐**） |
| SlateCore | C++（Slate 的地基：布局、样式、绘图原语） | Slate 本身依赖它 |

> 💡 **记忆**：Slate = "毛坯房装修（C++ 硬装）"；UMG = "精装房拎包入住（蓝图拖拽）"。游戏 UI 99% 用 UMG，Slate 是你好奇"编辑器 UI 咋来的"时往下挖的那一层。

---

## 三、"声明式"是什么意思（Slate 最核心的概念）

传统写 UI 是"一步步命令"：先创建按钮→设文字→添加到窗口…

Slate 反过来，用 **`SNew` + `Slot`** 一次性**描述**整棵控件树长什么样：

```cpp
// 一个简单的 Slate 窗口内容：竖排一个按钮
SNew(SVerticalBox)                            // 新建"竖排容器"
+ SVerticalBox::Slot()                        // 加一个插槽(行)
  .Padding(10)                                // 留白
  [
    SNew(SButton)                             // 新建按钮
      .OnClicked_Lambda([this]() { DoSomething(); })
      [
        SNew(STextBlock)                      // 按钮里的文字
          .Text(FText::FromString(TEXT("点我")))
      ]
  ]
```

**逐个词解剖：**

```
 SNew(SVerticalBox)          创建控件 = "new 一个竖排盒子"
 + SVerticalBox::Slot()      往盒子里加一格（像 UMG 的 VerticalBox 加子项）
   .Padding(10)              .参数名(值) = 设置属性（链式调用）
   [ ... ]                   中括号把"子控件"包进这个 Slot
 SNew(SButton)               又创建按钮
 .OnClicked_Lambda(...)      给按钮绑点击回调
 SNew(STextBlock) .Text(...) 文字控件 + 文本内容
```

**Slate 控件命名规律**：几乎都以 `S` 开头（`SButton`/`STextBlock`/`SVerticalBox`/`SWindow`…）——看到 `S` 开头就知道是 Slate 控件。

> 💡 **场景**：你在编辑器里右键点开的每个菜单（如"Window > Output Log"）都是一个 Slate 窗口；你自己写编辑器插件时，想弹一个自定义工具窗口，就要用 `SNew(SWindow)` + 上面这套拼法。

---

## 四、为什么游戏 UI 不用 Slate，而用 UMG

| 对比 | Slate | UMG |
|---|---|---|
| 谁来写 | 必须 C++ 编译 | 蓝图拖拽、美术/策划也能做 |
| 改起来 | 改一行要重编译 | 编辑器里即时改、立即看效果 |
| 资源/样式 | 代码里写死样式 | 可存资产、可热重载 |
| 网络动画定位 | 麻烦 | 内置锚点/动画蓝图 |
| 适合 | 编辑器工具、原生小控件 | **游戏运行时界面** |

所以分工是：

- **编辑器里的界面（工具面板、右键菜单）→ Slate**（反正都是 C++ 环境）
- **游戏里的界面（血条、菜单）→ UMG**（快、可蓝图化、美术友好）
- **两者关系**：UMG 控件的底层实现仍是 Slate（`UUserWidget` 内部挂着一个 Slate 控件树）。你平时不用知道，知道是为了"UI 出 bug 往底层排查"时能定位。

---

## 五、回到 Lyra：哪些地方真的用了 Slate

翻源码你会发现，Slate 依赖在 `LyraGame.Build.cs` 的 **Private** 名单里（`05b` 组 1），但主要在**编辑器侧**真正写：

| 用 Slate 的地方 | 干嘛 |
|---|---|
| Lyra 编辑器工具 / 自定义按钮 | 用 Slate + ToolMenus 往编辑器加菜单项 |
| 引擎自带窗口（Content Browser、Detail 面板…） | 全是 Slate（Lyra 只是"坐在里面"） |
| 特殊启动画面（早期 Loading） | 有时直接 Slate，因为引擎还没跑 UMG |

> 换句话说：**写 Lyra 玩法 UI 时，你碰的是 UMG/CommonUI；Slate 是"引擎和编辑器自己穿的内衣"**。但你看到 `05b` 里那个 `Slate` 依赖不奇怪——模块头文件/编译层面总会带上它。

---

## 六、本篇一句话

Slate = **UE 的 C++ 声明式 UI 框架**：用 `SNew(...)` + `Slot` + `[...]` 拼控件树，是编辑器 UI 的本体、UMG 的地基。做游戏界面用 UMG，做编辑器工具/想懂底层才碰 Slate。**看到 `S` 开头（SButton/STextBlock/SWindow）就是 Slate 控件**——这是你以后读引擎/插件源码时最常遇到的"暗号"。
