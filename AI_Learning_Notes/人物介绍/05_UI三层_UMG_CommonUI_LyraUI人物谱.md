# UI 三层：UMG / CommonUI / LyraUI 人物谱

> **定位**：前面几篇讲的都是"台上演员怎么动"（角色/技能/输入）。但一出戏还得有**布景、道具、字幕、幕布**——这就是 **UI**。本文讲清 Lyra 里 UI 的**三层楼**：最底下的 **UMG**、中间的 **CommonUI**、最上面的 **LyraUI**，它们各干什么、代码上差在哪。
>
> **一句话**：**UMG** 是"造景片的木工坊"（能显示一块板子+基础控件）；**CommonUI** 是"舞台监督"（管这块板子什么时候能点、焦点在哪、手柄/键鼠怎么切、菜单怎么一层层叠）；**LyraUI** 是"本剧的舞美组"（按这出戏的具体规矩，把 CommonUI 用成本剧的 HUD/暂停/大厅/手柄断连提示）。

---

## 一、先认识 3 位"布景师傅"（延续剧院世界观）

| 角色 | 技术对应 | 一句话职责 | 层次 |
|------|---------|-----------|------|
| 🧱 **木工坊** | **UMG**（`UUserWidget` / `UWidget` / `UWidgetTree` / `UPanelWidget`） | 造出"一块能显示的板子"和上面的基础控件：按钮、文字、图片、血条、进度条。**它只管'画出来'，不管'谁在操作'。** | 底层 |
| 🎛️ **舞台监督** | **CommonUI**（`UCommonActivatableWidget` / `UPrimaryGameLayout` / `UCommonUIExtensions` / `UCommonInputSubsystem`） | 在木工坊之上，管**哪块板子现在能接收操作、焦点停在哪个按钮上、手柄/键鼠/触摸怎么切换、菜单一层层怎么叠、弹窗怎么模态**。 | 中间层 |
| 🎭 **本剧舞美组** | **LyraUI**（`ULyraActivatableWidget` / `ULyraHUDLayout` / `ULyraUIMessaging` / `GameFeatureAction_AddWidget` / `ULyraFrontendStateComponent`） | Lyra 项目**自己**写的 UI 代码，把 CommonUI 用成本剧的习惯：HUD 布局、按 Esc 弹暂停、手柄断连弹提示、大厅流程、按钮按输入设备换皮肤。 | 项目层 |

> **核心认知**：这三层是**继承/叠加**关系，不是并列。LyraUI 的控件**继承自** CommonUI 的控件，CommonUI 的控件**继承自** UMG 的控件。越往上越"懂游戏"，越往下越"懂显示"。

---

## 二、三层继承链（一张图看懂"谁是谁的爹"）

```
UObject
  └─ UVisual
       └─ UWidget                          ┐
            └─ UUserWidget                 │  🧱 UMG（引擎底座，管"显示"）
                 └─ UCommonUserWidget      ┐│
                      └─ UCommonActivatableWidget  │  🎛️ CommonUI（框架层，管"操作/导航/多设备"）
                           └─ ULyraActivatableWidget   │  🎭 LyraUI（项目层，管"本剧的规矩"）
                                └─ ULyraHUDLayout        ┘     └─ ULyraSettingScreen / 各种屏幕
```

**读法**：
- 最底下 `UUserWidget` 是 UMG 的当家——你在编辑器里拖控件、绑数据、播动画，靠的都是它。
- 往上一层 `UCommonActivatableWidget` 是 CommonUI 的当家——它给 UMG 加了"**可激活 / 可推进层栈 / 可配置输入模式**"这些"游戏菜单才需要的能力"。
- 再往上 `ULyraActivatableWidget` 是 Lyra 的当家——它只比父类多干一件事：**根据当前是游戏还是菜单，决定按键还要不要传给游戏**（见第四节代码）。

> **证据**：打开 `UI/LyraActivatableWidget.h`，第一行继承就是 `public UCommonActivatableWidget`；而 `UCommonActivatableWidget` 的头文件里继承自 `UCommonUserWidget`（属于 CommonUI 插件）。

---

## 三、故事：台上要显示一个"开始游戏"按钮

