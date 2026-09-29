# `ULyraGameData` 干嘛的？—— 全局数据资产

> **定位**：只讲 `System/LyraGameData.h`（`ULyraGameData` 类的头文件）。
>
> **原代码**（`LyraGameData.h`）：
> ```cpp
> UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "Lyra Game Data", ShortTooltip = "Data asset containing global game data."))
> class ULyraGameData : public UPrimaryDataAsset
> {
>     GENERATED_BODY()
> public:
>     UE_API ULyraGameData();
>     // 返回已加载的游戏数据。
>     static UE_API const ULyraGameData& Get();
>
> public:
>     // 伤害 GE（用 SetByCaller 传数值）
>     UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Damage Gameplay Effect (SetByCaller)"))
>     TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;
>
>     // 治疗 GE（用 SetByCaller 传数值）
>     UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Heal Gameplay Effect (SetByCaller)"))
>     TSoftClassPtr<UGameplayEffect> HealGameplayEffect_SetByCaller;
>
>     // 动态增删 Tag 用的 GE
>     UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects")
>     TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect;
> };
> ```
>
> **一句话**：`ULyraGameData` 是一个**全局唯一、只读的数据资产**（DataAsset），头文件里集中放了 3 个核心 GameplayEffect（伤害/治疗/动态Tag）的软引用。任何地方想用这些 GE，都通过 `ULyraGameData::Get()` 这一个入口拿。

---

## 一、它是什么

| 问题 | 答案 |
|------|------|
| 它是什么？ | 一个**数据资产**（DataAsset），纯数据容器 |
| 存什么？ | 3 个 GameplayEffect 的**软引用**（伤害/治疗/动态Tag） |
| 谁用它？ | 全项目任何地方，通过 `ULyraGameData::Get()` |
| 能改吗？ | **不能**（`Const`，只读） |
| 几个实例？ | **全局唯一**（单例，`Get()` 拿到的都是同一个） |

> **核心**：它是 Lyra 的"全局配置抽屉"——把整局都要用的公共数据集中放一处，谁需要谁来取，且**只读不可改**。

---

## 二、从头文件逐行看关键信息

### 2.1 继承 `UPrimaryDataAsset` —— 它是数据资产

```cpp
class ULyraGameData : public UPrimaryDataAsset
```

- `UPrimaryDataAsset` = UE 的**主数据资产**基类（`#include "Engine/DataAsset.h"`）。
- DataAsset = **纯数据容器**，只装 `UPROPERTY` 数据，没有逻辑（不像 Actor/Component）。
- `Primary` = 有唯一 ID，能被 AssetManager 统一管理、加载。

> **JS 对照**：DataAsset ≈ 一个"导出的配置常量对象"：
> ```js
> export const LyraGameData = {
>     DamageGE: "DamageEffect",
>     HealGE:   "HealEffect",
> };
> ```
> 区别是 UE 的 DataAsset 能在编辑器里可视化配置、被资源系统管理。

### 2.2 `UCLASS` 说明符 —— 只读 + 蓝图可用

```cpp
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "Lyra Game Data", ...))
```

| 说明符 | 含义 |
|--------|------|
| `MinimalAPI` | 只导出必要接口（标 `UE_API` 的 `Get()` 等），减小模块耦合 |
| `BlueprintType` | 蓝图里能用这个类型（做变量、参数） |
| `Const` | **只读**——属性运行时不可修改，蓝图里也不能改 |
| `Meta = (DisplayName=...)` | 编辑器里显示的名字："Lyra Game Data" |

> **重点**：`Const` 决定了它是"只读配置"——一旦加载，运行时不能改它的值。

### 2.3 `static const ULyraGameData& Get()` —— 全局唯一入口

```cpp
// Returns the loaded game data.
static UE_API const ULyraGameData& Get();
```

- `static`：不用实例就能调，`ULyraGameData::Get()` 直接用。
- 返回 `const ...&`：返回**只读引用**（不能改）+ 引用（不拷贝，高效）。
- 语义：**全局单例**——全项目拿到的都是同一个对象。

> 这就是"全局配置抽屉"的取用方式：任何地方一句 `ULyraGameData::Get()` 就能拿到那份公共数据。

### 2.4 三个成员变量 —— 都是 GameplayEffect 软引用

```cpp
TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;   // 伤害
TSoftClassPtr<UGameplayEffect> HealGameplayEffect_SetByCaller;     // 治疗
TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect;           // 动态 Tag
```

**共同点**：
- 类型都是 `TSoftClassPtr<UGameplayEffect>`——**类的软引用**（只存路径，不强制加载，用到才加载，省内存）。
- 都是 `UPROPERTY(EditDefaultsOnly, ...)`——只能在**编辑器里**（默认值）配置，运行时不能改。

**三个 GE 的作用**（从注释看）：
| 成员 | 作用 |
|------|------|
| `DamageGameplayEffect_SetByCaller` | 施加**伤害**，数值用 SetByCaller 动态传入 |
| `HealGameplayEffect_SetByCaller` | 施加**治疗**，数值用 SetByCaller 动态传入 |
| `DynamicTagGameplayEffect` | 动态**增删 GameplayTag**（给角色加/减状态标签） |

