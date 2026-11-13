# 02 — CommonUI 详解（高级游戏 UI 框架）

> **定位**：CommonUI 是 UE 的一个**游戏 UI 框架插件**——比基础 UMG 更高级，专为**现代游戏 UI**（手柄导航、输入切换、弹出窗口）设计。
>
> **一句话**：`CommonUI` = **UMG 的"高级版"**。它解决 UMG 做现代游戏 UI 的痛点——**手柄/键盘/鼠标切换、焦点导航、弹出 UI、UI 输入模式**。做现代游戏（尤其主机/手柄）UI 必备。
>
> **文件**：`Engine/Plugins/Runtime/CommonUI/`

---

## 一、先搞清：CommonUI 和 UMG 什么关系

```
CommonUI（高级框架，基于 UMG）
  └─ 是 UMG 的"增强版"（不是替代品）
       ├─ 用 UMG 控件做基础
       └─ 加了：手柄导航、输入切换、弹出UI等

UMG（基础 UI）
  └─ 你之前学的，做血条/按钮/菜单
```

| | UMG | CommonUI |
|---|---|---|
| 是什么 | 基础 UI | 高级游戏 UI 框架 |
| 有没有 | 引擎自带 | 插件（需启用） |
| 适用 | 简单 UI | **现代游戏 UI（手柄/主机）** |
| 多了啥 | - | 手柄导航、输入切换、弹出UI |

**一句话**：CommonUI 是**基于 UMG 的高级框架**，解决现代游戏 UI 的痛点。**不是替代 UMG，是在 UMG 上加东西。**

---

## 二、CommonUI 解决的核心痛点（配场景）

### ① 输入切换（键盘/鼠标 ↔ 手柄）

**痛点**：玩家插上手柄，UI 要自动从"鼠标点击"切到"手柄按键导航"。

```cpp
// CommonUI 的 InputAction / 输入模式
// 手柄插入 → UI 自动显示手柄提示（按键图标）
// 键盘使用 → UI 自动切回键盘提示
```

**场景**：PC 游戏同时支持键盘和手柄，UI 提示图标要跟着切换。

### ② 焦点导航（手柄方向键移动选中项）

**痛点**：手柄没有鼠标，得用方向键在按钮间移动。

```
CommonUI 按钮（CommonButtonBase）
  ├─ 手柄按"上/下/左/右" → 焦点在按钮间移动
  └─ 按"确认" → 激活按钮
```

**场景**：主机游戏菜单，手柄方向键选菜单项。

### ③ 弹出 UI（Popup）和 UI 层级

**痛点**：弹窗、暂停菜单、设置界面互相叠加，要管理层级。

```cpp
// CommonUI 的 Activatable Widget
// 能管理 UI 的"激活/关闭"栈
// 打开设置 → 暂停游戏 + 弹窗在最上层
```

**场景**：按 ESC 弹暂停菜单，再开设置界面，层层叠加，按返回逐层关闭。

### ④ UI 输入模式（UI 要不要吃输入）

**痛点**：显示 UI 时，输入是给 UI 还是给游戏？

```cpp
// 显示主菜单 → UI 吃所有输入（不控制角色）
// 关闭菜单 → 输入回游戏（控制角色）
```

**场景**：开菜单时角色不能动，关菜单后能控制。

---

## 三、CommonUI 核心类（重点）

| 类 | 作用 |
|------|------|
| `UCommonActivatableWidget` | 可激活/关闭的 UI（弹出/层级管理） |
| `UCommonButtonBase` | 高级按钮（手柄导航） |
| `UCommonTextBlock` | 高级文本 |
| `UCommonBoundActionButton` | 绑定按键的按钮 |
| `UCommonInputSubsystem` | 输入切换管理 |

**场景：做一个手柄可导航的主菜单**

