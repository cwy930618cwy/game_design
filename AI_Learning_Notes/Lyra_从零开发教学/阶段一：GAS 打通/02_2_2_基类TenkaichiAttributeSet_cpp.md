# 02-2-2 — 第 02 课 · 第 2 步【2/4】：建**基类** `TenkaichiAttributeSet.cpp`

> **对应 Lyra**：`Source/LyraGame/AbilitySystem/Attributes/LyraAttributeSet.cpp`
>
> **一句话**：写基类的 `.cpp`——构造函数 + 两个工具函数（`GetWorld` / `GetTenkaichiAbilitySystemComponent`，对应 Lyra 的 `GetLyraAbilitySystemComponent`）。

---

## 一、这一小步要做出什么

新建 `Source/Tenkaichi/AbilitySystem/Attributes/TenkaichiAttributeSet.cpp`，实现基类的构造函数和两个工具函数。

---

## 二、先搞懂：为什么基类要有这两个工具函数

基类虽然是"空底座"，但提供了两个**所有子类都能用**的工具函数（Lyra 的 `LyraAttributeSet.cpp` 就是这么写的）：

**为什么①：为什么要写 `GetWorld()`？**
属性集不是 Actor，本身没有"世界"。但子类里经常要拿 World（比如发消息、查游戏状态）。`GetWorld()` 通过属性的 Outer（拥有者）反查出所在的 World，省得每个子类自己写。

> 类比：属性集是"抽屉"，抽屉自己不知道在哪个房间；`GetWorld()` 就是"顺着抽屉所属的柜子，查到它所在的房间"。

**为什么②：为什么要写 `GetTenkaichiAbilitySystemComponent()`（对应 Lyra 的 `GetLyraAbilitySystemComponent()`）？**
属性集挂在 ASC 上。子类经常要反过来拿 ASC（比如改属性、查拥有者）。这个函数把 `GetOwningAbilitySystemComponent()` 的结果转成我们自己的 ASC 类型，方便子类调用。

**为什么③：`.cpp` 顶部为什么要 `#include UE_INLINE_GENERATED_CPP_BY_NAME(...)`？**
UE5 的反射代码生成要求，任何 `UCLASS` 的 `.cpp` 都要引这行，漏了会编译报奇怪错。

---

## 三、你要写的代码（对照 Lyra `LyraAttributeSet.cpp`）

```cpp
#include "AbilitySystem/Attributes/TenkaichiAttributeSet.h"
#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"   // 第3小步会建，先引着
#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAttributeSet)

UTenkaichiAttributeSet::UTenkaichiAttributeSet()
{
}

UWorld* UTenkaichiAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);
	return Outer->GetWorld();
}

UTenkaichiAbilitySystemComponent* UTenkaichiAttributeSet::GetTenkaichiAbilitySystemComponent() const
{
	return Cast<UTenkaichiAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
```

> 注：`TenkaichiAbilitySystemComponent` 是第 3 步才建的类。如果现在编译报"找不到这个类"，可先把 `GetTenkaichiAbilitySystemComponent()` 这个函数**暂时注释掉**，等第 3 步建好 ASC 再放开；或直接用 `GetOwningAbilitySystemComponent()` 返回基类指针。

**逐段回扣"为什么"**：
- `GetWorld()` → 回扣为什么①，顺着 Outer 查 World。
- `GetTenkaichiAbilitySystemComponent()` → 回扣为什么②，把 ASC 转成我们的类型。
- `UE_INLINE_GENERATED_CPP_BY_NAME` → 回扣为什么③，反射必备。

---

## 四、验收

编译通过 = 这一小步过关。

---

## 五、一句话结论

**基类 `.cpp` = 构造函数 + `GetWorld()`（顺 Outer 查世界）+ `GetXxxASC()`（转 ASC 类型）；两个工具函数供所有子类复用。**
