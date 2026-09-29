# G02 — Level 就是 Map 吗？

> **问题**：UE 里说的 Level（关卡）、Map（地图）、`.umap` 文件，是不是一回事？
>
> **一句话**：基本是同一个东西。**Map 是磁盘上的文件（`.umap`），Level 是它加载进内存后的运行时对象**——一个 Level 对应一个 `.umap`。日常混用没问题。

---

## 一、区别（严格说法）

| 概念 | 指什么 | 例子 |
|------|--------|------|
| **Map** | **磁盘上的文件**，扩展名 `.umap`，保存关卡的持久化数据 | `Content/Maps/mainMap.umap` |
| **Level** | **运行时/编辑器里的对象**（`ULevel`/`UWorld`），一个 `.umap` 加载进内存后的产物 | 打开 `mainMap` 后，内存里那个可编辑、可 PIE 的关卡 |

- 你 `File → New Level` 建关卡，本质是**新建一个 `.umap` 文件**。
- 双击打开它，引擎把这个 `.umap` 加载成内存里的 Level。
- 所以"一个 Level = 一个 `.umap` 文件"。

---

## 二、为什么大家混着叫

因为一一对应，日常沟通说"地图/关卡/Level/Map"指的通常是同一个东西，不必纠结。硬要精确时：
- 谈**文件/路径/存盘** → 用 **Map**（如配置里的 `GameDefaultMap=/Game/Maps/mainMap.mainMap`）。
- 谈**运行时/加载/切换** → 用 **Level**（如 `OpenLevel` 切关卡、`LoadLevel`）。

---

## 三、进阶（阶段一不用管）

UE5 的 **World Partition** 会把一个大 Level 按网格拆成多个单元流送，此时"一个 Level"在磁盘上可能对应多个数据层/外部 Actor 包。但阶段一你只用默认关卡，就当"一个 Level = 一个 `.umap`"即可。

---

## 四、一句话结论

**Level ≈ Map：Map 是硬盘上的 `.umap` 文件，Level 是它加载进内存后的运行时对象，一一对应。谈文件用 Map，谈运行时用 Level，日常混用无妨。**
