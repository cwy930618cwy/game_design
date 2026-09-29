# `LyraPlayerInput.cpp` 速览

> 106 行，**逻辑很聚焦，就是延迟标记一件事**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | ⭐ **跳过 CDO/Archetype**（注释：否则会有一个永远存在的监听者，而且它根本不处理输入）→ `BindToLatencyMarkerSettingChange()` |
| 析构函数 | 退订 |
| `InputKey()` | Super 之后 `ProcessInputEventForLatencyMarker(Params)` |
| `ProcessInputEventForLatencyMarker()` | ⭐ 见下 |
| `BindToLatencyMarkerSettingChange()` | 平台不支持就直接返回；否则订阅设置变化 **并立刻调一次**初始化 |
| `UnbindLatencyMarkerSettingChangeListener()` | `RemoveAll(this)` |
| `HandleLatencyMarkerSettingChanged()` | 更新 `bShouldTriggerLatencyFlash`，并**通知所有延迟标记模块** |

## 打标记的条件

```
① bShouldTriggerLatencyFlash 为真（设置里开了）
② 按下的键是 EKeys::LeftMouseButton
→ 遍历所有 ILatencyMarkerModule，调 SetCustomLatencyMarker(7, GFrameCounter)
   （注释：TRIGGER_FLASH 是 7）
```

> 💡 **两处细节值得学**：
>
> 1. **构造函数里跳过 CDO** —— 这是个通用陷阱。CDO 不会被 tick、不处理输入，在它上面绑委托只会白占一个监听者。源码注释专门解释了这点。
>
> 2. **平台不支持就直接不订阅** —— `DoesPlatformSupportLatencyMarkers()` 为假时直接 return，连委托都不绑。
>
> 另外源码还有个注释：Lyra 只用 "Reflex" 插件做延迟标记，本来可以用 `#if PLATFORM_DESKTOP` 裁掉其它平台，但为了可扩展性故意没做。

**优先级**：`ProcessInputEventForLatencyMarker` → 构造函数
