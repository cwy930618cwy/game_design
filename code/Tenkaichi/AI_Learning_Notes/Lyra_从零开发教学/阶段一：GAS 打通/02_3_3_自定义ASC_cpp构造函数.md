# 02-3-3 — 第 02 课 · 第 3 步【3/3】：写自定义 ASC 的 `.cpp`（构造函数）

> **对应 Lyra**：`Source/LyraGame/AbilitySystem/LyraAbilitySystemComponent.cpp` 第 1-15 行（include 头）+ 第 19-27 行（构造函数）
>
> **一句话**：给上一小步声明的构造函数补上**实现**，内容暂时是"空的"（原因下面讲）。**写完这一步，工程应该编译通过——这就是第 3 步的验收点。**

---

## 一、这一小步要做出什么

新建文件：
```
Source/Tenkaichi/AbilitySystem/TenkaichiAbilitySystemComponent.cpp
```

**验收**：编译整个工程 → **0 error**。
（如果只写了上一小步的 `.h` 没写本文件，会报 **unresolved external symbol**（链接期找不到构造函数实现）；本文件补上后就消了。）

---

## 二、先讲"为什么这么写"

**为什么①：为什么要有单独的一个 `.cpp`？不能在 `.h` 里顺手写完吗？**
可以在 `.h` 里内联写，但 **Lyra 的做法是拆开**（声明在 `.h`、实现在 `.cpp`，见 Lyra 第 34 行 vs 第 19-27 行）。拆开的好处：改实现不用让所有 include 它的文件重新编译。我们一比一沿用。
> 更深入的 `.h` / `.cpp` 分工讨论见 [教学疑问解答/Q01](../教学疑问解答/Q01_第02课第2步_基类cpp方法为何是空的_h与cpp关系.md)。

**为什么②：为什么构造函数**函数体现在是空的**，还要写它？**
三个理由，缺一不可：
1. **`.h` 里声明了它**，`.cpp` 就必须给实现，否则链接错（这是 [11 号铁律] 的核对②，也是你上一小步报错的根源）。
2. **它要负责把 `FObjectInitializer` 透传给基类**——`: Super(ObjectInitializer)` 这一句不是废话，**少了它 UObject 的构造流程就断在这里**。
3. **它是未来内容的落点**：Lyra 的构造函数体里干的三件事（下面第四节），等我们第 03 课迁移"输入三件套 + 激活组"时会**照搬回来**。现在先占位。

> 类比（**这是我的类比，非源码**）：继承链像一条**接力跑道**——`Super(ObjectInitializer)` 是把接力棒往上游传；函数体是"本班精通的职责"，现在还没到班的题，先空着跑道。

**为什么③：为什么一定要写 `#include UE_INLINE_GENERATED_CPP_BY_NAME(...)`？**
这是 UE **内联生成 cpp** 的标准写法（引擎定义：`ObjectMacros.h` 第 733-734 行 → 展开成 `"xxx.gen.cpp"`），作用是把 UHT 生成的代码**直接内联进本文件**，少一个编译单元、加快编译。Lyra 第 15 行就是这么写的，你的属性集两个 `.cpp` 也已经是这个写法，保持一致。

---

## 三、你要写的代码

Lyra 原文参照：
```19:27:Source/LyraGame/AbilitySystem/LyraAbilitySystemComponent.cpp
ULyraAbilitySystemComponent::ULyraAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();

	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}
```

**你现在要写的** `Source/Tenkaichi/AbilitySystem/TenkaichiAbilitySystemComponent.cpp`：

```cpp
#include "TenkaichiAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAbilitySystemComponent)

UTenkaichiAbilitySystemComponent::UTenkaichiAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}
```

**逐段回扣"为什么"**：
- 第 1 行 `#include "TenkaichiAbilitySystemComponent.h"` → 本文件是它的实现，必须引自己的头文件（Lyra 第 3 行同款写法）。
- 第 3 行 `UE_INLINE_GENERATED_CPP_BY_NAME` → 引擎标准内联生成写法（为什么③）。
- `: Super(ObjectInitializer)` → 把 UObject 构造流程透传给基类，绝不能省（为什么②-2）。
- 空的函数体 → 现在还没到填内容的阶段，先占位（为什么②-3）。

