# `LyraSettingValueDiscreteDynamic_AudioOutputDevice.h` 速览

> 音频输出设备选择。⚠️ **这个功能大部分被注释掉了**。

| 成员 | 干嘛的 |
|---|---|
| `: UGameSettingValueDiscreteDynamic` | ⭐ 继承**动态**版（选项可以运行时增删） |
| `SetDiscreteOptionByIndex()` | ⚠️ **只调 Super，其余全被注释** |
| `OnInitialized()` | 订阅设备插拔事件 + 拉取可用设备列表 |
| `OnAudioOutputDevicesObtained()` | ⭐ 收到列表后填选项 |
| `OnCompletedDeviceSwap()` | ⚠️ **整个函数体被注释**（失败回滚逻辑没启用） |
| `DeviceAddedOrRemoved()` / `DefaultDeviceChanged()` | 设备变化就重新拉列表 |
| `OutputDevices` / `CurrentDeviceId` / `SystemDefaultDeviceId` | 设备信息 |
| `LastKnownGoodIndex` / `bRequestDefault` | 回滚用的（**当前没用上**） |
| 两个回调绑定 | `DevicesObtainedCallback` / `DevicesSwappedCallback` |

**优先级**：`OnAudioOutputDevicesObtained`
