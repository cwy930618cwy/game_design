# 03 — SlateCore 与 UMG 模块详解

> **定位**：对应全景地图第 65-66 行的 `SlateCore`（Slate 核心）和 `UMG`（蓝图 UI 系统）两个模块。
>
> **一句话**：`SlateCore` = **UI 的最底层**（Widget 基础、绘制、字体、输入）；`UMG` = **游戏 UI 层**（控件、蓝图、数据绑定）。**SlateCore 是地基，UMG 是盖在上面的房子。**
>
> **文件**：`Engine/Source/Runtime/SlateCore/`、`Engine/Source/Runtime/UMG/`

---

## 一、先定位：这两个模块在整个 UI 里的位置

```
你的游戏 UI（UUserWidget，蓝图）
  └─ 依赖
UMG 模块（游戏 UI：控件/蓝图/绑定）
  └─ 依赖
Slate 模块（UI 引擎：绘制/布局）
  └─ 依赖
SlateCore 模块（最底层：Widget 基础/字体/输入）← 这篇讲它
```

| 模块 | 干嘛的 | 你日常 |
|------|--------|:---:|
| **SlateCore** | UI 最底层（Widget 基础、字体、输入、绘制） | 不直接碰 |
| **Slate** | UI 引擎（编辑器界面） | 做编辑器才碰 |
| **UMG** | 游戏 UI（控件、蓝图、绑定） | **天天用** |

---

## 二、SlateCore —— UI 的最底层

### 2.1 是什么

SlateCore 是**整个 UI 系统的地基**。它提供：
- **Widget（控件）的基类**（所有 UI 控件的根）
- 字体、文本渲染
- 输入处理
- 布局
- 绘制、样式

**所有 UI（Slate 和 UMG）都建立在 SlateCore 之上。**

### 2.2 实际目录（我查了真实结构）

```
SlateCore/Public/
├── Widgets/      ← 控件基础（SWidget 基类）
├── Fonts/        ← 字体
├── Input/        ← 输入处理
├── Layout/       ← 布局
├── Rendering/    ← 绘制
├── Styling/      ← 样式（FSlateBrush 等）
├── Types/        ← 基础类型
├── Application/  ← 应用层
└── Brushes/      ← 画笔（绘制用的图）
```

**重点**：`Widgets/`（SWidget 基类）是**所有控件的根**。

### 2.3 SWidget —— 所有控件的根

```cpp
// SlateCore 提供 SWidget（所有 Slate 控件的基类）
// UMG 和 Slate 的控件最终都基于它
class SWidget {
    // 绘制、布局、输入的基础
};
```

**理解**：你用的 `UButton`、`UImage`，底层最终都来自 SlateCore 的 `SWidget`。

---

## 三、UMG —— 游戏 UI 层

### 3.1 是什么

UMG（Unreal Motion Graphics）是**游戏 UI 系统**，让你用蓝图做 UI。核心：
- **控件（UWidget）**：Button/Image/TextBlock
- **UUserWidget**：UI 基类
- **数据绑定**：BindWidget
- **动画**：Widget Animation

### 3.2 实际目录（我查了真实结构）

```
UMG/Public/
├── Components/    ← 控件（81 个文件，最大）
│   ├── Button.h / Image.h / TextBlock.h ...
├── Binding/       ← 数据绑定
├── Blueprint/     ← 蓝图支持
├── Animation/     ← UI 动画
├── Slate/         ← UMG 和 Slate 的桥接
└── Extensions/    ← 扩展
```

**重点**：`Components/`（81 个控件文件）是你做 UI 的核心——所有控件类型都在这。

### 3.3 核心控件（Components/ 里）

| 控件 | 干嘛 |
|------|------|
| `UButton` | 按钮 |
| `UImage` | 图片 |
| `UTextBlock` | 文本 |
| `UProgressBar` | 血条/进度 |
| `UVerticalBox` / `UHorizontalBox` | 布局 |
| `UCanvasPanel` | 自由摆放 |
| `UWidget` | 所有控件基类 |

```cpp
// 你用这些控件（实际在 UMG/Components/ 里）
UProgressBar* HealthBar;
HealthBar->SetPercent(0.5f);   // 血条设一半
```

---

## 四、SlateCore / Slate / UMG 三者关系（再巩固）

```
┌──────────────────────────────────┐
│    UMG（游戏 UI）               │
│  UButton UImage UProgressBar    │
│  （你用的控件，蓝图友好）        │
├──────────────────────────────────┤
│    Slate（UI 引擎）             │
│  SButton SImage（C++ 控件）     │
│  编辑器界面用它                  │
├──────────────────────────────────┤
│   SlateCore（最底层）           │
│  SWidget 基类/字体/输入/绘制    │
│  所有 UI 的地基                  │
└──────────────────────────────────┘
```

**依赖链**：UMG → Slate → SlateCore（从上层到最底层）。

---

## 五、你日常该用什么（结论）

| 模块 | 你日常用吗 |
|------|:---:|
| SlateCore | ❌ 不用碰（是地基） |
| Slate | ❌ 做编辑器才用 |
| UMG | ✅ **天天用**（做游戏 UI） |

**结论**：
- **做游戏 UI 用 UMG**（控件 + 蓝图 + UUserWidget）
- **SlateCore / Slate 是底层**，理解它们是什么即可，不用直接写
- SlateCore 的价值：让你知道"UI 的地基是 SWidget"

---

## 六、常见陷阱

**① 以为 SlateCore 和 UMG 是平级的**
```cpp
// ❌ 不是平级，是层级
// ✅ SlateCore（地基）→ Slate（引擎）→ UMG（游戏 UI）
```

**② 以为做游戏 UI 要学 SlateCore**
```cpp
// ❌ 做游戏 UI 不用写 SlateCore
// ✅ 用 UMG 蓝图就够，SlateCore 是编辑器/进阶
```

**③ 分不清 UButton（UMG）和 SButton（Slate）**
```cpp
// UButton = UMG 控件（你用的，蓝图友好）
// SButton = Slate 控件（底层，编辑器用）
```

---

## 七、总结速查

```
SlateCore（最底层）：SWidget 基类、字体、输入、绘制、布局
  └─ 所有 UI 的地基，你日常不直接碰

Slate（UI 引擎）：SButton 等 C++ 控件，编辑器界面用

UMG（游戏 UI）：UButton/UImage/UProgressBar，蓝图友好
  └─ 你日常做游戏 UI 用这个

关系：UMG → Slate → SlateCore（从上层到地基）
```

**一句话**：`SlateCore` 是 **UI 最底层**（SWidget 基类、字体、输入、绘制），`UMG` 是**游戏 UI 层**（控件、蓝图、绑定）。**SlateCore 是地基，UMG 是盖在上面的房子**。你日常做游戏 UI 用 UMG，SlateCore 理解是什么即可。

---

## 八、下一步

理解了 SlateCore / Slate / UMG 的分工，下一步深入 **UMG 的核心——UUserWidget 和控件**（怎么做血条/按钮、怎么绑定数据、怎么加动画），这是你实际做 UI 的核心。