```cpp
// 继承 CommonActivatableWidget（能管理激活/关闭）
UCLASS()
class UMainMenuWidget : public UCommonActivatableWidget {
    GENERATED_BODY()
public:
    // 手柄能导航的按钮
    UPROPERTY(meta=(BindWidget))
    UCommonButtonBase* StartButton;

    // 打开时（激活）
    virtual void NativeOnActivated() override {
        Super::NativeOnActivated();
        StartButton->SetFocus();   // 手柄默认选中开始按钮
    }
};
```

---

## 四、CommonUI 怎么配合 GameInstance（输入）

CommonUI 常和 **CommonInputSubsystem** 配合管理输入模式：

```cpp
// 切换输入模式（UI 显示时）
UCommonInputSubsystem& Input = UCommonInputSubsystem::Get(GetLocalPlayer());
Input.SetInputMethod(ECommonInputType::Gamepad);   // 切到手柄输入
```

---

## 五、什么时候用 CommonUI（该不该学）

| 你的游戏 | 用不用 CommonUI |
|---------|:---:|
| 手游（触屏） | 看情况（触屏 UI 不一定需要） |
| PC 键鼠游戏 | 可选 |
| **主机游戏 / PC+手柄** | **强烈推荐**（手柄导航必备） |
| **现代 3A 游戏 UI** | **推荐**（输入切换/弹出UI） |

**结论**：
- 做**现代游戏 UI（尤其手柄/主机）** → 用 CommonUI
- 做简单 PC/手游 UI → 基础 UMG 就够
- **它是 UMG 的增强，不是替代**

---

## 六、常见陷阱

**① 以为 CommonUI 替代 UMG**
```cpp
// ❌ CommonUI 不是替代，是基于 UMG
// ✅ 它是 UMG 的高级框架，控件仍是 UMG 那套
```

**② 忘了启用插件**
```cpp
// CommonUI 是插件，要在项目里启用才能用
// 编辑 → Plugins → Runtime → CommonUI → 启用
```

**③ 做简单 UI 也用 CommonUI（过度设计）**
```cpp
// ❌ 一个血条也用 CommonUI，没必要
// ✅ 简单 UI 用基础 UMG，复杂游戏 UI 才用 CommonUI
```

---

## 七、总结速查

```
CommonUI = 基于 UMG 的高级游戏 UI 框架
解决痛点：
  ① 输入切换（键鼠 ↔ 手柄）
  ② 焦点导航（手柄方向键选按钮）
  ③ 弹出 UI 层级管理
  ④ UI 输入模式

核心类：
  UCommonActivatableWidget（激活/关闭）
  UCommonButtonBase（高级按钮）
  UCommonInputSubsystem（输入切换）

适用：现代游戏 UI / 主机手柄 UI
不用：简单 PC/手游 UI（用基础 UMG 就够）
```

**一句话**：CommonUI 是**基于 UMG 的高级游戏 UI 框架**，专为**手柄导航、输入切换、弹出 UI** 设计。做现代游戏 UI（尤其主机/手柄）用它对，做简单 UI 用基础 UMG 就够。**它是 UMG 的增强版，不是替代品。**

---

## 八、完整案例：手柄可导航的主菜单（怎么实际用）

> 一个能直接跑起来的完整案例，从建 Widget 到显示，一步步讲。**展示 CommonUI 实际怎么用。**

**场景**：做一个主菜单，支持手柄方向键导航 + 键鼠点击。

### 第 1 步：创建 Widget Blueprint

```
Content Browser → 右键 → User Interface → Widget Blueprint
→ 命名为 UI_MainMenu
→ 父类选 CommonActivatableWidget（关键！）
```

> **为什么不选普通 UserWidget**：`CommonActivatableWidget` 才支持"激活/关闭管理 + 手柄导航"。

### 第 2 步：在蓝图里拖 CommonButtonBase 按钮

