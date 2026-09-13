# `GameFeatureAction_AddAbilities.h` 速览

> 给**指定类的 Actor** 授予技能 / 属性集 / AbilitySet。菜单里显示为 "Add Abilities"。

## 三个配置用的结构体

| 结构体 | 字段 | 干嘛的 |
|---|---|---|
| `FLyraAbilityGrant` | `AbilityType` | 要授予的技能类 |
| `FLyraAttributeSetGrant` | `AttributeSetType`、`InitializationData` | 属性集类 + 可选的初始化数据表 |
| `FGameFeatureAbilitiesEntry` | `ActorClass`、`GrantedAbilities`、`GrantedAttributes`、`GrantedAbilitySets` | ⭐ **配置一条规则**：给哪类 Actor 加什么 |

## 主要成员

| 成员 | 干嘛的 |
|---|---|
| `: UGameFeatureAction_WorldActionBase`（`final`） | 走世界感知的通用流程 |
| `AbilitiesList` | `TArray<FGameFeatureAbilitiesEntry>`，配置主入口 |
| `OnGameFeatureActivating` / `Deactivating` | 激活 / 卸载 |
| `IsDataValid()`（编辑器） | 校验配置完整性 |
| `AddToWorld()` | 给每个 `ActorClass` 注册扩展处理回调 |
| `HandleActorExtension()` | 收到"扩展添加/移除"或 **`NAME_LyraAbilityReady`** 时加/撤技能 |
| `AddActorAbilities()` / `RemoveActorAbilities()` | 真正的授予与回收 |
| `FindOrAddComponentForActor()` | 找 ASC，找不到就请求 GameFrameworkComponentManager 加一个 |
| `FActorExtensions` | 记录授予了什么（技能句柄 / 属性集 / AbilitySet 句柄），用于精确回收 |
| `FPerContextData` + `ContextData` | ⭐ 按 `FGameFeatureStateChangeContext` 分开记账（多世界/多 PIE 不串） |

**优先级**：`AddActorAbilities` → `HandleActorExtension` → `FindOrAddComponentForActor`
