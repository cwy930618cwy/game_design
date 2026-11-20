# 02-1 — 第 02 课 · 第 1 步：给工程加 GAS 依赖

> **一句话**：让 `Tenkaichi` 工程"认识"GAS——`Build.cs` 加三个模块 + `.uproject` 启用插件，两处都要改。

---

## 一、这一步要做出什么

让工程能用 GAS。做完后，后面才能 `#include` GAS 头文件、才能编译过。这是第 02 课真正的**第一步**（因为 `Tenkaichi` 是干净空白工程，尚无 GAS 依赖）。

---

## 二、要理解的概念：GAS 由三个模块组成

GAS 不是一个东西，它由三个模块组成，缺一不可：

| 模块 | 作用 | 类比 |
|------|------|------|
| `GameplayAbilities` | GAS 核心：ASC / GA / GE / AttributeSet 全在这 | 发动机本体 |
| `GameplayTags` | Tag 系统，GAS 用它标识一切（技能/状态/事件） | 标签语言 |
| `GameplayTasks` | 异步任务，Ability Task 的底层依赖 | 传动轴 |

**关键坑**：光在 `Build.cs` 加模块依赖**还不够**——`.uproject` 里还得**启用 `GameplayAbilities` 插件**。两处都要改，否则编译照样找不到模块。

---

## 三、Lyra 真实源码证据（照它配）

**① `LyraGame.Build.cs` 第 30-32 行**——三个模块一起加：
```csharp
"GameplayTags",
"GameplayTasks",
"GameplayAbilities",
```

**② `LyraStarterGame.uproject` 第 35-38 行**——启用插件：
```json
{
    "Name": "GameplayAbilities",
    "Enabled": true
}
```

---

## 四、落到 `Tenkaichi` 工程的两处改动

**改动 A：`Source/Tenkaichi/Tenkaichi.Build.cs`** —— 在 `PublicDependencyModuleNames` 补三个模块（保留原有 Core/CoreUObject/Engine/InputCore/EnhancedInput）：
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
    "GameplayAbilities",   // GAS 核心
    "GameplayTags",        // Tag 系统
    "GameplayTasks"        // 异步任务
});
```

**改动 B：`Tenkaichi.uproject`** —— 在 `"Plugins"` 数组加一项（与 ModelingToolsEditorMode 平级）：
```json
{
    "Name": "GameplayAbilities",
    "Enabled": true
}
```

---

## 五、验收

改完两处 → **右键 .uproject → Generate Visual Studio project files** → 重新编译。能编译通过 = 成功。

---

## 六、常见坑

- 只改 Build.cs 不改 uproject → 编译报 `Cannot open include file: 'AbilitySystemComponent.h'`。两个都要改。
- 改完 uproject 必须重新生成工程文件，否则模块变更不生效。

---

## 七、一句话结论

**GAS 依赖 = Build.cs 加三模块 + uproject 启用 GameplayAbilities 插件，两处缺一不可。**
