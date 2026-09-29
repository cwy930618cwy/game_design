# `LyraSettingValueDiscreteDynamic_AudioOutputDevice.cpp` 速览

> 137 行，**其中约 60 行是注释掉的代码**。

| 函数 | 干嘛的 |
|---|---|
| `OnInitialized()` | 用 `BindUFunction` 绑两个回调；订阅 `UAudioDeviceNotificationSubsystem` 的插拔/默认设备变化；发起 `GetAvailableAudioOutputDevices` |
| `OnAudioOutputDevicesObtained()` | ⭐ 见下 |
| `OnCompletedDeviceSwap()` | ⚠️ **全被注释** —— 失败时回滚到上一个好设备的逻辑没启用 |
| `DeviceAddedOrRemoved()` / `DefaultDeviceChanged()` | 重新拉一次列表（保证列表实时） |
| `SetDiscreteOptionByIndex()` | ⚠️ **只调 Super** —— 真正切换设备的调用被注释了 |

## 选项怎么填的

```
先 AddDynamicOption("", 空) 占第一位（留给"系统默认"，后面再改文本）
遍历设备：
    bIsSystemDefault → 记下 SystemDefaultDeviceId + 名字
    bIsCurrentDevice → 记下 CurrentDeviceId
    加入选项
最后把第 0 项文本改成 "Default Output - {系统默认设备名}"
SetDefaultValueFromString("")
RefreshEditableState()
```

> ⚠️ **结论：这个设置项目前只能"显示"设备列表，选中后并不会真正切换输出设备** —— 核心的 `SwapAudioOutputDevice` 调用被注释了。想用得自己把那段放开。

**优先级**：`OnAudioOutputDevicesObtained`
