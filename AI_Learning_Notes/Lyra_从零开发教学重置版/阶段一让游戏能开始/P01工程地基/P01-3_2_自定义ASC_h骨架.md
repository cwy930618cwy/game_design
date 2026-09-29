# 02-3-2 — 第 02 课 · 第 3 步【2/3】：写自定义 ASC 的 `.h` 骨架

> **对应 Lyra**：`Source/LyraGame/AbilitySystem/LyraAbilitySystemComponent.h` 第 27-34 行
>
> **一句话**：新建 `Source/Tenkaichi/AbilitySystem/TenkaichiAbilitySystemComponent.h`，内容先到"**类声明 + 构造函数声明**"，让 `TenkaichiAttributeSet.cpp` 里那个 `Cast<UTenkaichiAbilitySystemComponent>` 能编译。

---

## 一、这一小步要做出什么

新建文件：
```
Source/Tenkaichi/AbilitySystem/TenkaichiAbilitySystemComponent.h
```
（路径一比一映射 Lyra 的 `Source/LyraGame/AbilitySystem/LyraAbilitySystemComponent.h`，保留层级，只换前缀/类名）

**验收**：只写完 `.h` 还不能算过（缺少 `.cpp` 会报链接错 unresolved external symbol），**必须连下一小步 `02-3-3` 的 `.cpp` 一起写完，工程才编译得过**。本小步先把骨架敲定。

---

## 二、先讲"为什么这么写"（逐行为什么）

**为什么①：为什么要先建 `.h`，而且必须先有它？**
因为 `TenkaichiAttributeSet.cpp` 第 17-20 行要用 `Cast<UTenkaichiAbilitySystemComponent>(...)`。`Cast<>` 需要类型的**完整定义**（要能访问它的静态类信息），光有 `TenkaichiAttributeSet.h` 里的前向声明 `class UTenkaichiAbilitySystemComponent;` 不够 → **必须 include 真正的头文件**。**这是本步存在的最直接理由。**

**为什么②：为什么 `#include "AbilitySystemComponent.h"`？**
因为我们要**继承** `UAbilitySystemComponent`。注意：**继承必须看到基类的完整定义**，前向声明在继承场景下不合法。所以这一行不能省、也不能换成前向声明。（对比：`TenkaichiAttributeSet.h` 里对 ASC 用前向声明是合法的，因为那里只把它当指针用——**用指针可以前向声明，继承/调用成员必须完整定义**。）

**为什么③：为什么 `.generated.h` 必须放最后一个 include？**
UE 反射/UHT 的硬性要求：`.generated.h` 必须是 `#include` 列表的**最后一行**，否则 UHT 解析出错。（你在属性集那两个文件里已经这么做了，保持一致。）

**为什么④：为什么继承来的构造函数要写成 `(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())`？**
Lyra 第 34 行就是这么写的。原因是 UE 对象构造要经过 `FObjectInitializer`（UObject 的专属构造流程），这样写既能在构造参数里写上默认值让 UE 内部直接 new，又能让子类把 `ObjectInitializer` 透传给父类 `Super(ObjectInitializer)`。

> 底层依据：引擎基类用的是 `GENERATED_UCLASS_BODY()`（`AbilitySystemComponent.h` 第 111 行），它会生成"带 FObjectInitializer 参数的构造声明"；实现在 `Private/AbilitySystemComponent.cpp` 第 49-50 行 `UAbilitySystemComponent::UAbilitySystemComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)`。**所以 `Super(ObjectInitializer)` 这种写法是被基类支持的**，Lyra 也确实这么写。

**为什么⑤：`UCLASS(MinimalAPI)` 是什么？**
`MinimalAPI` = "告诉 UHT：这个类**只在本模块内用**，别给它的每个成员函数生成跨模块导出符"。Lyra 的 ASC（第 27 行）和它的属性集基类（`LyraAttributeSet.h` 第 51 行）都写了。

> ⚠️ **顺带纠错**：你上一小步写的 `TenkaichiAttributeSet.h` 用的是 `UCLASS()`，而 Lyra 对应位置是 `UCLASS(MinimalAPI)`。单模块工程里两者**都编得过、无实际差别**；既然要一比一，`ASC 这一步按 Lyra 写 MinimalAPI`，属性集那边要不要补回来你自己定（不补也不影响编译）。

---

