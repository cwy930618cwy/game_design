# P03-4 — 把主菜单 Widget 显示到屏幕（实例化 + AddToViewport）

> **对应 Lyra**：Lyra 通过 Experience 的 "Add Widget" action 显示 UI（`LyraHUD.h` 第 17-18 行注释）。阶段一做减法，在 GameMode 里手动 `CreateWidget` + `AddToViewport`。
>
> **一句话**：① 建蓝图子类 `WBP_MainMenu`（放一个同名按钮，对上 `BindWidget`）；② 在 `TenkaichiGameMode` 里 `BeginPlay` 创建这个 Widget 并 `AddToViewport` 显示到屏幕。

---

## 一、这一小步要解决什么问题

P03-1/P03-2 建好了 Widget 的 C++ 类（绑点击 + 跳关卡）。但它**从没被显示出来**——现在进 `L_MainMenu` 只有黑画面，看不到主菜单。

这一步让 Widget **真正出现在屏幕上**。

**验收**：PIE 进 `L_MainMenu`，能看到主菜单 Widget（一个按钮）。

---

## 二、关键前提：为什么必须建蓝图子类（先讲清 BindWidget）

你的 `StartButton` 标了 `meta=(BindWidget)`（P03-1）。这条约定的完整含义：

- **C++ 基类**只声明"有个叫 StartButton 的按钮指针"，但**没有真的按钮控件**。
- **蓝图子类**里必须放一个**同名**（`StartButton`）的 Button 控件，引擎才会自动把控件绑到 C++ 指针上。
- 如果**直接用 C++ 类实例化**（没有蓝图子类），`StartButton` 指针是 **null** → 点击逻辑失效。

**所以**：先建蓝图子类 `WBP_MainMenu`（基于 `UTenkaichiMainMenuWidget`），在 Designer 里放一个命名 `StartButton` 的按钮。

---

## 三、两个引擎真实 API（已核实）

### API ①：`CreateWidget<T>`（创建一个 Widget 实例）

```1766:1767:d:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\UMG\Public\Blueprint\UserWidget.h
template <typename WidgetT = UUserWidget, typename OwnerType = UObject>
WidgetT* CreateWidget(OwnerType OwningObject, TSubclassOf<UUserWidget> UserWidgetClass = WidgetT::StaticClass(), FName WidgetName = NAME_None)
```

- 第 1 个参数 `OwningObject`：拥有者，传 **PlayerController**（UI 归某个玩家所有）。
- 第 2 个参数 `UserWidgetClass`：要创建的 Widget 类（填我们的 `WBP_MainMenu`）。

### API ②：`AddToViewport`（把 Widget 加到屏幕）

```340:341:d:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\UMG\Public\Blueprint\UserWidget.h
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="User Interface|Viewport", meta=( AdvancedDisplay = "ZOrder" ))
	UMG_API void AddToViewport(int32 ZOrder = 0);
```

- 创建出来的 Widget 默认不在屏幕上，必须调 `AddToViewport` 才显示。
- `ZOrder`：层叠顺序（多个 UI 谁在上层），默认 0 即可。

---

## 四、你要写的代码

### 4-1 改 `TenkaichiGameMode.h`（加一个 Widget 类属性 + BeginPlay）

```cpp
#pragma once

#include "GameFramework/GameMode.h"
#include "TenkaichiGameMode.generated.h"

class APawn;
class AController;
class UUserWidget;

UCLASS()
class ATenkaichiGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ATenkaichiGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// 游戏开始时，创建并显示主菜单 Widget
	virtual void BeginPlay() override;

	// 主菜单 Widget 类（编辑器里指定为 WBP_MainMenu）
	UPROPERTY(EditAnywhere, Category = "Tenkaichi|UI")
	TSubclassOf<UUserWidget> MenuWidgetClass;
};
```

### 4-2 改 `TenkaichiGameMode.cpp`（实现 BeginPlay）

在原有构造函数 + include 基础上，加：

```cpp
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

void ATenkaichiGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 只在本地玩家的客户端显示主菜单（GameMode 在服务器/客户端都有，UI 只需本地一份）
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (MenuWidgetClass)
		{
			UTenkaichiMainMenuWidget* MenuWidget = CreateWidget<UTenkaichiMainMenuWidget>(PC, MenuWidgetClass);
			if (MenuWidget)
			{
				MenuWidget->AddToViewport();
			}
		}
	}
}
```

