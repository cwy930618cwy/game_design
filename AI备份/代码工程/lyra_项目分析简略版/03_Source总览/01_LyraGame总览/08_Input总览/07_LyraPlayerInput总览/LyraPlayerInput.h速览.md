# `LyraPlayerInput.h` 速览

> 自定义 PlayerInput。**唯一目的是做"输入延迟测量"** —— 按键时在屏幕上闪一下标记，配合 Reflex 测出端到端延迟。

| 成员 | 干嘛的 |
|---|---|
| `: UEnhancedPlayerInput`（`config = Input, transient`） | 标准继承 |
| `InputKey()` | ⭐ 重写：Super 之后再处理延迟标记 |
| `ProcessInputEventForLatencyMarker()` | 真正打标记 |
| `BindToLatencyMarkerSettingChange()` / `UnbindLatencyMarkerSettingChangeListener()` | 订阅/退订设置变化 |
| `HandleLatencyMarkerSettingChanged()` | 设置变了就更新开关状态 |
| `bShouldTriggerLatencyFlash`（protected） | 当前要不要闪 |

> 💡 **为什么要在 PlayerInput 这一层做？** 因为这里是"按键刚进来"的最早时机，能捕获到最接近真实的输入时间点。

**优先级**：`InputKey` → `ProcessInputEventForLatencyMarker`