## 三、你要写的代码（对照 Lyra 第 27-34 行一比一）

Lyra 原文（带 Lyra 特有的 API 宏）：
```27:34:Source/LyraGame/AbilitySystem/LyraAbilitySystemComponent.h
UCLASS(MinimalAPI)
class ULyraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	UE_API ULyraAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
```

**你现在要写的** `Source/Tenkaichi/AbilitySystem/TenkaichiAbilitySystemComponent.h`：

```cpp
#pragma once

#include "AbilitySystemComponent.h"

#include "TenkaichiAbilitySystemComponent.generated.h"

UCLASS(MinimalAPI)
class UTenkaichiAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	UTenkaichiAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
```

**逐段回扣"为什么"**：
- `#include "AbilitySystemComponent.h"` → 继承需要基类完整定义（为什么②）。
- `#include "TenkaichiAbilitySystemComponent.generated.h"` → UCLASS 必需，且必须在最后（为什么③）。
- `UCLASS(MinimalAPI)` → 对齐 Lyra 第 27 行（为什么⑤）。
- `class UTenkaichiAbilitySystemComponent : public UAbilitySystemComponent` → 这一句就是"给项目的规矩安个家"（见 `02_3_1`）。
- 构造函数带 `FObjectInitializer` → 对齐 Lyra 第 34 行，且基类支持 `Super(ObjectInitializer)`（为什么④）。

---

## 四、明确标注：我教的这版 **vs** Lyra 的差异（分阶段还原，非结构改变）

| 项 | Lyra 原版 | 本步我们写的 | 说明 |
|---|-----------|-------------|------|
| `UE_API` 宏 + `#define/#undef` | 有（第 9 行 `#define UE_API LYRAGAME_API`、第 11 行宏贯穿全文、末尾 `#undef`） | **暂不写** | Lyra 是多模块工程，需要显式导出符；我们单模块 `Tenkaichi` 用不到，强行加反而是噪音 |
| 前向声明列表（`class AActor;` `class UGameplayAbility;` `class ULyraAbilityTagRelationshipMapping;` 等） | 有（第 13-18 行） | **暂不写** | 它们服务于还没迁移过来的那些函数；等第 03 课后回填函数时一起加回来 |
| `UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked)` | 有（第 20 行） | **暂不写** | 服务于"输入被屏蔽"逻辑，属第 04/05 课的输入阶段 |
| 其余成员函数（EndPlay / InitAbilityActorInfo / 输入三件套 / 激活组……） | 有 | **后续回填** | 见 `02_3_1` 第四节，它们依赖 `LyraGameplayAbility` 等尚未迁移的类 |

> **判断标准（遵守 [10 号铁律]）**：文件数量不变（就是这一个类一个文件）、**没有合并、没有拆分类**，只是"**同一个文件的内容按依赖顺序分阶段长出来**"（遵守 [04/07 号铁律] 的一步一 md）。最终形态要和 Lyra 一比一。

---

## 五、核对清单（已逐条打勾）

- [x] **核对③（[12 号]）**：`UAbilitySystemComponent`（引擎第 108-111 行）、`GENERATED_UCLASS_BODY` 生成的构造形式（引擎 cpp 第 49-50 行）、`UCLASS(MinimalAPI)`（Lyra 第 27 行）——均已实际打开源码核对。
- [x] **核对①（[11 号]）**：对照 `LyraAbilitySystemComponent.h`，本版没有缺行/漏类/漏继承。
- [x] **核对②（[11 号]）**：本小步只声明了构造函数，**下一小步 `.cpp` 必须实现它**，否则链接期报 unresolved external symbol。

---

## 六、一句话结论

**`.h` 这一步就是把 Lyra 第 27-34 行照搬过来：一个 `UCLASS(MinimalAPI)` 的 `UTenkaichiAbilitySystemComponent`，继承 `UAbilitySystemComponent`，只声明一个带 `FObjectInitializer` 的构造函数——它存在的直接理由是让 `TenkaichiAttributeSet.cpp` 的 `Cast<>` 拿到完整定义。**

---

## 七、下一步

**02-3-3：写 `Source/Tenkaichi/AbilitySystem/TenkaichiAbilitySystemComponent.cpp`**（构造函数实现，对应 Lyra `.cpp` 第 19-27 行的可分阶段部分）。写完**工程应该编译通过**——这是第 3 步的验收点。
