# `LyraGameInstance.h` 速览

> 声明层：继承 `UCommonGameInstance`，主要干三件事 —— **初始化全局 InitState、登录回调、网络加密握手**。

| 成员 | 干嘛的 |
|---|---|
| `UCLASS(Config = Game)` | 能从 `DefaultGame.ini` 读配置 |
| `: UCommonGameInstance` | 不直接继承 `UGameInstance`，而是套 CommonGame 插件的版本（拿到登录/会话能力） |
| `ULyraGameInstance()` | 构造函数，基本为空 |
| `Init()` / `Shutdown()` | 生命周期入口，干活最多 |
| `GetPrimaryPlayerController()` | 取出转成 `ALyraPlayerController` 的主控制器 |
| `CanJoinRequestedSession()` | 能否加入指定会话（现在是摆设，永远 true） |
| `HandlerUserInitialized()` | 登录完成回调，成功后让 LocalPlayer 读档设置 |
| `ReceivedNetworkEncryptionToken()` | 服务端收到客户端加密令牌，返回密钥 |
| `ReceivedNetworkEncryptionAck()` | 客户端侧配套：带上证书指纹去校验服务端 |
| `OnPreClientTravelToSession()` | 跳转前在 URL 上拼 `?EncryptionToken=` |
| `DebugTestEncryptionKey` | 硬编码的调试用 AES256 密钥（**源码明确写了生产不可用**） |

**优先级**：`Init` → `HandlerUserInitialized` → `ReceivedNetworkEncryptionToken`
