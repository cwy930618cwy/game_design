# 09 — 教学铁律：新建文件夹/文件的路径，也要一比一对应 Lyra 目录

> **定位**：本文件规定教学里"文件放哪"的**路径约定**。新对话开始教学前，除 00~08 外，**必须读本文件**。
>
> **一句话**：让用户新建文件/文件夹时，**路径要一比一对应 Lyra 真实目录结构**，不能我自己拍脑袋发明文件夹名，否则后期移动文件会很痛苦。

---

## 一、这条铁律怎么来的（真实教训，2026-09-22）

教第 02 课第 2 步（建属性集）时，AI 让用户把文件建在：
```
Source/Tenkaichi/AttributeSet/TenkaichiAttributeSet.h   ❌
```
但 Lyra 真实结构里，属性集是放在：
```
Source/LyraGame/AbilitySystem/Attributes/LyraAttributeSet.h   ✅ 真实
```

你当场质疑：**"文件夹都不对，我怕后面不好移动文件。"**

**根因**：AI 只想着"类名对应上"，忽略了**目录结构也要对应**。自己发明了 `AttributeSet/` 这个 Lyra 里根本不存在的顶层文件夹，导致后续要和 Lyra 对照、迁移时会全部错位。

---

## 二、核心规则

**让用户新建文件前，先到 Lyra 真实源码里确认"这个类放在哪个目录"，然后让 Tenkaichi 的路径一比一映射过去。**

- ✅ **先查 Lyra 目录，再教路径**：讲某一课要建某文件前，先到 `LyraStarterGame5.6` 里看这个类真实躺在哪个文件夹。
- ✅ **路径一一映射**：Lyra 的 `AbilitySystem/Attributes/` → Tenkaichi 的 `AbilitySystem/Attributes/`（保留层级，只换项目前缀/类名）。
- ❌ **禁止自创顶层文件夹**：Lyra 里没有的文件夹名（如 `AttributeSet/`），不要自己发明。
- ❌ **禁止只改类名不改路径**：类名对应上了、但目录对不上，照样算错。

> 映射原则：**Lyra 的目录树 = Tenkaichi 的目录树**，只是把 `LyraGame` 换成 `Tenkaichi`、把 `Lyra` 前缀换成 `Tenkaichi`。

---

## 三、Tenkaichi 与 Lyra 目录映射表（GAS 阶段）

| Lyra 真实路径（`Source/LyraGame/`） | Tenkaichi 对应路径（`Source/Tenkaichi/`） | 放什么 |
|-------------------------------------|------------------------------------------|--------|
| `AbilitySystem/Attributes/LyraAttributeSet.h` | `AbilitySystem/Attributes/TenkaichiAttributeSet.h` | 属性集基类 |
| `AbilitySystem/Attributes/LyraHealthSet.h` | `AbilitySystem/Attributes/TenkaichiHealthSet.h`（将来拆出） | 血量属性集 |
| `AbilitySystem/LyraAbilitySystemComponent.h` | `AbilitySystem/TenkaichiAbilitySystemComponent.h` | 自定义 ASC |
| `AbilitySystem/Abilities/LyraGameplayAbility.h` | `AbilitySystem/Abilities/...` | 技能基类 |
| `AbilitySystem/Effects/...` | `AbilitySystem/Effects/...` | GE |
| `AbilitySystem/Executions/...` | `AbilitySystem/Executions/...` | 伤害执行 |
| `Character/LyraCharacterWithAbilities.h` | `Character/TenkaichiCharacter.h` | 角色 |

> ⚠️ **重点纠错**：属性集的正确位置是 **`AbilitySystem/Attributes/`**，不是 `AttributeSet/`。之前的 md 写错了，已修正。

---

## 四、对照表（做什么 / 不做什么）

| 场景 | ✅ 我做 | ❌ 我不做 |
|------|--------|----------|
| 教建某文件 | 先查 Lyra 真实目录 → 给出对应 Tenkaichi 路径 | 拍脑袋发明文件夹名 |
| 目录层级 | 保留 Lyra 的层级（如 `AbilitySystem/Attributes/`） | 自创 `AttributeSet/` 这种 Lyra 没有的顶层 |
| 类名 vs 路径 | 类名和路径**都**对应 Lyra | 只改类名、路径乱来 |
| 发现路径写错 | 立刻改 md + 提醒用户按正确路径建 | 将错就错 |

---

## 五、执行要求（每次教学自检）

1. **教路径前先查 Lyra**：不确定放哪，就到 `LyraStarterGame5.6` 搜这个类的真实位置。
2. **给映射后的路径**：把 Lyra 路径换成 Tenkaichi 前缀后给用户，保留层级。
3. **发现错误立即修**：一旦意识到路径写错，立刻改对应 md，并明确告诉用户"正确路径是 X，不是 Y"。
4. **配合其他铁律**：仍遵守 [01](./01_一比一还原Lyra_只教不写代码.md)（一比一还原，路径也是还原的一部分）、[08](./08_教学md开头要先讲为什么再给代码.md)（讲清"为什么放这个目录"）。

---

## 六、下一步

第 02 课第 2 步（属性集）的正确路径应为 **`Source/Tenkaichi/AbilitySystem/Attributes/TenkaichiAttributeSet.h/.cpp`**。已修正 `02_2_1` / `02_2_3` / `01_教程设计总纲` 里的错误路径。后续所有"建文件"的教学，都按本文件先查 Lyra 目录、再给对应 Tenkaichi 路径。
