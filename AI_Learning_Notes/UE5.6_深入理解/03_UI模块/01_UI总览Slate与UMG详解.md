# 01 — UI 总览：Slate 与 UMG 详解

> **定位**：UE 的两套 UI 系统——`Slate`（底层，编辑器 UI 用）和 `UMG`（上层，游戏 UI 用）。学 UI 前先搞清它们的关系。
>
> **一句话**：**UMG 是给玩家做的 UI（血条/菜单/按钮），Slate 是 UMG 的底层引擎（也是编辑器界面的基础）**。你日常做游戏 UI 用 UMG（蓝图层），Slate 是进阶/编辑器用。
>
> **文件**：`Engine/Source/Runtime/Slate/`、`Engine/Source/Runtime/UMG/`、`Engine/Source/Runtime/SlateCore/`

---

## 一、先搞清：UE 有两套 UI

UE 有**两套 UI 系统**，分工不同：

```
Slate（底层 UI 框架）
  └─ 是"绘制 UI 的引擎"（控制绘制、布局、输入）
       ├─ 编辑器界面（Content Browser、属性面板）都用它
       └─ 是 UMG 的底层

UMG（用户界面层）
  └─ 在 Slate 之上，给游戏做 UI
       ├─ 血条、按钮、菜单、HUD
       └─ 用蓝图（Widget Blueprint）做，你日常用这个
```

| | Slate | UMG |
|---|---|---|
| 是什么 | 底层 UI 框架 | 游戏 UI 层 |
| 谁用 | 编辑器界面 | 游戏内 UI |
| 怎么用 | C++ 代码（SButton 等） | **蓝图**（Widget Blueprint） |
| 你日常 | 很少用 | **天天用** |

---

## 二、对应全景地图的模块（00 文档 64-66 行）

```
Slate      → UI 框架（编辑器界面、UMG 底层）⭐⭐
SlateCore  → Slate 核心（布局、绘制、输入）⭐⭐
UMG        → 蓝图 UI 系统（Widget Blueprint）⭐⭐
```

**分层**：
```
UMG（游戏 UI，蓝图做）
  └─ 依赖
Slate（UI 引擎，绘制/布局）
  └─ 依赖
SlateCore（最底层，widget 基础）
```

---

## 三、UMG —— 你日常做游戏 UI 的系统

UMG 让你**用蓝图做 UI**，不用写 C++。核心概念：

### 3.1 Widget（控件）—— UI 的零件

UI 是由**控件（Widget）**拼出来的：

| 控件 | 作用 |
|------|------|
| `UButton` | 按钮 |
| `UImage` | 图片 |
| `UTextBlock` | 文本 |
| `UProgressBar` | 血条/进度条 |
| `UVerticalBox` / `UHorizontalBox` | 垂直/水平排列 |
| `UCanvasPanel` | 自由摆放 |

### 3.2 Widget Blueprint —— 用蓝图做 UI

你创建一个 `Widget Blueprint`，在里面拖控件、连线逻辑：

```
一个 Widget Blueprint（血条 UI）
  └─ 拖一个 UProgressBar（血条）
  └─ 拖一个 UTextBlock（血量数字）
  └─ 蓝图事件里更新它们的值
```

**具体场景：做一个血量 UI**

```cpp
// 在 C++ 里拿到 UI 控件（UUserWidget 子类）
UCLASS()
class UHUDWidget : public UUserWidget {
    GENERATED_BODY()
public:
    UPROPERTY(meta=(BindWidget))   // 绑定蓝图里的同名控件
    UProgressBar* HealthBar;       // 血条控件

    void UpdateHealth(float Percent) {
        HealthBar->SetPercent(Percent);   // 更新血条
    }
};
```

### 3.3 UUserWidget —— UI 的基类

**所有游戏 UI 都继承 `UUserWidget`**：

```
UUserWidget（UI 基类）
├── 你的血条 UI
├── 你的主菜单
└── ...
```

---

## 四、Slate —— 底层 UI 引擎（进阶）

Slate 是**用 C++ 代码写 UI** 的底层框架。编辑器界面（Content Browser、材质编辑器）都是 Slate 做的。

```cpp
// Slate 用 C++ 写 UI（编辑器/工具用）
SNew(SButton)
    .Text(FText::FromString("Click"))
    .OnClicked_Lambda([]() { UE_LOG(LogTemp, Log, TEXT("Clicked")); });
```

**你什么时候用 Slate**：
- 做**编辑器插件**（自定义编辑器面板）
- 深入 UMG 底层原理
- **一般游戏 UI 不用**（用 UMG 蓝图）

---

## 五、做游戏 UI 的正确姿势（你该用什么）

**结论：做游戏 UI 用 UMG（蓝图），不用 Slate。**

```
你日常做游戏 UI：
  ✅ 用 UMG（Widget Blueprint + UUserWidget）—— 拖控件、连线
  ❌ 不用 Slate（那是编辑器/进阶用的）

只有做编辑器插件才用 Slate
```

**具体场景：给玩家做一个"开始游戏"按钮**

```
1. 创建一个 Widget Blueprint（UI_BeginMenu）
2. 拖一个 UButton（按钮）+ UTextBlock（文字"开始游戏"）
3. 按钮 OnClicked 事件 → 蓝图调用 OpenLevel
4. 在 PlayerController 里 AddToViewport() 显示它
```

---

## 六、UI 的显示流程（怎么让 UI 出现在屏幕）

```
UUserWidget（你的 UI）
  └─ CreateWidget() 创建
       └─ AddToViewport() 加到屏幕
            └─ 显示出来
```

```cpp
// 创建并显示一个 UI
UHUDWidget* HUD = CreateWidget<UHUDWidget>(GetWorld(), HUDWidgetClass);
HUD->AddToViewport();   // 显示到屏幕
```

---

## 七、总结速查

```
UE 两套 UI：
  UMG（游戏 UI，蓝图做）← 你天天用
    └─ 控件：Button/Image/TextBlock/ProgressBar
    └─ 基类：UUserWidget
  Slate（底层 UI 引擎，C++ 写）← 编辑器/进阶
    └─ 编辑器界面用它
    └─ 是 UMG 的底层

做游戏 UI：
  ✅ 用 UMG（Widget Blueprint + UUserWidget）
  ❌ 不用 Slate（编辑器才用）

显示 UI：
  CreateWidget() → AddToViewport()
```

**一句话**：UE 有**两套 UI**——**UMG**（游戏 UI，用蓝图做，你日常用）和 **Slate**（底层 UI 引擎，用 C++ 做编辑器界面）。**做游戏 UI 用 UMG + UUserWidget，Slate 只有做编辑器插件才用。**

---

## 八、下一步

理解了 UMG vs Slate，下一步深入 **UMG 的控件和 UUserWidget**（怎么做血条、按钮、怎么绑定数据），这是你实际做 UI 的核心。
