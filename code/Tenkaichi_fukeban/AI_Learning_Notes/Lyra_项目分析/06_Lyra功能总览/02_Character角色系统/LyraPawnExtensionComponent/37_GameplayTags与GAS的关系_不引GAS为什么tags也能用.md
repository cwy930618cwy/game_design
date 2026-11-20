# 37 — GameplayTags 和 GAS 到底啥关系？不引 GAS 为什么 tags 也能用？

> **定位**：你看 Lyra 用了这么多 tags，心里冒出两个疑问：
> 1. **GameplayTags 和 GAS（GameplayAbilities）是什么关系？** 我以为引入 GAS 才能用 tags？
> 2. **Lyra 里大量代码直接用 `LyraGameplayTags::XXX`，如果没引入 GAS 插件，为什么还能编译能用？**
>
> 这篇用**工程依赖证据**（LyraGame.Build.cs / .uproject / 引擎目录）把关系钉死。

---

## 〇、结论先放这（破两个误解）

> **误解 1**："要用 tags 必须先引入 GAS" —— ❌ **反了！**
> **真相**：GameplayTags 是**独立的引擎模块**（在引擎 `Runtime/` 里，不是 GAS 的附属），GAS **依赖它**，而不是它依赖 GAS。
>
> **误解 2**："Lyra 没用 GAS 也能用 tags，所以 tags 是引擎白送的" —— 一半对。
> **真相**：tags 确实独立于 GAS，但**不是自动就能用**——必须在模块的 `.Build.cs` 里**声明依赖 GameplayTags 模块**。Lyra 声明了，所以能编译。

---

## 一、关键：GameplayTags 在引擎里是"模块"，不在 GAS 插件里

看引擎目录结构，两个东西住的地方完全不同：

```
UE_5.6/Engine/Source/Runtime/GameplayTags/          ← GameplayTags 在这
   └─ GameplayTags.Build.cs                          （引擎 Runtime 核心模块）

UE_5.6/Engine/Plugins/Runtime/GameplayAbilities/    ← GAS 在这
   └─ GameplayAbilities.uplugin                       （插件）
```

**GameplayTags 是引擎 Runtime 自带模块**（`FGameplayTag`、`GameplayTagsManager` 都在里面）。它**不是** GAS 的一部分，也不在 GameplayAbilities 插件文件夹里。

### GAS 的 uplugin 反而"引用"了 GameplayTags

看 `GameplayAbilities.uplugin`（真源码）：
```json
"Modules": [
	{ "Name": "GameplayAbilities", ... },
	{ "Name": "GameplayAbilitiesEditor", ... },
	{ "Name": "GameplayTagsEditor", ... }   // ← GAS 自己还引用了 GameplayTagsEditor！
	...
]
```
> GAS 插件的模块列表里**出现了 GameplayTagsEditor**——这本身就说明 **GAS 是站在 GameplayTags 肩膀上的**（依赖关系：GAS → GameplayTags），不是反过来。

---

## 二、Lyra 的依赖证据：`.Build.cs` 里两个都写了（为什么）

Lyra 模块 `LyraGame.Build.cs`（L22~47）的 `PublicDependencyModuleNames` 里，**两个都列了**：

```csharp
"GameplayTags",        // ← ① GameplayTags 模块
"GameplayTasks",       // ← ②
"GameplayAbilities",   // ← ③ GAS
"ModularGameplay",
...
```

**为什么两个都要写？** 因为它们服务不同的东西：
- `GameplayAbilities`（GAS）→ 能力系统（ASC/Ability/Effect，第 10~16 篇那些）
- `GameplayTags` → **只**提供 tag 类型和查询（LyraGameplayTags 清单能用的前提）

**Lyra 既用能力系统、又用大量 tag（输入/状态/InitState…），所以两个模块都依赖。**

---

## 三、那"不引 GAS，为什么 tags 也能用"？（核心疑问）

你的疑问本质是："Lyra 用了 GAS 所以能用 tags 我理解；但如果我没引 GAS，tags 还能用吗？"

### 答案分两层

**第 1 层：tags 能用的前提 = 声明依赖 GameplayTags 模块，而不是依赖 GAS。**

```csharp
// 你自己的模块 .Build.cs
PublicDependencyModuleNames.AddRange(new string[] {
	"GameplayTags",        // ← 想用 FGameplayTag？声明这个就够了，不需要 GAS！
	// "GameplayAbilities" // ← GAS 是可选的，tags 不需要它
});
```

