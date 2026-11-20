# 26 — 教学铁律：`.uproject` 模块声明里的字段也要一比一完整还原，别拿"阶段没到"当借口漏字段

> **定位**：本文件是 [25_代码一比一完整写上_逻辑可后讲代码绝不能后补](./25_代码一比一完整写上_逻辑可后讲代码绝不能后补.md) 在「**`.uproject` 模块声明**」这个具体场景上的精确补强。新对话开始教学前，除 00~25 外，**必须读本文件**。
>
> **一句话**：`Tenkaichi.uproject` 里 `Modules` 数组的每个模块声明，**每一个字段都要一比一对应 Lyra 的 `LyraStarterGame.uproject` 完整写上**（只换前缀），不能因为"这个字段是 L3/L5 阶段才用得上"就当场漏掉、说"后面阶段再加"。

---

## 一、这条铁律怎么来的（真实教训，2026-09-29）

教「阶段一第 1 步 · 模块改名」讲到 `.uproject` 时，Lyra 的 `LyraGame` 模块声明里有一个 `AdditionalDependencies` 字段（`DeveloperSettings`、`Engine`）。AI 当时写了这样一句：

> "Lyra 的 `LyraGame` 还多了 `AdditionalDependencies`（`DeveloperSettings`、`Engine`）——那是 L3/L5 阶段才需要的，**本小步暂不加**，后面阶段到了再加。"

用户立刻纠正：

> "你是不是听不懂人话？为什么又不加？？？"

**根因复盘**：这是 25 号铁律「代码后补」的**又一次重犯**，只是换了个对象——之前漏的是 `Target.cs` 的 280 行、`Build.cs` 的依赖，这次漏的是 **`.uproject` 模块声明里的字段**。AI 又把「**逻辑/用途**」和「**字段本身**」混为一谈：`AdditionalDependencies` 里的 `DeveloperSettings`/`Engine` 确实要到 L3/L5 才"用得上"，但这不等于"字段可以现在不写"。

**关键区分**：

| 维度 | 可以靠后吗 |
|------|-----------|
| **字段本身**（`AdditionalDependencies` 这个字段 + `DeveloperSettings`/`Engine` 两个值） | ❌ 必须当场一比一写上 |
| **字段的作用讲解**（为什么 Lyra 要额外依赖 `DeveloperSettings`/`Engine`、在哪个阶段派上用场） | ✅ 可以到对应阶段再深入讲 |

---

## 二、核心规则（一条）

**`.uproject` 的 `Modules` 数组，每个模块对象都要一比一对应 Lyra 完整还原——字段一个不能少、值一个不能改（只换前缀）。**

对照 Lyra 真实源码（`LyraStarterGame.uproject` 第 6~21 行）：

```json
"Modules": [
    {
        "Name": "LyraGame",
        "Type": "Runtime",
        "LoadingPhase": "Default",
        "AdditionalDependencies": [
            "DeveloperSettings",
            "Engine"
        ]
    },
    {
        "Name": "LyraEditor",
        "Type": "Editor",
        "LoadingPhase": "Default"
    }
]
```

对应到 Tenkaichi，必须完整写成：

```json
"Modules": [
    {
        "Name": "TenkaichiGame",
        "Type": "Runtime",
        "LoadingPhase": "Default",
        "AdditionalDependencies": [
            "DeveloperSettings",
            "Engine"
        ]
    },
    {
        "Name": "TenkaichiEditor",
        "Type": "Editor",
        "LoadingPhase": "Default"
    }
]
```

> 注意两个不对等：
> 1. `LyraGame`（Runtime）**有** `AdditionalDependencies`，`LyraEditor`（Editor）**没有**——所以 `TenkaichiGame` 要带上、`TenkaichiEditor` 不带，照 Lyra 原样。
> 2. `AdditionalDependencies` 里有 `DeveloperSettings` 和 `Engine` 两个值，**两个都要写**，不能只写一个。

---

## 三、为什么这条要单独立（和 25 号的边界）

25 号铁律管的是「`.h`/`.cpp`/`Target.cs`/`Build.cs` 里的代码」，但 `.uproject` 是**配置文件**，很容易被当作"可以后面再补"的漏网之鱼。单独立这条，就是为了堵住这个口子：

| 对象 | 是否也受"完整还原"约束 | 举例 |
|------|----------------------|------|
| `.h` / `.cpp` 代码 | ✅ 受（25 号） | `FLyraGameModule` 类、`IMPLEMENT_PRIMARY_GAME_MODULE` 宏 |
| `Build.cs` 依赖 | ✅ 受（25 号） | 22 公开 + 26 私有 + 2 后补 + `PublicDefinitions` |
| `Target.cs` | ✅ 受（25 号） | 10 个文件 + `ApplySharedLyraTargetSettings` 280 行 |
| **`.uproject` 模块声明字段** | ✅ 受（**本文件 26 号**） | `AdditionalDependencies`、`Type`、`LoadingPhase` 等 |

> 一句话：**"配置"和"代码"一样，都算"要一比一还原的东西"，都不能拿"阶段没到"当借口漏掉。**

---

## 四、什么算"漏字段"（反面清单）

| ❌ 违规说法 | ✅ 正确说法 |
|-----------|-----------|
| "`AdditionalDependencies` 是 L3/L5 才需要的，本小步暂不加" | "`AdditionalDependencies` 完整写上（`DeveloperSettings`、`Engine` 两个值），**它为什么需要这两个依赖，到 L3/L5 再深入讲**" |
| "`Type`/`LoadingPhase` 这些默认的就不写了" | "`Type`、`LoadingPhase` 照 Lyra 原样写全，一个字段不落" |
| "Editor 模块现在用不到，先只建 Game 模块" | "`LyraEditor` 也一比一建出来（`TenkaichiEditor`，`Type: Editor`），这是铁律 23 的模块数量还原" |
| "`Engine` 依赖谁都默认有，可以省" | "Lyra 写了 `Engine`，就照写 `Engine`，哪怕看起来是默认的也不省" |

---

## 五、执行要求（每次涉及 `.uproject` 时自检）

1. **字段对齐了吗**？打开 Lyra 的 `LyraStarterGame.uproject`，逐字段对照我给的 `Tenkaichi.uproject`，有没有漏字段？
2. **值对齐了吗**？`AdditionalDependencies` 里是 `DeveloperSettings` + `Engine` 两个值，我是不是只写了一个？
3. **"阶段没到"是不是借口**？我有没有把"这个字段现在用不到"当成"现在可以不写"？——用不到 ≠ 不写。
4. **模块数量对齐了吗**？Lyra 是 2 个模块（`LyraGame` + `LyraEditor`），我是不是只还原了 1 个？

---

## 六、一句话结论

**`.uproject` 里 `Modules` 数组的每个模块声明，字段和值都要一比一对应 Lyra 完整写上（只换前缀），不能拿"这个字段是 L3/L5 阶段才用得上"当借口当场漏掉。字段现在写、用途后面讲——和 25 号铁律一个道理，只是对象换成了配置文件的字段。**