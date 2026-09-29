# P03-2 — 建 `TenkaichiMainMenuWidget.cpp`（绑按钮点击 + 跳关卡）

> **对应 Lyra**：Lyra 用 CommonUI 的 `NativeOnInitialized` + ActivatableButton 体系绑点击。阶段一做减法，用引擎原生 `UButton::OnClicked` + `UGameplayStatics::OpenLevel`。
>
> **一句话**：实现 `.cpp`——在 `NativeConstruct` 里把 `StartButton->OnClicked` 绑到 `HandleStartClicked`，回调里调 `UGameplayStatics::OpenLevel` 跳转到战斗关卡。

---

## 一、这一小步要解决什么问题

上一步（P03-1）建好了 `.h`：声明了按钮指针 `StartButton`、初始化函数 `NativeConstruct`、点击回调 `HandleStartClicked`。

这一步实现它们：① 界面显示时（`NativeConstruct`）绑上点击；② 点击后跳关卡。

**验收**：`.cpp` 建好、整体编译通过。

---

## 二、两个引擎真实 API（先查源码）

### API ①：`UButton::OnClicked`（按钮点击委托）

```73:75:d:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\UMG\Public\Components\Button.h
	/** Called when the button is clicked */
	UPROPERTY(BlueprintAssignable, Category="Button|Event")
	FOnButtonClickedEvent OnClicked;
```

`OnClicked` 是一个**动态多播委托**（`DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnButtonClickedEvent)`，第 17 行）。动态委托绑定要用 `AddDynamic`，且回调函数必须标 `UFUNCTION()`（所以 `.h` 里 `HandleStartClicked` 才加 `UFUNCTION()`）。

### API ②：`UGameplayStatics::OpenLevel`（切换关卡）

```340:341:d:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Engine\Classes\Kismet\GameplayStatics.h
	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject", AdvancedDisplay = "2", DisplayName = "Open Level (by Name)"), Category="Game")
	static ENGINE_API void OpenLevel(const UObject* WorldContextObject, FName LevelName, bool bAbsolute = true, FString Options = FString(TEXT("")));
```

- 第 1 个参数 `WorldContextObject`：提供一个能拿到 World 的对象（传 `this` 即可）。
- 第 2 个参数 `LevelName`：目标关卡名（如战斗关卡 `L_Battle`）。

---

## 三、你要写的代码（`Source/Tenkaichi/UI/TenkaichiMainMenuWidget.cpp`）

```cpp
#include "TenkaichiMainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiMainMenuWidget)

void UTenkaichiMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 界面显示时，把按钮点击绑到回调（OnClicked 是动态委托，用 AddDynamic）
	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UTenkaichiMainMenuWidget::HandleStartClicked);
	}
}

void UTenkaichiMainMenuWidget::HandleStartClicked()
{
	// 点击"开始游戏"→ 跳转到战斗关卡（对应 GameplayStatics.h 第 340 行 OpenLevel）
	UGameplayStatics::OpenLevel(this, FName(TEXT("L_Battle")));
}
```

> `L_Battle` 是战斗关卡名，换成你实际存的战斗关卡名（P02 那个有角色站着的场景）。

---

## 四、逐段回扣"为什么"

| 代码 | 为什么这么写 |
|------|-------------|
| `#include "Components/Button.h"` | 要用 `UButton::OnClicked`，必须引 Button 完整定义 |
| `#include "Kismet/GameplayStatics.h"` | 要用 `UGameplayStatics::OpenLevel`，引它的头文件 |
| `Super::NativeConstruct()` | 先调基类，保证引擎的初始化流程走完（UE 覆写虚函数的规范） |
| `if (StartButton)` | 防御：万一蓝图里没放同名按钮，指针为空时不崩 |
| `OnClicked.AddDynamic(this, &...::HandleStartClicked)` | 动态委托绑定标准写法；`AddDynamic` 要求回调标 `UFUNCTION()`（.h 已标） |
| `OpenLevel(this, FName(TEXT("L_Battle")))` | `this` 提供 WorldContext；关卡名用 `FName(TEXT(...))` |

---

## 五、核对（.h ↔ .cpp 成对）

- `.h` 声明：`NativeConstruct()` / `HandleStartClicked()`
- `.cpp` 实现：`UTenkaichiMainMenuWidget::NativeConstruct()` / `UTenkaichiMainMenuWidget::HandleStartClicked()`

名字完全一致（配合 11 号铁律）。

---

## 六、我们这版 vs Lyra 的差异

| 项 | Lyra | 我们 | 说明 |
|---|------|------|------|
| 绑点击时机 | CommonUI `NativeOnInitialized` | 引擎 `NativeConstruct` | 都是"界面就绪后"，时机等价 |
| 按钮类型 | `UCommonButtonBase`（CommonUI） | `UButton`（引擎原生） | 做减法 |
| 跳关卡 | CommonUI 页面栈 Push/Pop | `UGameplayStatics::OpenLevel` | 阶段一用最直接的关卡切换 |

---

## 七、一句话结论

**`.cpp` 干两件事：`NativeConstruct` 里 `StartButton->OnClicked.AddDynamic(...)` 绑点击（OnClicked 是动态委托，`Button.h` 第 74 行）；`HandleStartClicked` 里 `UGameplayStatics::OpenLevel(this, "L_Battle")` 跳关卡（`GameplayStatics.h` 第 340 行）。回调必须 `UFUNCTION()` 才能 AddDynamic。**

---

## 八、下一步

**P03-2b：建 Widget 蓝图子类 + 放 `StartButton`**（编辑器手动，手把手教）——基于 `UTenkaichiMainMenuWidget` 建蓝图子类 `WBP_MainMenu`，在 Designer 拖一个 Button 命名 `StartButton`（对上 C++ 的 BindWidget）。**这步是下一步的前提**，详见 `P03-2b_建Widget蓝图子类放StartButton.md`。

> 之后再走 P03-3（新建地图 + 打光 + 铺地 + 设默认地图）、P03-4（在 GameMode 里 `CreateWidget` + `AddToViewport` 显示 `WBP_MainMenu`），最后验收：启动→主菜单→点开始→进战斗场景。