> **include 风格的两种选择**（都能编过）：
> - ✅ **Lyra 写法（推荐）**：`"TenkaichiAbilitySystemComponent.h"`（短形式，同目录下优先）——和 Lyra 第 3 行一致。
> - 你之前的写法：`"AbilitySystem/TenkaichiAbilitySystemComponent.h"`（相对 `Source/Tenkaichi` 全路径）——也 OK。
>
> 建议本文件用 Lyra 的短形式（**自己配对的头文件用短形式**），其它 `.cpp` 引用它时用全路径。

---

## 四、Lyra 构造函数体里那三件事，我们什么时候回填？

Lyra 构造函数体干了三件事（Lyra `.cpp` 第 22-26 行），它们各自依赖还没迁移的东西：

| Lyra 做的事 | 依赖什么 | 何时回填 |
|-------------|---------|---------|
| `InputPressedSpecHandles.Reset()` | 头文件里的 `TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;`（Lyra 第 98 行） | 第 04 课做 **Tag 驱动输入**时，连成员一起加回来 |
| `InputReleasedSpecHandles.Reset()` | 同上系列（Lyra 第 101 / 104 行） | 同上 |
| `FMemory::Memset(ActivationGroupCounts, 0, sizeof(...))` | `ActivationGroupCounts[(uint8)ELyraAbilityActivationGroup::MAX]`（Lyra 第 107 行），而该枚举在 `LyraGameplayAbility.h` 第 58 行 | 先有 `TenkaichiGameplayAbility`（第 03 课）→ 才能加回来 |

> ⚠️ 关键提醒：`ActivationGroupCounts` 依赖的是 **`ELyraAbilityActivationGroup` 枚举**，它住在 `LyraGameplayAbility.h`——**这就是 `02_3_1` 里说的"必须先有 GameplayAbility 才能补齐 ASC"的具体位置**。到那一步我们会回到**本文件**追加，不会另建文件。

---

## 五、核对清单（已逐条打勾）

- [x] **核对③（[12 号]）**：`UE_INLINE_GENERATED_CPP_BY_NAME` 真实存在（引擎 `CoreUObject/Public/UObject/ObjectMacros.h` 第 733-734 行）；`Super(ObjectInitializer)` 被基类支持（引擎 `GameplayAbilities/Private/AbilitySystemComponent.cpp` 第 49-50 行）。
- [x] **核对①（[11 号]）**：对照 Lyra `.cpp` 第 19-27 行，本版只少了依赖未迁移类的三条语句，已在第四节**逐条标注**，不是漏写。
- [x] **核对②（[11 号]）**：`.h` 声明的构造函数 ↔ `.cpp` 实现，函数名 / 参数（`const FObjectInitializer&`）/ 类名限定一致 → **照抄即可编译链接通过**。

---

## 六、一句话结论

**`.cpp` 就做两件事：把 `ObjectInitializer` 接力棒传给基类（`Super(ObjectInitializer)`），再借内联生成宏把 UHT 代码带进来；函数体留空是"未到填内容的阶段"，等 `TenkaichiGameplayAbility` 到位后会按 Lyra 原样把三条 Reset/Memset 补回来。**

---

## 七、下一步（第 02 课剩余）

第 3 步验收 = **编译通过**。之后：

| 下一步 | 内容 | 对应 Lyra |
|--------|------|-----------|
| 第 4 步 | 建 `Character/TenkaichiCharacter.h/.cpp`，构造里 `CreateDefaultSubobject` 挂 ASC + 属性集 | `Character/LyraCharacterWithAbilities.h/.cpp` |
| 第 5 步 | `PossessedBy`/`OnRep_PlayerState` 里调 `InitAbilityActorInfo`，并给 Health 赋初值验证 | `LyraCharacterWithAbilities.cpp` + `LyraHealthSet.cpp` 初值写法 |

> 第 5 步做完，第 02 课正式验收：**PIE 里 `GetHealth()` 返回 100**。
