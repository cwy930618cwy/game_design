# 04 — UMG 控件大全详解

> **定位**：UMG 的所有**控件（Widget）**——怎么分类、每个干嘛、什么时候用。
>
> **一句话**：UMG 控件分 **布局类、基础显示类、输入类、数据类、交互类、特殊类** 几大类。做 UI 就是"选控件 + 摆布局 + 绑数据"。
>
> **文件**：`Engine/Source/Runtime/UMG/Public/Components/`（81 个控件文件）

---

## 一、控件分类总览（先看全貌）

```
UMG 控件（Widget）
├── 布局类：摆位置（CanvasPanel/VerticalBox/HorizontalBox/Overlay/GridPanel/SizeBox）
├── 基础显示类：显示东西（TextBlock/Image/Border/ProgressBar/Spacer）
├── 输入交互类：玩家操作（Button/CheckBox/Slider/ComboBox/EditableText）
├── 数据类：显示数据（ListView/ListView/TreeView/TableView/EntryBox）
├── 动画/媒体类：动效（WidgetAnimation/Image 序列）
└── 特殊类：通用（UserWidget/RetainerBox/BackgroundBlur）
```

**做 UI 的核心**：**布局类（摆）+ 显示类（看）+ 输入类（点）+ 数据类（列表）**。

---

## 二、布局类（控件的"容器/位置"）

> 决定控件怎么摆。**做 UI 第一步：选布局。**

| 控件 | 干嘛 | 场景 |
|------|------|------|
| `UCanvasPanel` | 自由摆放（绝对坐标） | 血条放屏幕角落 |
| `UVerticalBox` | 垂直排列 | 菜单项竖排 |
| `UHorizontalBox` | 水平排列 | 按钮横排 |
| `UOverlay` | 层叠（重叠） | 文字盖在图上 |
| `UGridPanel` | 网格排列 | 背包格子 |
| `USizeBox` | 固定大小 | 按钮固定宽高 |
| `UScaleBox` | 按比例缩放 | 适应不同分辨率 |

**具体场景：主菜单竖排按钮**

```
UVerticalBox（垂直）
  ├─ Button（开始游戏）
  ├─ Button（选项）
  └─ Button（退出）
```

```cpp
// C++ 里用布局控件
UVerticalBox* MenuBox = NewObject<UVerticalBox>(this);
UButton* StartBtn = NewObject<UButton>(this);
MenuBox->AddChildToVerticalBox(StartBtn);   // 加进竖排
```

---

## 三、基础显示类（显示内容）

> 显示文字、图片、进度等。**做 UI 第二步：显示内容。**

| 控件 | 干嘛 | 场景 |
|------|------|------|
| `UTextBlock` | 文本 | 血量数字 |
| `UImage` | 图片 | 头像/图标 |
| `UBorder` | 边框/背景 | 面板背景 |
| `UProgressBar` | 进度条 | 血条/加载 |
| `USpacer` | 占位空白 | 拉开间距 |

**具体场景：做血条 + 血量数字**

```cpp
UCLASS()
class UHUDWidget : public UUserWidget {
    GENERATED_BODY()
public:
    UPROPERTY(meta=(BindWidget))
    UProgressBar* HealthBar;      // 血条

    UPROPERTY(meta=(BindWidget))
    UTextBlock* HealthText;       // 血量文字

    void UpdateHealth(float Current, float Max) {
        HealthBar->SetPercent(Current / Max);   // 血条比例
        HealthText->SetText(FText::AsNumber(Current));  // 数字
    }
};
```

---

## 四、输入交互类（玩家操作）

> 让玩家点击/输入。**做 UI 第三步：交互。**

| 控件 | 干嘛 | 场景 |
|------|------|------|
| `UButton` | 按钮 | 开始游戏 |
| `UCheckBox` | 勾选框 | 开启音效 |
| `USlider` | 滑块 | 音量调节 |
| `UComboBoxString` | 下拉框 | 选分辨率 |
| `UEditableTextBox` | 输入框 | 输入名字 |
| `UTextBox` | 多行文本输入 | 聊天 |

**具体场景：音量滑块**

```cpp
// 滑块拖动更新音量
UPROPERTY(meta=(BindWidget))
USlider* VolumeSlider;

void OnVolumeChanged(float NewVolume) {
    // 保存音量（GameInstance 或配置）
    MyGameInstance->MasterVolume = NewVolume;
}
```

---

## 五、数据/列表类（显示集合）

> 显示一组数据（列表、网格）。**进阶但常用。**

| 控件 | 干嘛 | 场景 |
|------|------|------|
| `UListView` | 列表 | 好友列表 |
| `UTileView` | 网格列表 | 背包物品 |
| `UTreeView` | 树形 | 技能树 |
| `UEntryBox` | 条目容器 | 数据绑定列表 |

