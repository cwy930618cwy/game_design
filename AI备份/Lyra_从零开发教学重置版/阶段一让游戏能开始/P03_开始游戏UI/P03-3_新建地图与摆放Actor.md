# P03-3 — 新建自己的地图 + 往关卡里放物品（Actor）

> **定位**：这一步是**编辑器操作**（产物是二进制 `.umap`，拖拽摆放无法用代码/配置表达）——属于 19 号铁律里"确实只能手动"的例外。但"设为默认地图"这步能用配置，仍用配置方式给。
>
> **一句话**：① 新建一个关卡（你已建好 `mainMap`）；② 往关卡里拖 Actor（地面、PlayerStart、按钮所在的 Widget 等）；③ 用配置把这个关卡设为默认地图。

---

## 一、为什么这一步只能手动（先讲清）

- 关卡文件 `.umap` 是**二进制**，Actor 的位置/旋转/缩放是拖拽出来的，没法写进文本文件。
- 所以"摆场景"必须用编辑器。**能用配置表达的（如默认地图、GameMode）我仍给配置**（见第四节），摆模型/放 PlayerStart 才手动。

---

## 二、新建一个关卡（你已完成 ✅）

**操作**：`File → New Level`（或 `Level → New Level`）→ 选一个空模板（Empty Level）→ 创建。

- 你已建好：`Content/Maps/mainMap.umap`。
- **建议**：阶段一我们会用到**两个**关卡——一个"菜单关卡"（放主菜单 Widget）、一个"战斗关卡"（P02 那个有角色站着的场景）。可以建两个，如 `L_MainMenu` 和 `L_Battle`；也可以先只用一个 `mainMap` 练手。

> 命名建议用 `L_` 前缀（Lyra 关卡命名习惯，如 `L_LyraFrontEnd`），方便日后对应。

---

## 三、往关卡里放物品（Actor）

"物品"在 UE 里就是 **Actor**——能放进场景的东西（地面、灯光、PlayerStart、角色出生点等）。

### 操作
1. 打开你的关卡（双击 `mainMap`）。
2. 左侧 **`Place Actors`** 面板（或 `Window → Place Actors`）。
3. 在搜索框输入 Actor 名，找到后**拖进视口**。

### 阶段一你至少要放这几个

| Actor | 在哪搜 | 作用 | 是否必须 |
|-------|--------|------|---------|
| **Floor / 地面** | 新建空关卡通常自带一个地面；没有就搜 `Floor` 或建个 Cube 拉平 | 让角色有地可站 | 是 |
| **PlayerStart** | 搜 `PlayerStart` | 决定角色出生位置（P02-4 讲过，GameMode 照它位置生成角色） | 是 |
| **PlayerStart 之外的装饰** | 搜 `Static Mesh` 里的 Cube/Sphere 等 | 摆场景，看得不那么空 | 否（可选） |

> **关键**：`PlayerStart` 一定要放，且摆在你希望角色出现的位置（比如地面中央上方一点）。不放的话角色会出现在世界原点。

### 摆放小技巧
- 拖进视口后，用 **W/E/R** 键切换 移动/旋转/缩放 工具（或左上角工具栏）。
- 选中 Actor，右侧 `Details` 面板能精确改 `Location` 数值。

---

## 三·五、打光：空关卡乌漆嘛黑怎么办（来自 G03）

新建的**空关卡默认全黑**（UE 空模板不自带任何光源）。要亮起来，`Place Actors` 搜下表拖进视口：

| 搜什么 | 引擎真实类 | 作用 | 必须 |
|--------|-----------|------|------|
| **Sky Atmosphere** | `ASkyAtmosphere` | 天空盒——蔚蓝天空/白云背景 | ✅（要"整个天空亮"必须） |
| **Directional Light** | `ADirectionalLight` | 太阳——主光，决定明暗 | ✅ |
| **Sky Light** | `ASkyLight` | 天空环境光——补光，阴影面不全黑 | ✅ |

### ⚠️ 关键一步（拖了 Sky Light 还黑，就是漏了这个）★
想要"**整个天空发亮的户外大白天**"，Sky Light 必须勾 **`Real Time Capture`（实时捕获）**：
1. 选中 **Sky Light** → 右侧 `Details` → 找 **`Real Time Capture`** → **勾上** ✅
2. 原理：Sky Light 默认需要预先烘焙的 cubemap（空关卡没有→黑）；勾了实时捕获，它会**实时把天空当环境光**均匀照亮场景。此属性引擎真实存在：`USkyLightComponent::bRealTimeCapture`（`SkyLightComponent.h` 第 107-108 行）。

> 只拖 Sky Light 不勾捕获 = 还是黑。**Sky Atmosphere + Sky Light 勾 Real Time Capture + Directional Light** 才是户外大白天。

---

## 三·六、铺地面：旁边太黑怎么铺满（来自 G04）

**旁边黑不是光照问题，是地面太小**——超出地面的地方没有几何体，就是黑色虚空。解决 = 把地面做大铺满视野。