> 需要在 .cpp 顶部 include `TenkaichiMainMenuWidget.h`（用到了它的类型）。

---

## 五、逐段回扣"为什么"

| 代码 | 为什么这么写 |
|------|-------------|
| `BeginPlay() override` | 游戏开始时触发（`AActor` 虚函数），此时 PlayerController 已就绪，适合创建 UI |
| `Super::BeginPlay()` | 先调基类，走完引擎流程（覆写虚函数规范） |
| `UGameplayStatics::GetPlayerController(GetWorld(), 0)` | 拿本地 0 号玩家控制器；Widget 要有 OwningObject 才能创建 |
| `if (MenuWidgetClass)` | 防御：没指定 Widget 类时不崩 |
| `CreateWidget<...>(PC, MenuWidgetClass)` | 创建 Widget 实例，owner = PC，类 = 蓝图子类 |
| `MenuWidget->AddToViewport()` | 加到屏幕才看得见 |
| `TSubclassOf<UUserWidget> MenuWidgetClass` | 暴露到编辑器，让你选 `WBP_MainMenu`（C++ 不能硬编码蓝图类引用） |

---

## 六、编辑器步骤（配合代码，缺一不可）

### ① 建蓝图子类 `WBP_MainMenu`
1. Content Browser 右键 → `User Interface → Widget Blueprint`
2. 选父类 **`TenkaichiMainMenuWidget`**（不是默认 UserWidget）
3. 命名 `WBP_MainMenu`，存到 `Content/UI/`

### ② 在 Designer 里放按钮（对上 BindWidget）
1. 打开 `WBP_MainMenu` → 切到 `Designer` 视图
2. 左侧 `Palette` 拖一个 **Button** 到画布
3. 选中这个 Button，右侧 `Details` 顶部把它的**名字改成 `StartButton`**（必须和 C++ 里的 `BindWidget` 名完全一致）
4. 可选：Button 里加个 Text 子控件，文字写"开始游戏"
5. 编译保存

### ③ 让 GameMode 用上这个蓝图类
1. 打开 `L_MainMenu` 关卡
2. `Window → World Settings → GameMode` 区域，或选中场景里的 GameMode（若用默认 GameMode 则在 Project Settings → Maps & Modes 里）
3. 找到 `Tenkaichi|UI` 分类下的 **`Menu Widget Class`**，选 `WBP_MainMenu`

> 若找不到该属性：说明 GameMode 用的是引擎默认实例，需要在关卡 World Settings 里把 GameMode Override 设成 `TenkaichiGameMode`，才能看到自定义属性。

---

## 七、核对（.h ↔ .cpp 成对）

- `.h` 声明：`BeginPlay()`
- `.cpp` 实现：`ATenkaichiGameMode::BeginPlay()`

名字一致（配合 11 号铁律）。

---

## 八、我们这版 vs Lyra 的差异

| 项 | Lyra | 我们 | 说明 |
|---|------|------|------|
| 显示 UI 的方式 | Experience 的 "Add Widget" action（GameFeature） | GameMode `BeginPlay` 手动 CreateWidget | 阶段一不碰 Experience |
| 创建时机 | Action 执行时 | `BeginPlay` | 都是"游戏开始、PC 就绪后" |
| Widget 基类 | `ULyraActivatableWidget`（CommonUI） | `UUserWidget` | 做减法 |

---

## 九、一句话结论

**显示 Widget = 建蓝图子类 `WBP_MainMenu`（放同名 `StartButton` 对上 BindWidget）+ 在 GameMode 的 `BeginPlay` 里 `CreateWidget(PC, MenuWidgetClass)` 创建、`AddToViewport()` 显示。`MenuWidgetClass` 是暴露到编辑器的类引用，用来选蓝图子类。C++ 不能硬编码蓝图类，所以用 TSubclassOf + 编辑器指定。**

---

## 十、下一步

**P03 收尾验收**：PIE 进 `L_MainMenu` → 看到主菜单 + "开始游戏"按钮 → 点按钮 → `OpenLevel` 跳到 `L_Battle`（角色自动站着）。阶段一完成。