所以：**想用 tags 只引 GameplayTags 模块即可，跟 GAS 无关。** Lyra 是因为"既用 GAS 又用 tags"，才同时引了两个。

**第 2 层：GameplayTags 不是"插件开关"，是引擎自带的编译单元。**

GameplayTags 住在引擎 `Runtime/`（不是需要你手动 Enable 的插件），引擎默认编译它。你的模块只要在 Build.cs 里**声明依赖**，就能 include `GameplayTagsManager.h`、用 `FGameplayTag`。

### 依赖关系图（一图看清谁靠谁）

```
                    引擎核心（Core等）
                          │
           ┌──────────────┴──────────────┐
           ▼                             ▼
  GameplayTags 模块                  （其他模块...）
   - FGameplayTag / Manager
   - 独立的！不依赖 GAS
           │  ▲
           │  │ GAS 依赖它（GAS 站在 tags 上）
           ▼  │
  GameplayAbilities（GAS）插件
   - ASC / Ability / Effect
           │
           ▼
      LyraGame 模块（LyraGame.Build.cs）
       ├─ PublicDependencyModuleNames 加了 "GameplayTags"   ← 用 tag
       ├─ PublicDependencyModuleNames 加了 "GameplayAbilities" ← 用 GAS
       └─ （两个都加了，因为两样都用）
```

### 那张"名不副实"的图（纠正直觉）

你可能脑子里的关系是：
```
❌ 错误印象：GAS 包含 Tags（tags 是 GAS 的一部分，引 GAS 才送 tags）
```

正确的是：
```
✅ 真相：GAS 和 Tags 是平级模块，Tags 更底层、GAS 依赖 Tags
   Tags ◄──── GAS ◄──── Lyra（两样都用，两个都引）
```

---

## 四、那 GameplayAbilities 的 uplugin 为什么必须 Enable？

你可能会问："那为什么 .uproject 里还要 `"Name": "GameplayAbilities", "Enabled": true`？GameplayTags 不用 Enable？"

| | GameplayTags | GameplayAbilities（GAS） |
|---|---|---|
| 住在哪 | 引擎 `Runtime/`（核心模块） | 引擎 `Plugins/`（插件） |
| 要 Enable 吗 | 不用（引擎默认在） | **要**（插件要开） |
| 要在 Build.cs 声明依赖吗 | 要 | 要 |

> - **GameplayTags**：引擎"亲儿子"模块，默认就在，**只需 Build.cs 声明依赖**。
> - **GAS**：是"插件"，需要在 `.uproject` 里 `"Enabled": true` 打开 + Build.cs 声明依赖。

---

## 五、回答你的具体问题（合起来）

> **Q1：GameplayTags 和 GAS 什么关系？**
> A：**平级但 tags 更底层**——GameplayTags 是引擎 Runtime 独立模块（提供 `FGameplayTag`），GAS（GameplayAbilities）是插件且**依赖** tags。**不是 tags 依赖 GAS，是 GAS 依赖 tags。**
>
> **Q2：不引 GAS，为什么 tags 也能用？**
> A：因为 tags 本来就**不需要 GAS**——它的前提只是在**你的模块 Build.cs 里声明依赖 `GameplayTags` 模块**。Lyra 引 GAS 是为了能力系统（ASC 那些），引 tags 是为了那 40 个标签清单；**两件事互相独立，各引各的**。如果只想要 tags，只加 `"GameplayTags"` 一行就够，根本不需要开 GAS 插件。

---

## 六、总结一句话

> **GameplayTags 是引擎 Runtime 自带的独立模块**（提供 `FGameplayTag` 类型和查询），住在引擎 `Runtime/GameplayTags/`，**不需要 enable 插件**，只要模块 Build.cs 声明依赖即可用。**GAS（GameplayAbilities）是引擎插件，它依赖 GameplayTags 而不是反过来**——所以"要用 tags 必须先引 GAS"是误解。Lyra 之所以两个都引（Build.cs 里 `"GameplayTags"` + `"GameplayAbilities"` 并列），是因为它**既用能力系统又用大量 tag**，各取所需。

---

## 七、下一步

- 在你的空项目里试：Build.cs 只加 `"GameplayTags"`（不加 GameplayAbilities），写个 `FGameplayTag` 试试——验证"tags 不需要 GAS"。
- 打开 GAS 的 `.Build.cs`（GameplayAbilities），看它的依赖列表里是否也有 GameplayTags，验证"GAS 依赖 tags"。
- 理解模块依赖如何决定"include 什么、链接什么"（UE 的模块化编译基础）。
