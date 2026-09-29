# P03-2b — 建 Widget 蓝图子类 `WBP_MainMenu` + 放一个 `StartButton`

> **定位**：这是 P03-2（Widget `.cpp`）之后、P03-4（显示 Widget）之前的一个**编辑器手动步骤**。
>
> **为什么单独拎出来**：你在 P03-2 的"下一步"看到"建蓝图子类"，但它实际埋在 P03-4 里，顺序对不上，所以专门建这个 md 手把手教。
>
> **一句话**：基于 C++ 类 `UTenkaichiMainMenuWidget` 建一个蓝图子类 `WBP_MainMenu`，在 Designer 里放一个**命名为 `StartButton`** 的 Button——这样 C++ 里标了 `BindWidget` 的按钮指针才会被引擎自动绑上。

---

## 一、为什么必须建这个蓝图子类（先讲清 BindWidget）

P03-1 里你的 C++ 代码是这么写的：

```cpp
UPROPERTY(meta=(BindWidget))
TObjectPtr<UButton> StartButton;
```

`meta=(BindWidget)` 这条约定的完整含义：

| 谁 | 干什么 |
|----|--------|
| **C++ 基类**（`UTenkaichiMainMenuWidget`） | 只**声明**"有个叫 StartButton 的按钮指针"，但**没有真的按钮控件** |
| **蓝图子类**（`WBP_MainMenu`） | 必须放一个**同名**（`StartButton`）的 Button 控件 |
| **引擎** | 自动把蓝图里那个同名控件，绑到 C++ 的指针上 |

**如果不建蓝图子类会怎样**：直接用 C++ 类实例化，没有任何蓝图控件 → `StartButton` 指针是 **null** → P03-2 里 `if (StartButton)` 直接跳过 → 点击逻辑失效。

> 这就是"BindWidget = C++ 声明、蓝图实现"的分工。C++ 定逻辑，蓝图定长相。

---

## 二、为什么这一步只能编辑器手动（19 号铁律的例外）

- 蓝图子类是 **`.uasset` 二进制资产**，Designer 里拖控件、改名字、摆位置——这些**没法用文本/代码表达**。
- 所以这步属于 19 号铁律里"确实只能手动"的例外（能用配置表达的默认地图/GameMode 仍用配置，见 P03-3）。
- 别担心：这一步是"点几下建个资产"，不是"翻半天菜单"，很快。

---

## 三、手把手：建蓝图子类 `WBP_MainMenu`

### 步骤 1：新建 Widget Blueprint，选对父类
1. 底部打开 **`Content Browser`**（内容浏览器）
2. 在内容区空白处**右键** → `User Interface` → `Widget Blueprint`
3. 弹出"选父类"对话框 → **选 `TenkaichiMainMenuWidget`**（不是默认的 `User Widget`！）
   - 如果列表里看不到，点对话框右下角展开，搜 `TenkaichiMainMenuWidget`
4. 点 `Select`

> ⚠️ **必须选 `TenkaichiMainMenuWidget` 当父类**。如果选了默认 `User Widget`，你的 C++ 逻辑（绑点击、跳关卡）就不在这个蓝图里了。

### 步骤 2：命名 + 存位置
1. 建好后，在 Content Browser 里**右键重命名**为 `WBP_MainMenu`
2. 建议拖到 `Content/UI/` 文件夹（没有就新建一个 `UI` 文件夹）

---

## 四、手把手：在 Designer 里放 `StartButton` 按钮

### 步骤 3：打开 Designer
1. **双击** `WBP_MainMenu` 打开
2. 顶部确认在 **`Designer`** 标签页（不是 Graph）

### 步骤 4：拖一个 Button 进来
1. 左侧 **`Palette`**（控件板）面板，搜 `Button`
2. 把 **`Button`** 拖到中间画布上

### 步骤 5：把按钮改名为 `StartButton`（关键！）
1. **选中**画布上那个 Button
2. 右侧 **`Details`**（细节）面板，最顶部有个**名字输入框**（默认显示 `Button_0` 之类）
3. 把它改成 **`StartButton`**（**必须和 C++ 里 `BindWidget` 的变量名完全一致，区分大小写**）
4. 改完按回车

> ⚠️ **名字必须一字不差 = `StartButton`**。写成 `startButton`、`Btn_Start`、`Button1` 都不行——BindWidget 靠名字匹配，对不上就绑不上（指针 null）。

### 步骤 6（可选）：给按钮加个文字"开始游戏"
1. 左侧 Palette 搜 `Text`，拖一个 **`Text`** 控件到刚才那个 Button **里面**（拖到 Button 上，让它成为 Button 的子控件）
2. 选中 Text，Details 里找到 `Text` 内容，改成"开始游戏"
3. 可调 `Justification` 居中

### 步骤 7：编译保存
1. 顶部点 **`Compile`**（编译）按钮
2. 点 **`Save`**（保存）

---

## 五、验证按钮绑上了（重要）

编译后，如果名字对上了：
- 打开 `WBP_MainMenu` 的 **`Graph`**（图表）页，或看 C++ 的 `NativeConstruct`，`StartButton` 指针在运行时会指向你放的那个按钮。
- 最直接验证：等 P03-4 显示 Widget 后，PIE 里点这个按钮能跳关卡（P03-2 写的 `OpenLevel`），就说明绑上了。

**如果没绑上（指针 null）**，99% 是名字没对上——回去检查步骤 5 的名字是不是 exactly `StartButton`。

---

## 六、常见坑

| 现象 | 原因 | 解决 |
|------|------|------|
| 编译报错 "BindWidget 找不到控件" | 蓝图里没放同名控件，或名字写错 | 确认 Designer 里有个名字 exactly = `StartButton` 的 Button |
| PIE 点按钮没反应 | 按钮指针 null（名字没对上） | 检查名字大小写、拼写 |
| 建蓝图时看不到 `TenkaichiMainMenuWidget` 父类 | C++ 没编译成功 | 先编译 C++ 工程，让 UHT 生成反射 |
| 拖了 Text 但按钮里没文字 | Text 没拖进 Button 内部 | 把 Text 拖到 Button 上，成为它的子控件 |

---

## 七、我们这版 vs Lyra 的差异

| 项 | Lyra | 我们 |
|---|------|------|
| 按钮控件 | CommonUI 的 `ActivatableButton` | 引擎原生 `Button` |
| 绑定方式 | CommonUI 的输入/委托体系 | `BindWidget` + 原生 `OnClicked` |
| 建蓝图子类 | 一样要建蓝图子类放控件 | 一样 |

> Lyra 也是"C++ 基类 + 蓝图子类放控件"这套分工，只是按钮类型和绑定体系不同（CommonUI）。我们做减法用原生 UMG。

---

## 八、一句话结论

**建 `WBP_MainMenu`（父类必须选 `TenkaichiMainMenuWidget`），在 Designer 拖一个 Button 并把它的名字改成 `StartButton`（和 C++ 的 BindWidget 变量名一字不差），编译保存。这步是蓝图二进制资产，只能编辑器手动做（19 号例外）。名字对不上 = 按钮指针 null = 点击失效。**

---

## 九、下一步

**P03-3：新建地图 + 摆放 Actor + 打光 + 铺地 + 设默认地图**（你已建好 `L_MainMenu`/`L_Battle`，这步主要补打光、铺地、设默认地图配置）。之后 **P03-4** 教在 GameMode 里 `CreateWidget` + `AddToViewport` 把这个 `WBP_MainMenu` 显示到屏幕。