假设你要在屏幕上放一个按钮，让玩家按了开始游戏。三层师傅依次上手：

```
🎮 玩家需求："我要一个'开始游戏'按钮！"
        │
        ▼
① 🧱 木工坊（UMG）先干活：
   "我给你造一块板子（UUserWidget），上面放一个按钮控件（UButton）、一行文字（UTextBlock）。"
   —— 现在这块板子能'显示'出来了，也能点。
   —— 但问题来了：手柄的焦点光标停哪？按方向键焦点怎么移？同时按了游戏操作和菜单怎么办？
        │
        ▼
② 🎛️ 舞台监督（CommonUI）接手：
   "板子归我管。我把它推进'层栈'（UPrimaryGameLayout 的某一层），
    让它成为'当前激活的板子'，焦点自动落到第一个可聚焦控件上。
    玩家用手柄时，我管方向键在按钮间移动焦点；
    按返回键，我管'回退到上一层'。"
        │
        ▼
③ 🎭 本剧舞美组（LyraUI）最后定规矩：
   "这出戏的'开始游戏'属于大厅界面，归 UI.Layer.Menu 层。
    按 Esc 要弹暂停菜单；手柄突然断连要弹'请重连'提示；
    按钮在手柄模式下显示'按 A 开始'，键鼠模式下显示'点击开始'。"
```

**三层各管一段**：UMG 负责"画出来"，CommonUI 负责"能操作、会导航"，LyraUI 负责"符合这出戏的具体玩法"。

---

## 四、代码级区别（同一件事，三层各怎么写）

这是本文**最核心**的一节。用"显示并激活一块 UI"这件事，看三层代码差在哪。

### 4.1 🧱 UMG 层：最原始——自己 Create、自己 AddToViewport

```cpp
// 纯 UMG 的写法（引擎底座）
UMyStartWidget* W = CreateWidget<UMyStartWidget>(GetWorld()->GetFirstPlayerController());
W->AddToViewport();   // 直接塞进视口，完事
```

**问题**：能显示，但——
- 分屏多玩家怎么办？每人一块？
- 手柄焦点默认落哪？方向键怎么移焦点？
- 想叠一层"暂停菜单"盖在 HUD 上，谁在上谁在下？
- 按返回键自动回退上一层？

这些**全得自己写**。UMG 只给你"显示"，不给你"游戏菜单的那套规矩"。

### 4.2 🎛️ CommonUI 层：框架——推进"层栈"，自动管焦点/输入/返回

CommonUI 引入了两个关键概念：
1. **`UPrimaryGameLayout`（主布局）+ 层栈（Layer Stack）**：UI 不再直接 `AddToViewport`，而是**推进某一层**（如 `UI.Layer.Game` / `UI.Layer.Menu` / `UI.Layer.Modal`）。层栈自动处理"谁盖谁、返回键回退、焦点交接"。
2. **`UCommonActivatableWidget`（可激活控件）**：比普通 `UUserWidget` 多了 `OnActivated / OnDeactivated`、`GetDesiredInputConfig()`（声明这块板子激活时，按键还传不传给游戏）等能力。

```cpp
// CommonUI 的写法：不 AddToViewport，而是"推进某一层"
if (UCommonLocalPlayer* LocalPlayer = GetLocalPlayer<UCommonLocalPlayer>())
{
    if (UPrimaryGameLayout* RootLayout = LocalPlayer->GetRootUILayout())
    {
        // 把对话框推进 "UI.Layer.Modal" 这一层，自动处理模态/焦点/返回
        RootLayout->PushWidgetToLayerStack<UCommonGameDialog>(
            TAG_UI_LAYER_MODAL, DialogClass,
            [](UCommonGameDialog& Dialog) { Dialog.SetupDialog(Descriptor, Callback); });
    }
}
```

> **对比 4.1**：UMG 是"塞进视口就完事"；CommonUI 是"推进一个有秩序的层栈"，焦点、返回键、多设备导航由框架接管。**这就是 CommonUI 存在的意义——把'游戏 UI 的脏活累活'标准化。**

### 4.3 🎭 LyraUI 层：项目——数据驱动 + 本剧专属规矩

Lyra 在 CommonUI 之上又加了两样东西：