**具体场景：背包物品列表**

```cpp
// 把物品数据放进 ListView 显示
UPROPERTY(meta=(BindWidget))
UListView* ItemList;

void ShowItems(TArray<FItem> Items) {
    ItemList->ClearListItems();
    for (auto& Item : Items) {
        ItemList->AddItem(Item);   // 每件物品加进列表
    }
}
```

---

## 六、特殊/高级类

| 控件 | 干嘛 |
|------|------|
| `UUserWidget` | 所有 UI 的基类（自定义 UI） |
| `URetainerBox` | 缓存绘制（性能优化） |
| `UBackgroundBlur` | 背景模糊（弹窗效果） |
| `UWidgetSwitcher` | 多个 UI 切换（Tab） |

**具体场景：Tab 切换界面**

```cpp
// WidgetSwitcher 管理多个界面切换
UPROPERTY(meta=(BindWidget))
UWidgetSwitcher* TabSwitcher;

void ShowTab(int32 Index) {
    TabSwitcher->SetActiveWidgetIndex(Index);   // 切到第 Index 个界面
}
```

---

## 七、控件分类速查（一张表）

| 分类 | 控件 | 场景 |
|------|------|------|
| **布局** | CanvasPanel/VerticalBox/HorizontalBox/Overlay/GridPanel | 摆位置 |
| **显示** | TextBlock/Image/Border/ProgressBar | 显示内容 |
| **输入** | Button/CheckBox/Slider/ComboBox/EditableTextBox | 玩家操作 |
| **数据** | ListView/TileView/TreeView/EntryBox | 显示列表 |
| **特殊** | UserWidget/RetainerBox/BackgroundBlur/WidgetSwitcher | 高级功能 |

---

## 八、做 UI 的核心套路（记住这个流程）

```
做任何 UI 都是：
1. 选布局（怎么摆）→ CanvasPanel/VerticalBox...
2. 放显示控件（显示啥）→ TextBlock/Image/ProgressBar...
3. 加交互控件（能点吗）→ Button/Slider...
4. 绑数据（值从哪来）→ BindWidget + SetText/SetPercent
5. 加动画（可选）→ WidgetAnimation
```

**具体场景：一个"开始菜单" = 布局 + 按钮 + 文字**

```
UI_MainMenu（UUserWidget）
  ├─ 布局：CanvasPanel（自由摆）
  │    ├─ 显示：Image（背景图）
  │    ├─ 布局：VerticalBox（竖排按钮）
  │    │    ├─ 输入：Button（开始游戏）
  │    │    ├─ 输入：Button（选项）
  │    │    └─ 输入：Button（退出）
  │    └─ 显示：TextBlock（版本号）
```

---

## 九、常见陷阱

**① 忘了选布局控件，控件乱摆**
```cpp
// ❌ 直接放一堆控件，没布局，乱
// ✅ 先用 CanvasPanel/VerticalBox 规划位置
```

**② 混淆 VerticalBox（竖排）和 HorizontalBox（横排）**
```cpp
// VerticalBox = 上下排
// HorizontalBox = 左右排
```

**③ 数据控件忘了绑定数据源**
```cpp
// ❌ ListView 没 AddItem，空的
// ✅ ItemList->AddItem(...)
```

**④ 做 UI 全用 C++（应该用蓝图拖）**
```cpp
// ✅ 大部分 UI 用 Widget Blueprint 拖，C++ 只写逻辑
```

---

## 十、总结速查

```
UMG 控件分类：
  布局：CanvasPanel/VerticalBox/HorizontalBox/Overlay/GridPanel
  显示：TextBlock/Image/Border/ProgressBar
  输入：Button/CheckBox/Slider/ComboBox/EditableTextBox
  数据：ListView/TileView/TreeView
  特殊：UserWidget/RetainerBox/BackgroundBlur/WidgetSwitcher

做 UI 流程：布局 → 显示 → 交互 → 绑数据 → 动画
```

**一句话**：UMG 控件分**布局（摆）、显示（看）、输入（点）、数据（列表）、特殊**几类。做 UI 就是"**选布局 + 放控件 + 绑数据**"。日常最常用 `CanvasPanel`（布局）、`TextBlock`/`Image`/`ProgressBar`（显示）、`Button`/`Slider`（交互）、`ListView`（列表）。

---

## 十一、下一步

理解了控件分类，下一步深入 **UUserWidget 和 C++ 交互**（怎么绑定蓝图控件、怎么传数据、怎么加动画），这是把 UI 和游戏逻辑连起来的关键。
