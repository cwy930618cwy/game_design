# 24 — 教学铁律：前缀替换规则（Lyra → Tenkaichi，其余一模一样）

> **定位**：本文件是 [17_一切文件结构命名内容都要一比一对应Lyra](./17_一切文件结构命名内容都要一比一对应Lyra.md) 和 [14_禁止自作聪明搞二选一简化](./14_禁止自作聪明搞二选一简化.md)、[15_教学md文件命名也要一比一对应Lyra](./15_教学md文件命名也要一比一对应Lyra.md) 的**命名映射精确版**。新对话开始教学前，除 00~23 外，**必须读本文件**。
>
> **一句话**：把 Lyra 里出现的 `Lyra` 这个字，换成 `Tenkaichi`，**其余一个字都不动**。`Lyra` 对应 `Tenkaichi`，其他（包括 `Game`、`WithAbilities`、`HealthSet` 等所有后缀/单词）**一模一样、原样保留**。

---

## 一、这条铁律怎么来的（真实教训，2026-09-29）

进入「自下而上底层代码搭建」路线、要建第 1 步（模块地基）时，AI 纠结模块名到底该叫 `Tenkaichi` 还是 `TenkaichiGame`，还一度以为模块名是 `Tenkaichi`（因为用户工程里现有的模块就叫 `Tenkaichi`）。

用户一句话点破：

> "你按照 lyra 来啊，比如他叫 `lyra` 我就叫 `Tenkaichi`，他叫 `LyraGame` 我就叫 `TenkaichiGame`。能听懂吗？`lyra` 对应 `Tenkaichi`，其他一模一样。"

**根因复盘**：
- AI 被「用户工程里现有模块名 = `Tenkaichi`」这个现状干扰，以为要以现状为准。
- 但正确的规则只有一个：**不看现状叫什么，只看 Lyra 叫什么**。Lyra 叫 `LyraGame`，那对应名就是 `TenkaichiGame`，`Game` 必须保留。
- AI 之前还犯过「砍后缀」的错（`WithAbilities`→丢，见 [14](./14_禁止自作聪明搞二选一简化.md)、[15](./15_教学md文件命名也要一比一对应Lyra.md)），本质是同一个病根：**没把「前缀替换」当成唯一的、机械的映射规则，总想"简化/砍掉/看现状"**。

---

## 二、核心规则（唯一、机械、无例外）

**映射规则只有一条：把字符串里的 `Lyra` 换成 `Tenkaichi`，其余字符原样保留。**

- `Lyra` → `Tenkaichi`
- `LyraGame` → `TenkaichiGame`（`Game` 保留！）
- `FLyraGameModule` → `FTenkaichiGameModule`（`F` 前缀、`Game`、`Module` 全保留）
- `ALyraCharacterWithAbilities` → `ATenkaichiCharacterWithAbilities`（`WithAbilities` 保留）
- `ULyraAbilitySystemComponent` → `UTenkaichiAbilitySystemComponent`
- `LyraHealthSet` → `TenkaichiHealthSet`
- `LyraAttributeSet` → `TenkaichiAttributeSet`

> 判断标准：**把 Lyra 名里的 `Lyra` 字符替换成 `Tenkaichi`，得到的就是对应名**。凡是替换后还"多砍了/改短了/加了东西"的，都是错的。

---

## 三、特别提醒：`Game` 这类"看起来像通用词"的字，也要保留

最容易犯的错：觉得 `LyraGame` 里的 `Game` 是"通用词、可省略"，就砍成 `Tenkaichi`。

**错。** `Game` 是 Lyra 模块名的一部分，不是"通用前缀"。Lyra 叫 `LyraGame`，对应就是 `TenkaichiGame`，一个字不能砍。

同理：
- `LyraGame.Build.cs` → `TenkaichiGame.Build.cs`
- `LyraGameModule.cpp` → `TenkaichiGameModule.cpp`
- `IMPLEMENT_PRIMARY_GAME_MODULE(FLyraGameModule, LyraGame, "LyraGame")` → `IMPLEMENT_PRIMARY_GAME_MODULE(FTenkaichiGameModule, TenkaichiGame, "TenkaichiGame")`

---

## 四、和 [14]/[15] 号的关系（本文件是它们的"精确操作版"）

| 铁律 | 管什么 |
|------|--------|
| **14 号** | 禁止砍后缀/改短（`WithAbilities` 必须保留） |
| **15 号** | md 文件名里的类名也要对应 Lyra |
| **24 号（本文件）** | 给出**唯一的、机械的替换规则**：`Lyra`→`Tenkaichi`，其余不动 |

> 一句话：14/15 号说"别砍"，24 号给"到底怎么换"——**就是把 `Lyra` 换成 `Tenkaichi`，其他原样**。

---

## 五、对照表（做什么 / 不做什么）

| 场景 | ✅ 我做 | ❌ 我不做 |
|------|--------|----------|
| Lyra 叫 `LyraGame` | 对应 `TenkaichiGame` | 砍成 `Tenkaichi`（丢了 `Game`） |
| Lyra 叫 `LyraCharacterWithAbilities` | 对应 `TenkaichiCharacterWithAbilities` | 砍成 `TenkaichiCharacter` |
| 用户工程现状叫 `Tenkaichi` | 仍以 Lyra 名为准，改成 `TenkaichiGame` | 迁就现状、不按 Lyra |
| 判断对应名 | 把 `Lyra` 换成 `Tenkaichi`，其余原样 | 替换后还擅自砍/改/加 |

---

## 六、执行要求（每次命名自检）

1. **先看 Lyra 叫什么**：这个模块/类/文件在 Lyra 里的完整名字是什么？
2. **机械替换**：把其中的 `Lyra` 换成 `Tenkaichi`，其余字符一个不动。
3. **不复盘现状**：不要被"用户工程里现在叫什么"干扰，以 Lyra 名为唯一基准。
4. **替换后自检**：替换结果里，除了 `Lyra`→`Tenkaichi` 这一处，还有没有其他被改动的字符？有 = 错。
5. **配合其他铁律**：[14](./14_禁止自作聪明搞二选一简化.md)、[15](./15_教学md文件命名也要一比一对应Lyra.md)、[17](./17_一切文件结构命名内容都要一比一对应Lyra.md)（命名也是一比一还原的一部分）。

---

## 七、一句话结论

**命名映射只有一条机械规则：把 `Lyra` 换成 `Tenkaichi`，其余一模一样、一个字不砍。`LyraGame` → `TenkaichiGame`（`Game` 保留），`LyraCharacterWithAbilities` → `TenkaichiCharacterWithAbilities`（`WithAbilities` 保留）。不要被"用户工程现状叫什么"干扰，以 Lyra 名为唯一基准。**