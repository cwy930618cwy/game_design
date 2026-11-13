# 01 — UnrealEd 与 BlueprintGraph 详解（编辑器模块）

> **定位**：对应全景地图第 87-88 行的 `UnrealEd`（编辑器核心）和 `BlueprintGraph`（蓝图图节点系统）——这两个是 **Editor（编辑器）模块**，不是运行时游戏代码。
>
> **一句话**：`UnrealEd` = **UE 编辑器本身**（Content Browser、关卡编辑器、属性面板）；`BlueprintGraph` = **蓝图图编辑器的节点系统**（节点/引脚/连线）。它们是"编辑器"的代码，和"游戏运行"代码是两套体系。
>
> **文件**：`Engine/Source/Editor/`

---

## 一、先分清：Editor（编辑器）vs Runtime（运行时）

这是 UE 源码的**两大世界**：

| | Editor 模块 | Runtime 模块 |
|---|---|---|
| 什么时候运行 | **编辑器里**（开发时） | **游戏运行时**（玩家玩） |
| 干嘛 | 编辑器功能（面板、工具） | 游戏逻辑 |
| 例子 | UnrealEd、BlueprintGraph | Core、Engine、UMG |
| 你平时 | 开发时接触 | 写游戏逻辑 |

**关键**：`UnrealEd`、`BlueprintGraph` 属于 **Editor 模块**——它们是"编辑器这个工具"的代码，**只在编辑器里运行，打包进游戏时不会包含**。

---

## 二、UnrealEd —— 编辑器核心

### 2.1 是什么

**UnrealEd（Unreal Editor）是 UE 编辑器本身**。你打开 UE 看到的**所有编辑器窗口**，都是 UnrealEd 实现的：

```
UE 编辑器（你看到的界面）
├── Content Browser（内容浏览器）
├── Level Editor（关卡编辑器）
├── Property Editor（属性/细节面板）
├── Blueprint Editor（蓝图编辑器）
├── ...（几乎所有编辑器功能）
```

**UnrealEd 是最大的模块**，装了一堆编辑器基础设施（操作、命令、交互）。

### 2.2 目录结构（大概）

```
Engine/Source/Editor/UnrealEd/
├── Classes/    ← 编辑器相关类
├── Public/     ← 编辑器头文件
├── Private/    ← 实现
└── UnrealEd.Build.cs
```

### 2.3 什么时候用 UnrealEd

**只有做"编辑器插件/工具"才用**（比如自定义一个编辑器面板、批量处理资产的工具）。写游戏逻辑**完全不用**。

```cpp
// 例：自定义一个编辑器命令（做工具才用）
#include "UnrealEd/Public/Editor.h"
```

---

## 三、BlueprintGraph —— 蓝图图节点系统

### 3.1 是什么

**BlueprintGraph 是"蓝图图编辑器"的节点系统**——管理蓝图里的**节点、引脚、连线**。

```
一个蓝图（Blueprint）
├── 节点（Node）：事件/函数/变量节点
├── 引脚（Pin）：节点上的输入/输出接口
└── 连线（Wire）：连接引脚
```

**BlueprintGraph 就是实现"你拖蓝图节点、连引脚"这套逻辑的代码。**

### 3.2 和 UnrealEd 的关系

```
UnrealEd（编辑器整体）
  └─ 包含
BlueprintGraph（蓝图编辑器里的"节点/引脚/连线"系统）
```

**UnrealEd 是编辑器的大框架，BlueprintGraph 是其中"蓝图图"那一部分。**

---

## 四、这两个模块对你（新手）的意义

**一句话：你写游戏逻辑基本不用碰它们。**

| 模块 | 你用得到吗 | 什么时候用 |
|------|:---:|------|
| `UnrealEd` | ❌ 一般不用 | 做编辑器插件/工具才用 |
| `BlueprintGraph` | ❌ 一般不用 | 做自定义蓝图节点才用 |

**结论**：
- **新手/写游戏**：不用学这两个，知道"它们让编辑器能工作"即可
- **进阶**：做编辑器工具、自定义蓝图节点时才深入
- **它们属于 Editor 世界**，和运行时游戏代码是两套

---

## 五、理解"编辑器 vs 运行时"的分工（重要）

**UE 的架构**：同样的游戏，编辑器里能"预览 + 调试 + 改资源"，打包后只留运行时。

```
开发时（Editor 世界）：
  你打开 UE → UnrealEd（编辑器）+ BlueprintGraph（蓝图图）
  → 编辑关卡、拖蓝图节点、看预览

打包后（Runtime 世界）：
  玩家运行游戏 → 只有 Runtime 模块（Core/Engine/UMG）
  → UnrealEd/BlueprintGraph 不打包进去
```

**这就是为什么"编辑器模块"和"运行时模块"分开**——编辑器工具不需要跟着游戏发给玩家。

---

## 六、总结速查

```
UnrealEd = UE 编辑器本身（Content Browser/关卡编辑器/属性面板）
BlueprintGraph = 蓝图图编辑器的节点系统（节点/引脚/连线）

它们属于 Editor 模块（编辑器世界）：
  - 只在编辑器运行，不打包进游戏
  - 做编辑器插件/工具才用
  - 写游戏逻辑不用碰

分工：
  开发时 → Editor（UnrealEd/BlueprintGraph）+ Runtime
  运行游戏 → 只有 Runtime
```

**一句话**：`UnrealEd` 是 **UE 编辑器本身**（Content Browser、关卡编辑器等所有编辑器界面），`BlueprintGraph` 是**蓝图图编辑器的节点系统**（节点/引脚/连线）。它们都是 **Editor 模块**——**只在编辑器运行、不打包进游戏**，**做编辑器插件/工具才用，写游戏逻辑不用碰。**

---

## 七、下一步

理解了 Editor 模块是什么，你就可以明确：**做游戏逻辑学 Runtime（已学的 Core/CoreUObject/UI），做编辑器工具才学 Editor（UnrealEd 等）**。这条主线很清晰了。
