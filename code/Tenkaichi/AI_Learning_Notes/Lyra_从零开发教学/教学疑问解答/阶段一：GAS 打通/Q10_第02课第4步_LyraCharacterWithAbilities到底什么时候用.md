# Q10 — 第 02 课第 4 步：`LyraCharacterWithAbilities` 到底什么时候用？

> **对应源码**：`Source/LyraGame/Character/LyraCharacterWithAbilities.h/.cpp`、`LyraCharacter.h/.cpp`、`Player/LyraPlayerState.h`
>
> **一句话**：`LyraCharacterWithAbilities` 在 Lyra 里**几乎没被真正用到**——它是 Epic 留的一个"self-contained（自带 ASC）范例类"，主要给 NPC / 非玩家单位用，或当教学样板。真正的玩家主角走的是 `ALyraCharacter` + **PlayerState 上的 ASC** 那条路。

---

## 一、问题是什么（你的原话）

> "看晕了，`LyraCharacterWithAbilities` 一般什么时候用？"

晕的根源：02_4_1 里说"我们走 self-contained 这一支"，但你去看 Lyra 实际项目，主角好像又不是这么挂的——**对不上号，所以晕**。这个疑问很关键，因为搞不清"这个类到底谁在用"，就没法判断"我们照它做对不对"。

---

## 二、源码事实：全项目只有它自己引用它

我在 `LyraStarterGame5.6` 里全量搜了 `LyraCharacterWithAbilities`：

| 搜索范围 | 结果 |
|---------|------|
| `Source/`（C++ 源码） | 只出现在 `LyraCharacterWithAbilities.h` 和 `.cpp` **这两个自己的文件**里 |
| `Plugins/`（GameFeature 插件） | **0 处** |
| 其他 C++ 类 | **没有任何一处** `SpawnActor<ALyraCharacterWithAbilities>` 或引用它 |

> 结论：**Lyra 的 C++ 代码里，没有任何地方真正生成（spawn）这个类的实例。** 它是个"摆在那儿给你看怎么写"的范例类，不是被业务代码调用的类。

---

## 三、那 Lyra 玩家主角到底怎么挂 ASC？（真实分工）

Lyra 里其实有**两种挂 ASC 的方式**，对应两个不同的类：

| 挂法 | 类 | ASC 真正在哪 | 谁用它 |
|------|----|-------------|--------|
| **① 从 PlayerState 取**（Lyra 主角真实走这条） | `ALyraCharacter` | ASC 挂在 **PlayerState** 上，角色只是"取用者" | **真正的玩家主角** |
| **② 自带（self-contained）** | `ALyraCharacterWithAbilities` | ASC 直接建在**角色自己身上** | **NPC / 非玩家单位**，或当范例 |

### 证据 A：主角走①——`ALyraCharacter::GetAbilitySystemComponent()`

```187:195:Source/LyraGame/Character/LyraCharacter.cpp
UAbilitySystemComponent* ALyraCharacter::GetAbilitySystemComponent() const
{
	if (PawnExtComponent == nullptr)
	{
		return nullptr;
	}

	return PawnExtComponent->GetLyraAbilitySystemComponent();
}
```

主角自己不建 ASC，而是通过 `PawnExtComponent` 去**取**一个 ASC。

### 证据 B：那个 ASC 真正躺在 PlayerState 上

```151:153:Source/LyraGame/Player/LyraPlayerState.h
	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "Lyra|PlayerState")
	TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;
```

注释写得很直白："**The ability system component sub-object used by player characters.**"（玩家角色用的 ASC）——它建在 **PlayerState** 上，不在角色上。

### 证据 C：②只是"范例"——文件顶部注释

```15:16:Source/LyraGame/Character/LyraCharacterWithAbilities.h
// ALyraCharacter typically gets the ability system component from the possessing player state
// This represents a character with a self-contained ability system component.
```

翻译："`ALyraCharacter` 通常从它附身的 PlayerState 取 ASC；**这个类（WithAbilities）代表一个自带 ASC 的角色**"。——Epic 自己把②定位成"另一种代表"，不是主角默认路径。

---

## 四、那②（自带 ASC）到底什么时候用？

按 Lyra 的定位 + GAS 通用实践，self-contained 这一支适合：

| 场景 | 为什么用"自带 ASC" |
|------|-------------------|
| **NPC / AI 单位**（如巡逻兵、怪物） | 没有 PlayerState，ASC 只能挂在自己身上 |
| **短期存在、可抛弃的对象**（如训练假人、可破坏的 GAS 物体） | 不需要跨 Pawn 存活，随生随灭 |
| **教学 / 最小可跑骨架**（**就是我们现在的阶段一**） | 不用先建 PlayerState + PawnExtension 那一大套，就能让"ACharacter 上就有 ASC + 属性"跑起来 |

> 类比（**这是我的类比，非源码**）：①像"**档案放在局里**"——人换了（重生），档案还在；②像"**档案自己随身带**"——人没了档案跟着走。NPC 没有"局"（PlayerState）可放档案，只能自己带着。

---

## 五、那我们（阶段一）为什么选②，对不对？

**对，是合理选择**，但要讲清这是"**应用层的选择**"，不是结构简化：

- 我们阶段一的目标是"**先让 ACharacter 上就有 ASC + HealthSet 跑起来**"，PlayerState 那套（`ALyraPlayerState` + `PawnExtensionComponent` + 初始化握手）还没建。
- 走②（self-contained）能**最少代码**达成目标，且**完全照抄 Lyra 第 15-21 行的成员声明 + 创建方式**，只是"宿主"从 PlayerState 换成了角色自己。
- ⚠️ 这**不是**"简化掉 Lyra 的东西"：成员、创建方式、复制设置（`SetIsReplicated` + `Mixed`）、`InitAbilityActorInfo` 调用点，全都泳 Lyra 的 `LyraCharacterWithAbilities`。**唯一变的是宿主**，符合 10 号铁律"要简化先说清代价"（`教学经验新对话必看/10_代码放哪个文件也要对应Lyra禁止乱放.md`）。

> 将来阶段往后走，如果要接玩家主角，就会切到①（ASC 挂 PlayerState），那时会引入 `ALyraPlayerState` 和 `PawnExtensionComponent`。现在先不碰。

---

## 六、一句话结论

**`LyraCharacterWithAbilities` 是 Epic 留的"自带 ASC 范例类"，Lyra 玩家主角并不用它（主角走 `ALyraCharacter` + PlayerState 上的 ASC）；它真正的用武之地是 NPC / 非玩家单位，或像我们阶段一这样搭"最小可跑 GAS 骨架"。我们选它当模板是对的——照抄它的写法，只把宿主从 PlayerState 换成角色自己。**

---

## 七、下一步

回到 **02_4_2（写 `TenkaichiCharacterWithAbilities.h`）**：成员（ASC + HealthSet）、构造函数、`PostInitializeComponents` 声明，并一比一还原 Lyra：实现 `IAbilitySystemInterface` + `GetAbilitySystemComponent()` 写 `override`（类名也一比一对应，叫 `ATenkaichiCharacterWithAbilities`）。
