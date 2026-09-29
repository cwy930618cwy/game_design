# P03-1 — 建 `TenkaichiMainMenuWidget.h`（主菜单 Widget 基类头文件）

> **对应 Lyra**：Lyra 的 UI 基类是 `ULyraActivatableWidget`（CommonUI 体系，`Plugins/GameFeatures/ShooterCore/...`），**绑死 CommonUI + Experience**（阶段六/七才教）。阶段一做减法，直接用引擎原生 `UUserWidget`。
>
> **一句话**：建一个 C++ 的 Widget 基类 `UTenkaichiMainMenuWidget`，继承引擎 `UUserWidget`，声明一个按钮成员（将来蓝图里同名按钮会自动绑上来）+ override `NativeConstruct`（用来在界面显示时绑按钮点击）。本小步只写 `.h`。

---

## 一、这一小步要解决什么问题

P02 结束，一按 Play 就直接进战斗场景、角色自动站着。但缺一个"开始游戏"的入口界面。

这一步建主菜单 Widget 的 C++ 基类 `.h`——先声明"这个界面有一个按钮、界面显示时要做一次初始化"。按钮的点击逻辑（跳关卡）在 P03-2 的 `.cpp` 里写。

**验收**：`.h` 建好、能编译（配合 P03-2 的 `.cpp` 后整体通过）。

---

## 二、命名：对应 Lyra（做减法的说明）

| Lyra 真实 | Tenkaichi 对应 | 说明 |
|-----------|---------------|------|
| `ULyraActivatableWidget`（CommonUI 插件） | `UTenkaichiMainMenuWidget`（引擎 `UUserWidget`） | Lyra 的 Widget 基类绑死 CommonUI；阶段一先用引擎原生 `UUserWidget`，聚焦"按钮 + 跳关卡" |
| 路径 `Plugins/GameFeatures/...` | `Source/Tenkaichi/UI/` | 我们的 UI 放 `UI/` 目录 |

> ⚠️ 这是"简化外围基础设施"（去掉 CommonUI），不是"简化核心范式"——Widget + 按钮回调 + OpenLevel 这条核心链路完整保留。CommonUI 深度内容留到阶段六（P13-P14）。

---

## 三、三个关键点（先讲清为什么，再给代码）

### 关键点 ①：继承引擎原生 `UUserWidget`

`UUserWidget` 是 UMG 里"一个界面"的 C++ 基类（引擎 `UMG/Public/Blueprint/UserWidget.h`）。我们的主菜单就是它的一个子类。

### 关键点 ②：按钮成员用 `UPROPERTY(meta=(BindWidget))`

这是 UMG 的核心约定。声明一个 `UButton*` 成员并标 `meta=(BindWidget)`：

- 编译后，**蓝图里只要有一个同名控件，引擎会自动把它绑到这个 C++ 指针上**（不用手写 `GetWidgetFromName`）。
- 所以按钮的"长相/位置"在蓝图里摆，C++ 只负责"拿到它、绑它的点击"。

### 关键点 ③：override `NativeConstruct`（界面显示时触发）

引擎源码核实，`UUserWidget` 的生命周期虚函数：

```1570:1573:d:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\UMG\Public\Blueprint\UserWidget.h
	UMG_API virtual void NativeOnInitialized();
	UMG_API virtual void NativePreConstruct();
	UMG_API virtual void NativeConstruct();
	UMG_API virtual void NativeDestruct();
```

`NativeConstruct` 在**界面被创建并添加到视口后**触发——此时按钮指针已被 `BindWidget` 填好，正好在这里绑 `OnClicked`（P03-2 的事）。

---

## 四、你要写的代码（`Source/Tenkaichi/UI/TenkaichiMainMenuWidget.h`）

```cpp
#pragma once

#include "Blueprint/UserWidget.h"
#include "TenkaichiMainMenuWidget.generated.h"

class UButton;

UCLASS()
class UTenkaichiMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 界面显示时触发，用来绑按钮点击（对应 UserWidget.h 第 1572 行 NativeConstruct）
	virtual void NativeConstruct() override;

	// 按钮成员：标 BindWidget，蓝图里同名按钮会自动绑上来
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> StartButton;

	// 按钮点击的回调函数（P03-2 里实现：调 OpenLevel 跳关卡）
	UFUNCTION()
	void HandleStartClicked();
};
```

---

## 五、逐段回扣"为什么"

| 代码 | 为什么这么写 |
|------|-------------|
| `#include "Blueprint/UserWidget.h"` | 要继承 `UUserWidget`，得先引基类头文件 |
| 前置声明 `class UButton;` | 头文件里只用到指针，前置声明即可 |
| `NativeConstruct() override` | 界面显示时触发，在这里绑按钮点击（此时按钮指针已填好） |
| `UPROPERTY(meta=(BindWidget)) UButton* StartButton` | BindWidget 约定：蓝图里同名按钮自动绑到这个指针，C++ 直接能用 |
| `UFUNCTION() HandleStartClicked()` | 按钮点击的回调；`UFUNCTION()` 是 `AddDynamic` 绑定的前提（反射需要） |

---

## 六、核对预告（.h ↔ .cpp 成对）

这个 `.h` 声明了 **2 个函数**：
1. `NativeConstruct()`
2. `HandleStartClicked()`

P03-2 的 `.cpp` 会一一实现这 2 个（配合 11 号铁律）。

---

## 七、我们这版 vs Lyra 的差异

| 项 | Lyra | 我们 | 说明 |
|---|------|------|------|
| 基类 | `ULyraActivatableWidget`（CommonUI） | `UUserWidget`（引擎原生） | 阶段一不碰 CommonUI |
| 按钮绑定 | CommonUI 的 ActivatableButton | 引擎原生 `UButton` + `BindWidget` | 用最基础的 UMG |
| 界面栈管理 | CommonUI Push/Pop 页面栈 | 无（就一个主菜单） | 阶段六再上 CommonUI |

---

## 八、一句话结论

**主菜单 Widget `.h` = 类名 `UTenkaichiMainMenuWidget` + 继承引擎 `UUserWidget` + 一个 `BindWidget` 按钮指针 `StartButton` + `NativeConstruct` override（绑点击）+ `HandleStartClicked` 回调声明。按钮长相在蓝图摆，C++ 只拿指针 + 绑逻辑。**

---

## 九、下一步

**P03-2：写 `Source/Tenkaichi/UI/TenkaichiMainMenuWidget.cpp`**——在 `NativeConstruct` 里把 `StartButton->OnClicked` 绑到 `HandleStartClicked`，并在回调里调 `UGameplayStatics::OpenLevel` 跳转到战斗关卡。