```
拖一个 VerticalBox（竖排）
  ├─ CommonButtonBase → StartButton（开始游戏）
  ├─ CommonButtonBase → OptionsButton（选项）
  └─ CommonButtonBase → QuitButton（退出）
```

> **关键**：用 `CommonButtonBase`（不是普通 Button），才能手柄方向键导航。

### 第 3 步：C++ 继承 CommonActivatableWidget 并绑定按钮

```cpp
// MainMenuWidget.h
#pragma once
#include "CommonActivatableWidget.h"
#include "MainMenuWidget.generated.h"

class UCommonButtonBase;

UCLASS()
class UMainMenuWidget : public UCommonActivatableWidget {
    GENERATED_BODY()
public:
    UPROPERTY(meta=(BindWidget))    // 绑定蓝图里同名按钮
    UCommonButtonBase* StartButton;

    UPROPERTY(meta=(BindWidget))
    UCommonButtonBase* OptionsButton;

protected:
    virtual void NativeOnActivated() override;   // 激活时调用
    virtual void NativeConstruct() override;     // 绑定点击
};
```

```cpp
// MainMenuWidget.cpp
#include "MainMenuWidget.h"
#include "CommonButtonBase.h"

void UMainMenuWidget::NativeConstruct() {
    Super::NativeConstruct();
    StartButton->OnClicked().AddUObject(this, &UMainMenuWidget::StartGame);
    OptionsButton->OnClicked().AddUObject(this, &UMainMenuWidget::OpenOptions);
}

void UMainMenuWidget::NativeOnActivated() {
    Super::NativeOnActivated();
    StartButton->SetFocus();   // 手柄默认选中"开始游戏"
}

void UMainMenuWidget::StartGame() {
    UGameplayStatics::OpenLevel(this, TEXT("Level_1"));   // 开始游戏
}

void UMainMenuWidget::OpenOptions() {
    // 打开设置界面（Push 到 UI 栈）
}
```

### 第 4 步：在 PlayerController 里显示主菜单

```cpp
UCLASS()
class AMyPlayerController : public APlayerController {
    GENERATED_BODY()
public:
    virtual void BeginPlay() override {
        Super::BeginPlay();
        // 创建主菜单
        UMainMenuWidget* Menu = CreateWidget<UMainMenuWidget>(this, MenuClass);
        Menu->AddToViewport();                 // 显示
        bShowMouseCursor = true;               // PC 也能鼠标点
        FInputModeUIOnly Mode;                  // UI 吃所有输入（角色不能动）
        Mode.SetWidgetToFocus(Menu->TakeWidget());
        SetInputMode(Mode);
    }
};
```

### 完整流程总结

```
1. 创建 Widget Blueprint（父类 CommonActivatableWidget）
2. 拖 CommonButtonBase 按钮（手柄可导航）
3. C++ 继承 + BindWidget 绑定按钮
   → NativeOnActivated 设手柄默认焦点
   → OnClicked 绑定点击逻辑
4. PlayerController 里 CreateWidget + AddToViewport + 设 UI 输入模式
```

**效果**：
- **手柄玩家**：按"上/下"方向键在 开始/选项/退出 间移动，按"确认"激活
- **键鼠玩家**：鼠标直接点按钮
- 打开后**角色不能动**（UI 吃输入）

### CommonUI 在这起到的作用

| CommonUI 干了啥 | 对应代码 |
|----------------|---------|
| 手柄能导航按钮 | 用 `CommonButtonBase`（不是普通 Button） |
| 管理 UI 激活/关闭 | 继承 `CommonActivatableWidget` |
| 手柄默认选中 | `NativeOnActivated` 里 `SetFocus()` |
| UI 吃输入 | `FInputModeUIOnly` + `SetInputMode` |

---

## 九、下一步

理解了 CommonUI，接下来可以深入 **UMG 基础控件 + UUserWidget**（怎么做血条/按钮/绑定数据），这是无论用不用 CommonUI 都要掌握的 UI 核心。
