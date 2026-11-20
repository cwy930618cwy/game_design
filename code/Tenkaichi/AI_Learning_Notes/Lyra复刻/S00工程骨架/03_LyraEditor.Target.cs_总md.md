# 03 — `Source\LyraEditor.Target.cs` —— 总 md

> 依据：`E:\ue5\LyraStarterGame\Source\LyraEditor.Target.cs`（**24 行**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraEditor.Target.cs`
> **✅ 本节可独立被 UBT 识别**（但 `LyraEditor` 模块要到 S22 才建，S00 阶段先放着不编译）

---

## 一、这个文件解决什么问题（为什么）

`LyraGame.Target.cs` 编的是**游戏**（打包出去给玩家跑）。但开发时你要在**编辑器**里工作 —— 编辑器是另一个目标，需要单独一份 `Target.cs`。

`LyraEditor.Target.cs` 只做 4 件事：

1. `Type = TargetType.Editor` —— 声明这是编辑器目标
2. `ExtraModuleNames` 加 `LyraGame` + `LyraEditor` —— 编辑器要同时编游戏模块和编辑器模块
3. 禁用「原生指针成员」—— 防止裸指针误用
4. 启用 `RemoteSession` 插件 —— 手机远程调试

🔑 **一句话比喻**：`LyraGame.Target.cs` 是「卖给玩家的成品房」，`LyraEditor.Target.cs` 是「自己住的样板间」—— 样板间多一间工具房（`LyraEditor` 模块）。

---

## 二、内部结构全景

```
LyraEditor.Target.cs
├─ using × 2                                    [3-4]
└─ class LyraEditorTarget : TargetRules         [6-24]
   └─ LyraEditorTarget(TargetInfo)              [8-23]   构造函数
      ├─ Type = TargetType.Editor               [10]
      ├─ ExtraModuleNames.AddRange              [12]     LyraGame + LyraEditor
      ├─ if (!bBuildAllModules)                 [14-17]  指针成员限制
      │  └─ NativePointerMemberBehaviorOverride [16]
      ├─ ApplySharedLyraTargetSettings(this)    [19]     复用 Game 目标的共用设置
      └─ EnablePlugins.Add("RemoteSession")     [22]     手机远程调试
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|---|---|---|---|
| `LyraEditor.Target.cs` | ✅ 依赖 `LyraGameTarget`（第 02 轮） | ⚠️ 需 `LyraGame.Target.cs` 已存在 | ❌ 不需要 |

**关键依赖表**

| 项 | 作用 |
|---|---|
| `LyraGameTarget.ApplySharedLyraTargetSettings` | 第 02 轮定义的静态方法，本文件第 19 行调用 |
| `bBuildAllModules` | UBT 内置属性，`true` 表示「编译全部模块」（`-AllModules` 命令行） |
| `PointerMemberBehavior.Disallow` | 禁止原生指针作为类成员，强制用 `TObjectPtr` / `TWeakObjectPtr` |
| `RemoteSession` | 引擎插件，配合手机 App「Unreal Remote 2」做触屏调试 |

> ⚠️ **S00 阶段这个文件「建了但不生效」**：`.uproject` 里暂时没有 `LyraEditor` 模块，`Source\LyraEditor\` 目录也不存在。UBT 会忽略这个 Target.cs（因为没有对应模块）。**S22 建完 `LyraEditor` 模块后自动生效**。

---

## 四、怎么教（推进节奏）

本轮两件套：

| # | 产物 | 路径 |
|---|---|---|
| ① | 总 md（本文件） | `S00工程骨架\03_LyraEditor.Target.cs_总md.md` |
| ② | 复制文件（代码 + 每行上方中文注释） | `S00工程骨架\03_LyraEditor.Target.cs复制文件.md` |

---

**下一步**：第 4 轮 `Source\LyraGame\LyraGame.Build.cs`（精简版，S06 收敛成 Lyra 完整版）。