**(a) 给可激活控件加"输入模式"开关**（`UI/LyraActivatableWidget.h`）：

```cpp
// Lyra 只比 CommonUI 多干一件事：声明"我激活时，按键还传不传给游戏"
UCLASS(Abstract, Blueprintable)
class ULyraActivatableWidget : public UCommonActivatableWidget   // ← 继承 CommonUI
{
    GENERATED_BODY()
public:
    //~UCommonActivatableWidget interface
    virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
    //~End
protected:
    // 本剧规矩：这块板子激活时，是"游戏+菜单"还是"纯菜单"？
    UPROPERTY(EditDefaultsOnly, Category = Input)
    ELyraWidgetInputMode InputConfig = ELyraWidgetInputMode::Default;   // Default/GameAndMenu/Game/Menu

    // 鼠标捕获模式：永久捕获 / 不捕获 等
    UPROPERTY(EditDefaultsOnly, Category = Input)
    EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
};
```

> `GetDesiredInputConfig()` 就是"舞台监督问这块板子：你现在要不要让按键穿透到游戏？"——比如暂停菜单要"独占"输入，HUD 要"和游戏共享"输入。Lyra 用一个枚举 `ELyraWidgetInputMode` 让美术在编辑器里勾选，而不用写 C++。

**(b) 数据驱动地"加 UI"——不写代码，在 Experience 里配**（`GameFeatures/GameFeatureAction_AddWidget.h`）：

```cpp
// 一个"要加进 HUD 的布局"请求：配一个布局类 + 配它放哪一层（用 GameplayTag）
USTRUCT()
struct FLyraHUDLayoutRequest
{
    // 布局控件（注意类型是 UCommonActivatableWidget，不是普通 UUserWidget）
    UPROPERTY(EditAnywhere, Category=UI, meta=(AssetBundles="Client"))
    TSoftClassPtr<UCommonActivatableWidget> LayoutClass;

    // 放哪一层，用 UI.Layer 标签指定（如 UI.Layer.Game）
    UPROPERTY(EditAnywhere, Category=UI, meta=(Categories="UI.Layer"))
    FGameplayTag LayerID;
};

// GameFeatureAction：体验里配一条，就能在开玩时自动把 HUD 布局推进对应层
UCLASS(MinimalAPI, meta = (DisplayName = "Add Widgets"))
class UGameFeatureAction_AddWidgets final : public UGameFeatureAction_WorldActionBase { ... };
```

它真正"加上去"的那一行（`GameFeatureAction_AddWidget.cpp`）：

```cpp
void UGameFeatureAction_AddWidgets::AddWidgets(AActor* Actor, FPerContextData& ActiveData)
{
    ALyraHUD* HUD = CastChecked<ALyraHUD>(Actor);
    if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(HUD->GetOwningPlayerController()->Player))
    {
        for (const FLyraHUDLayoutRequest& Entry : Layout)
        {
            if (TSubclassOf<UCommonActivatableWidget> ConcreteWidgetClass = Entry.LayoutClass.Get())
            {
                // 关键：用 CommonUI 的扩展函数，把布局推进指定层
                ActorData.LayoutsAdded.Add(
                    UCommonUIExtensions::PushContentToLayer_ForPlayer(LocalPlayer, Entry.LayerID, ConcreteWidgetClass));
            }
        }
    }
}
```

> **对比 4.2**：CommonUI 给你 `PushContentToLayer_ForPlayer` 这个"推进层"的工具；Lyra 把它包进 **GameFeatureAction**，让"加什么 UI、放哪层"变成**数据配置**——换个游戏模式（Experience）只要改配置，不用改代码。这就是 Lyra "数据驱动" 的一贯套路（和 Experience/AbilitySet 一个思路）。

---

## 五、LyraUI 里几个"本剧专属"的角色（都站在 CommonUI 肩上）