> **为什么用 SetByCaller**：同一个伤害 GE 可以给不同情况用（子弹伤害 10、爆炸伤害 50），数值在施加时动态传，不用为每种伤害建一个 GE。

### 2.5 `UE_API` 宏 —— 导出控制

```cpp
#define UE_API LYRAGAME_API      // 文件开头
UE_API ULyraGameData();          // 构造函数导出
static UE_API const ULyraGameData& Get();   // Get 导出
// ...
#undef UE_API                    // 文件结尾取消定义
```

- `UE_API` 是 `LYRAGAME_API`（LyraGame 模块的导出宏）。
- 标了 `UE_API` 的函数才能被**其他模块**调用（DLL 导出）。
- 开头 `#define`、结尾 `#undef` 配对，避免污染。

> 只有 `Get()` 和构造函数标了 `UE_API`——因为外部只需要"拿到数据"，不需要改它。

---

## 三、头文件能看出的设计意图

从头文件就能读出 Lyra 的几个设计思路：

1. **集中管理公共配置**：把整局都要用的 GE 放一个 DataAsset，而不是散落各处、硬编码。
2. **只读保证一致性**：`Const` + `const& Get()` 双重只读，防止运行时被意外篡改。
3. **软引用省内存**：`TSoftClassPtr` 不强制加载，配合 AssetManager 按需加载，不用的功能不占内存。
4. **单例好取用**：`static Get()` 让任何模块一句话就能拿到，不用层层传递。

---

## 四、JS 对照（帮你理解）

```js
// JS 里最接近的写法：一个冻结的（只读）全局配置对象
export const LyraGameData = Object.freeze({
    DamageGE: "DamageEffect",   // 软引用 ≈ 存字符串路径
    HealGE:   "HealEffect",
    DynamicTagGE: "DynamicTagEffect",
});

// 用
const damageGE = LyraGameData.DamageGE;   // 相当于 ULyraGameData::Get().DamageGameplayEffect_SetByCaller
```

| 概念 | JS | C++（LyraGameData.h） |
|------|-----|----------------------|
| 数据容器 | 冻结对象 `Object.freeze({...})` | `const` DataAsset |
| 只读 | `Object.freeze` / `const` | `Const` 说明符 + `const&` |
| 引用资源 | 存字符串路径 | `TSoftClassPtr`（软引用） |
| 全局取用 | `import { LyraGameData }` | `ULyraGameData::Get()` |

> **关键差异**：JS 的冻结对象运行时就在内存；C++ 的 DataAsset 是资源文件（.uasset），配合资源系统按需加载，`TSoftClassPtr` 只存路径不占加载开销。

---

## 五、常见疑问速答

| 疑问 | 答案 |
|------|------|
| DataAsset 是什么？ | 纯数据容器（只装 UPROPERTY，没逻辑），可在编辑器配置 |
| `Primary`DataAsset 的 Primary 啥意思？ | 有唯一 ID，能被 AssetManager 统一管理/加载 |
| 为什么是只读的？ | `Const` 说明符 + `Get()` 返回 `const&`，双重保证 |
| `TSoftClassPtr` 和 `TSubclassOf` 区别？ | 前者软引用（只存路径，延迟加载），后者硬引用（强制加载） |
| `SetByCaller` 是什么？ | GE 的一种数值传入方式，施加时动态给数值，一个 GE 复用多种情况 |
| `Get()` 为什么是 static？ | 全局单例，不用实例就能取 |
| `UE_API` 干嘛的？ | 模块导出宏，标了的函数才能被别的模块调用 |
| 三个 GE 分别干嘛？ | 伤害 / 治疗 / 动态增删 Tag |

---

## 六、总结

```
ULyraGameData = 全局唯一、只读的数据资产（DataAsset）

是什么：
  继承 UPrimaryDataAsset，纯数据容器
  头文件里集中放 3 个核心 GameplayEffect 的软引用：
    - DamageGameplayEffect_SetByCaller（伤害）
    - HealGameplayEffect_SetByCaller（治疗）
    - DynamicTagGameplayEffect（动态增删 Tag）

怎么取：
  static const ULyraGameData& Get()  →  全局单例，只读引用
  任何地方一句 ULyraGameData::Get().xxx 就能拿到

设计要点：
  Const 只读 + const& 返回  →  运行时不可改
  TSoftClassPtr 软引用      →  按需加载，省内存
  SetByCaller              →  一个 GE 复用多种数值场景
```

**一句话**：`ULyraGameData` 是 Lyra 的**全局只读配置抽屉**（DataAsset），头文件里集中放了 3 个核心 GameplayEffect（伤害/治疗/动态Tag）的软引用，靠 `static const Get()` 单例入口让全项目随时取用，`Const` + 软引用保证"只读 + 省内存"。

---

## 七、下一步

- 看 `.cpp` 里 `Get()` 具体怎么从 AssetManager 拿到这份数据。
- 看 `LyraAssetManager` 怎么在启动时加载它、路径配在哪。
- 看实际使用点（如 `LyraHealthComponent` 怎么取伤害 GE 施加伤害）。
- 对比 `ULyraPawnData`（另一个 DataAsset，存 Pawn 相关配置）。
