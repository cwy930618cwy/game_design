# `LyraInputConfig.h` 速览

> ⭐⭐ **整个输入层的地基**：一个数据资产，把 `UInputAction` 和 `FGameplayTag` 绑成对。

| 成员 | 干嘛的 |
|---|---|
| `FLyraInputAction`（USTRUCT） | 一对：`InputAction` + `InputTag`（`Categories = "InputTag"`） |
| `: UDataAsset`（`BlueprintType, Const`） | 只读数据资产 |
| `FindNativeInputActionForTag()` | ⭐ 按 Tag 找原生动作（找不到可打 Error） |
| `FindAbilityInputActionForTag()` | ⭐ 同上，在技能列表里找 |
| `NativeInputActions` | ⭐ C++ 代码手动绑定的动作 |
| `AbilityInputActions` | ⭐ **自动绑给技能**的动作 |

## 两套列表的区别

| | `NativeInputActions` | `AbilityInputActions` |
|---|---|---|
| 谁来绑 | C++ 手动调 `BindNativeAction` | `BindAbilityActions` 批量绑 |
| 绑给谁 | 普通函数 | 技能的按下/松开 |
| 典型用途 | 移动、视角、菜单 | 开枪、跳跃、技能 |

> 💡 **这是"改键不影响技能"的第一环**：技能只认 `InputTag`，不认具体按键。按键变了只是 `UInputAction` 里的映射变了，Tag 不变，所以技能代码一行都不用改。

**优先级**：两个 Find 函数 → `AbilityInputActions`