| LyraUI 类 | 继承自（CommonUI） | 本剧职责 | 代码亮点 |
|-----------|-------------------|---------|---------|
| `ULyraActivatableWidget` | `UCommonActivatableWidget` | 所有可激活 UI 的本剧基类 | 加 `ELyraWidgetInputMode` 输入模式开关 |
| `ULyraHUDLayout` | `ULyraActivatableWidget` | 玩家 HUD 的布局壳 | 绑 `UI.Action.Escape` 弹暂停；监听手柄断连弹提示 |
| `ULyraUIMessaging` | `UCommonMessagingSubsystem` | 统一的确认框/错误框 | 覆写 `ShowConfirmation`/`ShowError`，推进 `UI.Layer.Modal` 层 |
| `ULyraBoundActionButton` | `UCommonButtonBase` | 会随输入设备换皮肤的按钮 | 手柄→GamepadStyle / 触摸→TouchStyle / 键鼠→KeyboardStyle |
| `ULyraFrontendStateComponent` | `UGameStateComponent` | 大厅/主菜单流程调度 | 用 `FControlFlow` 串"按开始→登录→进主界面" |
| `UGameFeatureAction_AddWidgets` | `UGameFeatureAction_WorldActionBase` | 数据驱动加 UI | 配 `LayoutClass`+`LayerID`，运行时 Push 到层 |

### 5.1 例子：按 Esc 弹暂停（`UI/LyraHUDLayout.cpp`）

```cpp
void ULyraHUDLayout::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    // 用 CommonUI 的"UI 动作绑定"：把 UI.Action.Escape 这个动作绑到 HandleEscapeAction
    RegisterUIActionBinding(FBindUIActionArgs(
        FUIActionTag::ConvertChecked(TAG_UI_ACTION_ESCAPE), false,
        FSimpleDelegate::CreateUObject(this, &ThisClass::HandleEscapeAction)));
}

void ULyraHUDLayout::HandleEscapeAction()
{
    // 按 Esc → 用 CommonUI 扩展函数，把暂停菜单推进 UI.Layer.Menu 层
    UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer(
        GetOwningLocalPlayer(), TAG_UI_LAYER_MENU, EscapeMenuClass);
}
```

> 注意：这里**没有** `AddToViewport`，全程是"推进层栈"。返回键回退、焦点交接全由 CommonUI 的层栈自动处理——这就是"站在 CommonUI 肩上"的好处。

### 5.2 例子：按钮按设备换皮肤（`UI/Common/LyraBoundActionButton.cpp`）

```cpp
void ULyraBoundActionButton::HandleInputMethodChanged(ECommonInputType NewInputMethod)
{
    TSubclassOf<UCommonButtonStyle> NewStyle = nullptr;
    if (NewInputMethod == ECommonInputType::Gamepad)      NewStyle = GamepadStyle;   // 手柄：显示"按 A"
    else if (NewInputMethod == ECommonInputType::Touch)   NewStyle = TouchStyle;     // 触摸：显示触摸样式
    else                                                  NewStyle = KeyboardStyle;  // 键鼠：显示"点击"
    if (NewStyle) SetStyle(NewStyle);
}
```

> 它监听 CommonUI 的 `UCommonInputSubsystem::OnInputMethodChangedNative`——玩家从键鼠切到手柄，按钮外观自动跟着变。**这种"多输入设备自适应"正是 CommonUI 的核心价值，UMG 自己做不到。**

---

## 六、一张总图（三层 + 谁调用谁）

```
┌─────────────────────────────────────────────────────────────┐
│  🎭 LyraUI（本剧舞美组 · 项目层）                              │
│   ULyraActivatableWidget / ULyraHUDLayout / ULyraUIMessaging │
│   GameFeatureAction_AddWidgets / ULyraFrontendStateComponent │
│   —— 数据驱动加 UI、Esc暂停、手柄断连、大厅流程、换皮肤        │
│                          │ 继承 & 调用                        │
│                          ▼                                    │
│  🎛️ CommonUI（舞台监督 · 框架层，引擎插件 Runtime/CommonUI）  │
│   UCommonActivatableWidget / UPrimaryGameLayout(层栈)         │
│   UCommonUIExtensions(Push/Pop) / UCommonInputSubsystem       │
│   UCommonMessagingSubsystem(对话框)                           │
│   —— 可激活、层栈管理、焦点导航、多设备、模态弹窗              │
│                          │ 继承                               │
│                          ▼                                    │
│  🧱 UMG（木工坊 · 引擎底座，Runtime/UMG）                      │
│   UUserWidget / UWidget / UWidgetTree / UPanelWidget          │
│   —— 造板子、画控件、播动画、绑数据（只管"显示"）              │
└─────────────────────────────────────────────────────────────┘

调用方向（从下往上是"能力叠加"，从上往下是"实际调用"）：
  LyraUI 调 CommonUI 的工具（PushContentToLayer / RegisterUIActionBinding）
  CommonUI 调 UMG 的控件（UButton / UTextBlock / UImage ...）
```

