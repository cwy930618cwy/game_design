# `LyraGameInstance.cpp` 速览

> 实现层：340 行里 **一半是网络加密**，其余才是流程骨架。

| 函数 / 块 | 干嘛的 |
|---|---|
| `namespace Lyra` 的 3 个 CVar | 控制台开关：`Lyra.TestEncryption`、`Lyra.UseDTLSEncryption`、`Lyra.TestDTLSFingerprint` |
| `GenerateDTLSCertificate` 命令 | 生成自签名证书导出成 PEM（非 Shipping 才有） |
| `Init()` | ⭐ 注册 4 个 InitState；填充调试密钥；挂 `OnPreClientTravelEvent` |
| `Shutdown()` | 反注册事件，然后 `Super::Shutdown()` |
| `GetPrimaryPlayerController()` | 调父类再 Cast |
| `CanJoinRequestedSession()` | 目前只透传 `Super`，注释写着以后要检查玩家状态 |
| `HandlerUserInitialized()` | 登录成功后 → `LocalPlayer->LoadSharedSettingsFromDisk()` |
| `ReceivedNetworkEncryptionToken()` | 服务端：查/发 DTLS 证书，或直接用硬编码密钥 |
| `ReceivedNetworkEncryptionAck()` | 客户端：拿本地 PlayerId 拼 token，配指纹校验服务端 |
| `OnPreClientTravelToSession()` | 按 CVar 决定是否往 URL 追加 token |

## `Init()` 里注册的四步 InitState

```
InitState_Spawned → InitState_DataAvailable → InitState_DataInitialized → InitState_GameplayReady
```

> 这是整个 Lyra 组件初始化的总绪，注册给 `UGameFrameworkComponentManager`，所有 Pawn 上的组件都按这条链推进。

**优先级**：`Init` → `HandlerUserInitialized` →（`ReceivedNetworkEncryptionToken` 可先跳过）
