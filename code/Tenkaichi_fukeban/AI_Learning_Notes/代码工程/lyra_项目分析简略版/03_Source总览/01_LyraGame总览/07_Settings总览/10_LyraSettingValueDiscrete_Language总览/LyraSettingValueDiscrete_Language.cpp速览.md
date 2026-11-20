# `LyraSettingValueDiscrete_Language.cpp` 速览

> 171 行。**最麻烦的是"当前该显示哪个语言"的匹配逻辑**。

| 函数 | 干嘛的 |
|---|---|
| `OnInitialized()` | 取本地化管理器里所有语言，用 `IsCultureAllowed` 过滤后存起来，**在开头插入空串代表"系统默认"** |
| `StoreInitial()` | ⚠️ **空函数**（只有个 `// ?` 注释） |
| `OnApply()` | ⭐ 弹一个确认框提醒"**要完全重启游戏**语言改动才生效" |
| `ResetToDefault()` | 选回下标 0（系统默认） |
| `RestoreToInitial()` | `ClearPendingCulture()` |
| `SetDiscreteOptionByIndex()` | 下标 0 → `ResetToDefaultCulture()`；否则 `SetPendingCulture(...)` |
| `GetDiscreteOptionIndex()` | ⭐ 见下 |
| `GetDiscreteOptions()` | 系统默认显示成 "System Default (XXX)"；其它语言如果英文和母语名不同就都显示 |

## 匹配当前语言的三级回退

```
① 如果 ShouldResetToDefaultCulture() → 返回 0
② 优先用 PendingCulture（注释：UI 改的是 pending，得反映出来）
   PendingCulture 为空时：
       若正在用默认语言 → 返回 0
       否则用当前实际语言
③ 先找精确匹配；找不到就用"优先语言链"
   （比如存的是 en-US，列表里只有 en —— 要靠 GetPrioritizedCultureNames 兜住）
```

> 💡 **第 ② 步那个注释值得记住**：设置界面改的是"待应用的语言"，不是当前语言。如果这里读当前语言，玩家选完发现下拉框又跳回去了。

**优先级**：`GetDiscreteOptionIndex` → `OnApply`
