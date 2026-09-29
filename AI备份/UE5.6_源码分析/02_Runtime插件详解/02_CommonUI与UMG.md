# 02 - CommonUI 与 UMG（UI 框架）

> 涉及插件：`CommonUI` (306 文件) + `UMG` (引擎模块) + `Slate/SlateCore`
> Lyra 使用度：⭐⭐⭐ **核心**（整个前端大厅 + 游戏内 HUD 都用它）

---

## 一、UI 技术栈分层

UE 的 UI 有三层，从下到上：

```
┌─────────────────────────────────────┐
│   UMG (Widget Blueprint)            ← 美术/策划用的可视化控件
├─────────────────────────────────────┤
│   CommonUI (跨平台 UI 框架)          ← Lyra 的核心 UI 架构
├─────────────────────────────────────┤
│   Slate (底层 C++ UI 框架)           ← 编辑器 + 引擎内部 UI
└─────────────────────────────────────┘
```

| 层 | 使用者 | 特点 |
|----|--------|------|
| **Slate** | 程序员 | 声明式 C++ UI，性能最好 |
| **UMG** | 美术/策划 | 可视化编辑，Widget Blueprint |
| **CommonUI** | 游戏框架 | 跨平台输入导航、焦点管理、层级堆栈 |

---

## 二、CommonUI 是什么？

**CommonUI 是 Lyra 的 UI 架构核心**，解决三个问题：
1. **跨平台输入** — 手柄/键盘/鼠标统一导航
2. **UI 层级管理** — 类似移动端的 Fragment 栈
3. **输入模式切换** — 游戏时隐藏光标，菜单时显示光标

### 2.1 核心概念

#### ActivatableWidget（可激活控件）
继承 `UCommonActivatableWidget`，每个界面都是一个可激活控件：

```cpp
// Lyra 里的界面
W_LyraHUD              // 游戏内 HUD
W_MainMenu             // 主菜单
W_Settings             // 设置界面
W_PauseMenu            // 暂停菜单
```

#### UI Stack（UI 栈）
界面像手机 App 一样**压栈/弹栈**：

```cpp
// 推入一个新界面
UIManager->PushContentToLayerForPlayer(PlayerIndex, WidgetClass);

// 弹出当前界面
UIManager->PopContentFromLayerForPlayer(PlayerIndex);
```

#### Input Action Routing
根据当前焦点自动路由输入：

```cpp
// 游戏中 →  WASD 移动角色
// 菜单中  →  WASD 导航焦点
// CommonUI 自动处理这个切换
```

### 2.2 目录结构

```
CommonUI/
├── Source/
│   ├── CommonUI/              ← 主模块
│   │   ├── Public/
│   │   │   ├── CommonActivatableWidget.h      ← 可激活控件基类
│   │   │   ├── CommonUserWidget.h             ← 用户控件
│   │   │   ├── CommonActionWidget.h           ← 按键提示控件
│   │   │   ├── CommonInputSubsystem.h         ← 输入子系统
│   │   │   └── CommonUISettings.h             ← 设置
│   │   └── Private/
│   └── CommonUIEditor/        ← 编辑器扩展
└── CommonUI.uplugin
```

---

## 三、UMG — Widget Blueprint

### 3.1 核心类

| 类 | 作用 |
|----|------|
| `UUserWidget` | 控件基类 |
| `UWidget` | 所有控件的基类 |
| `UPanelWidget` | 面板控件（可容纳子控件） |
| `UImage` | 图片 |
| `UTextBlock` | 文本 |
| `UButton` | 按钮 |
| `UCanvasPanel` | 画布面板 |
| `UVerticalBox` / `UHorizontalBox` | 布局盒 |
| `UScrollBox` | 滚动盒 |
| `UListView` / `UTreeView` | 列表视图 |

### 3.2 生命周期

```cpp
virtual void NativeConstruct();    // 构造（BeginPlay 等价）
virtual void NativeDestruct();     // 销毁（EndPlay 等价）
virtual void NativeTick(FGeometry, float);  // 每帧
```

### 3.3 数据绑定

```cpp
// 属性绑定
UPROPERTY(meta=(BindWidget))
UTextBlock* HealthText;   // 自动绑定到名为 "HealthText" 的控件

// 委托绑定
UPROPERTY()
FOnButtonClickedEvent OnDamageClicked;
```

---

## 四、Slate — 底层框架

### 4.1 核心特点
- **声明式** — 用链式调用描述 UI
- **高性能** — 自绘，不依赖系统控件
- **编辑器全用它** — 你看到的编辑器 UI 都是 Slate

### 4.2 示例

```cpp
SNew(SVerticalBox)
+ SVerticalBox::Slot()
[
    SNew(STextBlock).Text(FText::FromString("Hello"))
]
+ SVerticalBox::Slot()
[
    SNew(SButton).OnClicked(this, &MyClass::OnButtonClicked)
    [
        SNew(STextBlock).Text(FText::FromString("Click Me"))
    ]
]
```

### 4.3 常用控件

| 控件 | 作用 |
|------|------|
| `SWidget` | 基类 |
| `SCompoundWidget` | 复合控件 |
| `SBox` / `SVerticalBox` / `SHorizontalBox` | 布局 |
| `SButton` / `STextBlock` / `SEditableTextBox` | 基础 |
| `SListView` / `STreeView` / `SComboBox` | 列表 |
| `SDockTab` / `SDockStack` | 停靠标签 |

---

## 五、Lyra 中的 UI 架构

### 5.1 层级划分

```
Root Layout (W_RootLayout)
├── Menu Layer (菜单层)
│   ├── W_MainMenu
│   ├── W_Settings
│   └── W_PauseMenu
├── Game Layer (游戏层)
│   └── W_LyraHUD
│       ├── W_HealthBar
│       ├── W_AmmoCounter
│       └── W_Crosshair
└── Modal Layer (模态层)
    └── W_ConfirmDialog
```

### 5.2 消息驱动 UI

Lyra 用 **GameplayMessageRouter** 解耦 UI 和游戏逻辑：

```cpp
// 游戏逻辑发消息
MessageSystem->BroadcastMessage(MSGKEY("Health.Changed"), FHealthMessage{NewValue});

// UI 监听消息更新
MessageSystem->RegisterListener<FHealthMessage>(MSGKEY("Health.Changed"), 
    this, &ThisClass::OnHealthChanged);
```

### 5.3 HUD Layout

`ULyraHUDLayout` 是游戏内 HUD 的容器：

```cpp
// 通过 GameplayCue/UIExtension 动态添加控件
UIExtensionSystem->AddWidgetExtension(HUDLayout, "HealthBar", HealthBarWidget);
```

---

## 六、学习建议

1. **先理解三层关系** — Slate → UMG → CommonUI
2. **看 Lyra 的 W_MainMenu** — 理解 ActivatableWidget 用法
3. **跟踪一个完整流程** — 如"打开设置 → 修改音量 → 保存"
4. **动手实践** — 用 CommonUI 做一个新界面

## 七、下一步

- [01_GameplayAbilities_GAS](./01_GameplayAbilities_GAS技能系统.md) — GAS 技能系统
- [03_ModularGameplay组件化](./03_ModularGameplay组件化.md) — 角色组件化
- [00_插件体系总览](./00_插件体系总览.md) — 回到总览
