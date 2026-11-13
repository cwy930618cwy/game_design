# 15 — 教学铁律：教学 md 的文件名，也要一比一对应 Lyra

> **定位**：本文件是 [14_禁止自作聪明搞二选一简化](./14_禁止自作聪明搞二选一简化.md) 的**命名专项强化版**。新对话开始教学前，除 00~14 外，**必须读本文件**。
>
> **一句话**：新建教学 md 时，**文件名里出现的类名/文件名部分，也要和 Lyra 真实一一对应**（只换前缀，一个字不能砍/不能改短）。禁止把 `...WithAbilities` 擅自写成 `...`。

---

## 一、这条铁律怎么来的（真实教训，2026-09-24）

同一天连续因为"擅自简化"挨批：
1. 先是把"实现 `IAbilitySystemInterface`"包装成"二选一"，偷偷选了不接接口（[14](./14_禁止自作聪明搞二选一简化.md)）。
2. 接着把类名 `ALyraCharacterWithAbilities` **擅自砍成** `ATenkaichiCharacter`（丢了 `WithAbilities`）。
3. 于是**教学 md 的文件名也跟着错**——建了个 `02_4_2_角色类TenkaichiCharacter_h.md`（类名部分是错的简化名），后来又得连文件带内容一起改名、删旧文件、修一堆引用。

你当场怒斥：**"你要教 `ALyraCharacterWithAbilities` 就好好命名 `TenkaichiCharacterWithAbilities`，我都不知道你在干嘛。"**

**根因**：我把"简化"从代码蔓延到了**文件命名**——觉得"`WithAbilities` 多余、砍了更简洁"，结果文件名和 Lyra 对不上号，你拿着 md 去 Lyra 里找对应文件都找不到。

---

## 二、核心规则

**教学 md 的文件名，凡出现"类名 / 源文件名"的地方，都要和 Lyra 真实一一对应（只换前缀）。**

- ✅ **类名对应**：Lyra 是 `LyraCharacterWithAbilities`，md 名里就写 `TenkaichiCharacterWithAbilities`，**保留 `WithAbilities`**。
- ✅ **只换前缀**：`Lyra` → `Tenkaichi`，其余部分（含后缀、大小写）**原样保留**。
- ❌ **禁止改短/砍后缀**：不能把 `...WithAbilities` 砍成 `...`，也不能把 `HealthSet` 写成 `Health`。
- ❌ **禁止自创 Lyra 没有的名字**：Lyra 里叫什么，md 名里就用对应的什么。

> 判断标准：**你拿着 md 文件名，能一一对应到 Lyra 的某个类/文件**。对不上号，就是命名错了。

---

## 三、命名映射示例（GAS 阶段）

| Lyra 真实类/文件 | 教学 md 名里应出现的对应名 |
|------------------|--------------------------|
| `ALyraCharacterWithAbilities` | `TenkaichiCharacterWithAbilities` |
| `LyraAbilitySystemComponent` | `TenkaichiAbilitySystemComponent` |
| `LyraHealthSet` | `TenkaichiHealthSet` |
| `LyraAttributeSet` | `TenkaichiAttributeSet` |

**md 文件名示例**（延续课次编号 + 见名知意 + 类名对应）：
- ✅ `02_4_2_角色类TenkaichiCharacterWithAbilities_h.md`
- ❌ `02_4_2_角色类TenkaichiCharacter_h.md`（砍了 `WithAbilities`，对不上号）

---

## 四、和 [09_新建文件夹也要一比一对应Lyra目录](./09_新建文件夹也要一比一对应Lyra目录.md) 的关系

- **09 号**管的是"**文件夹路径**对应 Lyra 目录"（如 `AbilitySystem/Attributes/`）。
- **本文件（15 号）**管的是"**文件名 / 类名**对应 Lyra 的类名"（如 `TenkaichiCharacterWithAbilities`）。
- 两者合起来：**路径对、文件名也对**，整条链路都能和 Lyra 一一对应。

---

## 五、对照表（做什么 / 不做什么）

| 场景 | ✅ 我做 | ❌ 我不做 |
|------|--------|----------|
| 建教学 md | 文件名里的类名对应 Lyra（只换前缀） | 擅自砍后缀 / 改短 |
| Lyra 类名带后缀（`WithAbilities`） | 原样保留 | 觉得"多余"就砍掉 |
| 改类名 | 同步改所有相关 md 文件名 + 引用 | 只改代码、md 名留着旧名 |
| 判断合格 | md 名能对应到 Lyra 类/文件 | 对不上号 |

---

## 六、执行要求（建 md 前自检）

1. **先确认 Lyra 真实类名/文件名**：这个 md 对应的 Lyra 类叫什么？
2. **换前缀生成对应名**：`Lyra` → `Tenkaichi`，其余原样。
3. **不砍不改短**：后缀、大小写全保留。
4. **改名要连引用一起改**：一旦改文件名，同步修所有引用该名的 md（配合全量搜索）。
5. **配合其他铁律**：[01](./01_一比一还原Lyra_只教不写代码.md)（一比一还原，命名也是还原的一部分）、[09](./09_新建文件夹也要一比一对应Lyra目录.md)（路径对应）、[14](./14_禁止自作聪明搞二选一简化.md)（禁止简化/改名）。

---

## 七、一句话结论

**教学 md 的文件名，类名部分要和 Lyra 一比一对应（只换前缀、不砍后缀）；改了类名就要连 md 文件名带所有引用一起改，绝不留着简化后的旧名。**