### 做法：用引擎内置 Plane 放大铺地
引擎内置平面：`/Engine/BasicShapes/Plane`。
1. 底部 `Content Browser`，路径栏输 `/Engine/BasicShapes/` 回车
2. 拖 **`Plane`** 进视口
3. 选中 → `Details` → `Transform` → **Scale 的 X、Y 改成 100**（100米×100米，够铺满）
4. 位置 Z 保持 0（贴地）

### 给材质（不然纯灰/纯黑不好看）
- 选中地面 → `Details` → `Material` 槽，填 `WorldGridMaterial`（Content Browser 搜，引擎内置网格地面）
- 更省事：用 `BasicShapes/Cube` 拉扁（Scale Z=0.5）当地板，有厚度不穿模

> 打光、铺地面都是关卡 Actor 操作，产物在二进制 `.umap`，**只能编辑器手动，没法用代码写**（19 号铁律例外）。

---

## 四、把这个关卡设为默认地图（能用配置，用配置）

> 19 号铁律：能写配置就别动编辑器。"默认地图"在 `Config/DefaultEngine.ini` 里配。

你现在的 `Config/DefaultEngine.ini` 里是：
```ini
[/Script/EngineSettings.GameMapsSettings]
GameDefaultMap=/Engine/Maps/Templates/OpenWorld
GlobalDefaultGameMode=/Script/Tenkaichi.TenkaichiGameMode
```

把 `GameDefaultMap` 改成你的**主菜单关卡**（进游戏先看到菜单，你已建好 `L_MainMenu`）：

```ini
GameDefaultMap=/Game/Maps/L_MainMenu.L_MainMenu
```

- 路径规律：`/Game/` + 关卡在 Content 下的相对路径（`Content/Maps/L_MainMenu.umap` → `/Game/Maps/L_MainMenu`）+ `.` + 关卡名。
- `.umap` 后缀在路径里**省略**。

> ⚠️ 关卡路径格式（`/Game/Maps/L_MainMenu.L_MainMenu`）和 C++ 类路径（`/Script/模块.类`）不同，也和蓝图 `_C` 不同——关卡是资产，用 `/Game/路径.资产名`（不带 `_C`）。

> 💡 **默认地图 = 进游戏第一眼看到的关卡** → 设成 `L_MainMenu`（主菜单），不是 `L_Battle`。`L_Battle` 由 P03-2 代码里的 `OpenLevel(this, "L_Battle")` 点按钮后跳转，不用设成默认。`GlobalDefaultGameMode` 那行保持不动。

### 改完长这样
```ini
[/Script/EngineSettings.GameMapsSettings]
GameDefaultMap=/Game/Maps/L_MainMenu.L_MainMenu
GlobalDefaultGameMode=/Script/Tenkaichi.TenkaichiGameMode
```

---

## 五、串起阶段一的完整流程（目标）

理想情况下，阶段一结尾应该是：
1. **默认地图** = 菜单关卡（显示主菜单 Widget + 一个"开始游戏"按钮）。
2. 点按钮 → `OpenLevel`（P03-2 写的）→ 跳到**战斗关卡**（`L_Battle`，有角色站着）。

所以你需要**两个关卡**（你都建好了 ✅）：
- `L_MainMenu`（菜单关卡）：放一个 UI 用的 Widget（P03 后续教怎么把 Widget 显示出来）。
- `L_Battle`（战斗关卡）：放地面 + PlayerStart，角色会自动出现。

> **跳转对应**：`L_MainMenu` 是默认地图（进游戏先到这）；点"开始游戏"按钮 → P03-2 的 `OpenLevel(this, "L_Battle")` → 跳到 `L_Battle`。`OpenLevel` 用**关卡名**（`L_Battle`），不是完整路径，跟 `GameDefaultMap`（用完整路径）格式不同。

---

## 六、一句话结论

**新建地图：`File → New Level`（你已建好 `mainMap`）。放物品：`Place Actors` 搜 Actor 名拖进视口，W/E/R 调整。打光：放 Sky Atmosphere + Directional Light + Sky Light 并勾 `Real Time Capture`（否则还黑）。铺地面：`/Engine/BasicShapes/Plane` 拖出，Scale X/Y 放大到 100+ 铺满。设默认地图：改 `Config/DefaultEngine.ini` 的 `GameDefaultMap=/Game/Maps/L_MainMenu.L_MainMenu`（配置方式，设成主菜单关卡）。关卡路径用 `/Game/路径.资产名`（不带 `_C`）。打光/铺地面是关卡 Actor，只能编辑器手动。**

---

## 七、下一步

回到 P03 主线：把主菜单 Widget **显示出来**——需要在关卡里创建 Widget 并 `AddToViewport`。这一步涉及"怎么把 C++ Widget 实例化并显示"，会在 P03-4 教（对应 Lyra 通过 Experience 的 "Add Widget" 显示 UI，我们做减法用手动创建）。