---

## 七、几个"啊哈"点（把困惑一次说清）

| 你可能困惑的 | 真相 |
|-------------|------|
| "UMG 和 CommonUI 是并列的两个插件吗？" | **不是**。UMG 是底座，CommonUI 是**建在 UMG 之上**的插件。CommonUI 的控件继承自 UMG 的 `UUserWidget`。 |
| "LyraUI 是另一个插件吗？" | **不是**。LyraUI 是 Lyra **项目自己** `Source/LyraGame/UI/` 目录下的代码，继承并使用 CommonUI。 |
| "为什么 Lyra 的 UI 都 `PushContentToLayer` 而不是 `AddToViewport`？" | 因为 CommonUI 的**层栈**能自动管焦点、返回键回退、模态层级、多玩家分屏。`AddToViewport` 是 UMG 的原始做法，游戏菜单里会一堆坑。 |
| "CommonUI 到底解决了什么 UMG 解决不了的？" | **多输入设备导航（手柄/键鼠/触摸）+ 焦点管理 + 菜单层栈 + 模态弹窗 + 返回键回退**。这些是"游戏 UI"的刚需，UMG 只提供"显示"。 |
| "LyraUI 比 CommonUI 多做了什么？" | 只多做"**本剧专属**"的事：输入模式开关、Esc 暂停、手柄断连提示、大厅流程、按 Experience 数据驱动加 UI、按钮换皮肤。 |
| "UI.Layer.Game / Menu / Modal 是什么？" | 是 **GameplayTag**，代表层栈里的不同层。Game=游戏内 HUD，Menu=菜单，Modal=模态弹窗（盖住一切）。用标签分层，而非写死顺序。 |

---

## 八、记住这 4 句话就够了

1. **三层是继承叠加**：`UUserWidget`(UMG) ← `UCommonActivatableWidget`(CommonUI) ← `ULyraActivatableWidget`(LyraUI)。越上越懂游戏，越下越懂显示。
2. **UMG 管"显示"，CommonUI 管"操作与导航"，LyraUI 管"本剧规矩"**。
3. **Lyra 的 UI 用 `PushContentToLayer`（推进层栈）而非 `AddToViewport`**——焦点、返回、模态、分屏全交给 CommonUI。
4. **Lyra 把"加什么 UI、放哪层"做成数据配置**（`GameFeatureAction_AddWidgets` + `UI.Layer` 标签），换游戏模式只改配置，不改代码。

---

## 九、想深入？回到源码

| 想了解 | 看哪个文件 |
|--------|-----------|
| Lyra 可激活 UI 的基类（+输入模式） | `UI/LyraActivatableWidget.h` |
| HUD 布局壳（Esc暂停/手柄断连） | `UI/LyraHUDLayout.h` / `.cpp` |
| 数据驱动加 UI 的 GameFeatureAction | `GameFeatures/GameFeatureAction_AddWidget.h` / `.cpp` |
| 统一对话框（模态弹窗） | `UI/Subsystem/LyraUIMessaging.h` / `.cpp` |
| 按钮按设备换皮肤 | `UI/Common/LyraBoundActionButton.cpp` |
| 大厅/主菜单流程调度 | `UI/Frontend/LyraFrontendStateComponent.h` |
| CommonUI 引擎插件本体 | `Engine/Plugins/Runtime/CommonUI/`（`UPrimaryGameLayout`、`UCommonActivatableWidget`、`UCommonUIExtensions`） |
| UMG 引擎底座 | `Engine/Source/Runtime/UMG/`（`UUserWidget`、`UWidgetTree`） |

> **复习路线**：晕了看本篇第一~三节的角色和故事；懂了回第四节的三段代码对比，那是三层区别的真正落点。
