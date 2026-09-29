# `GameFeatureAction_AddAbilities.cpp` 速览

> 301 行，**其中 60 行是编辑器校验**。核心就一件事：等 ASC 就绪后把技能交出去。

| 函数 | 干嘛的 |
|---|---|
| `OnGameFeatureActivating()` | 取/建本 Context 的数据；不干净就先 `Reset`；再调 Super |
| `IsDataValid()`（编辑器） | 检查 4 类错误：ActorClass 为空、三个授予列表全空、技能类为空、属性集/AbilitySet 为空 |
| `AddToWorld()` | 对每个 `Entry.ActorClass` 用 `ComponentMan->AddExtensionHandler` 注册回调（带 EntryIndex） |
| `HandleActorExtension()` | `ExtensionRemoved`/`ReceiverRemoved` → 回收；`ExtensionAdded` **或 `ALyraPlayerState::NAME_LyraAbilityReady`** → 授予 |
| `AddActorAbilities()` | ⭐ 核心，见下 |
| `RemoveActorAbilities()` | 反向：`RemoveSpawnedAttribute` / `SetRemoveAbilityOnEnd` / `TakeFromAbilitySystem` |
| `FindOrAddComponentForActor()` | 有组件就复用；**但要判断它是不是别人请求创建的** —— 是的话要再发一次请求（请求是引用计数的） |

## `AddActorAbilities()` 做的事

```
1. 没 HasAuthority 直接返回（只在服务器授予）
2. 已经授予过就直接返回（幂等）
3. 找/加 UAbilitySystemComponent
4. 逐个 GiveAbility
5. 逐个建 UAttributeSet（有初始化表就 InitFromMetaDataTable）
6. 逐个 ULyraAbilitySet::GiveToAbilitySystem
7. 记账到 ActiveExtensions，方便后面精确回收
```

> 💡 **`NAME_LyraAbilityReady` 这个事件是关键** —— 它来自 `ALyraPlayerState`，意思是"ASC 已经准备好了"。所以技能不是一看到 Actor 就发，而是**等到 PlayerState 上的 ASC 就绪**才发。这解释了为什么 Lyra 里技能授予时机和角色生成时机是分开的。

> ⚠️ 找不到 ASC 时会 `UE_LOG(Error)` 但**不会崩溃**，技能就静默地没加上 —— 排查"为什么技能没生效"时先看日志。

**优先级**：`AddActorAbilities` → `HandleActorExtension`
