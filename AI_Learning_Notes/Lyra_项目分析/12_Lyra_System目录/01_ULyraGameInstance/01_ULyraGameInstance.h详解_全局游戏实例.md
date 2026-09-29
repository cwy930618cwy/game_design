# ULyraGameInstance.h 详解 —— 只读头文件

> **定位**：**只讲 `LyraGameInstance.h` 头文件本身**，不掺 `.cpp` 实现。
>
> **源码位置**：`Source/LyraGame/System/LyraGameInstance.h`
>
> **一句话**：这个头文件声明了一个继承自 `UCommonGameInstance` 的**全局游戏实例**类，重写了登录、会话、网络加密相关的回调，并暴露一个拿主玩家控制器的便捷函数。

---

## 一、头文件全貌

整个 `.h` 就干一件事：**声明一个类，重写 6 个函数 + 1 个成员变量**。结构如下：

```cpp
#pragma once
#include "CommonGameInstance.h"
#include "LyraGameInstance.generated.h"

UCLASS(MinimalAPI, Config = Game)
class ULyraGameInstance : public UCommonGameInstance
{
    GENERATED_BODY()

public:
    ULyraGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    ALyraPlayerController* GetPrimaryPlayerController() const;

    virtual bool CanJoinRequestedSession() const override;
    virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext) override;

    virtual void ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate) override;
    virtual void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate) override;

protected:
    virtual void Init() override;
    virtual void Shutdown() override;
    void OnPreClientTravelToSession(FString& URL);

private:
    TArray<uint8> DebugTestEncryptionKey;
};
```

---

## 二、逐段拆解

### 2.1 类声明

```cpp
UCLASS(MinimalAPI, Config = Game)
class ULyraGameInstance : public UCommonGameInstance
```

| 部分 | 含义 |
|------|------|
| `MinimalAPI` | 只导出必要符号（控制 DLL 导出，减小体积/加快编译） |
| `Config = Game` | 可读写 `DefaultGame.ini` 配置 |
| `public UCommonGameInstance` | 继承 CommonUI 插件的游戏实例基类（比原生 `UGameInstance` 多了会话/多平台支持） |

> **背景**：`UGameInstance` 是**贯穿整个进程、跨关卡（地图）都存在**的单例——切地图时 Actor/World 都换了，但它不销毁。所以"换地图也不该丢的东西"（登录状态、全局设置、加密密钥）都放这里。

---

### 2.2 构造函数

```cpp
UE_API ULyraGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
```
- 标准 UE 构造写法，带对象初始化器（用于 `NewObject` 时的属性初始化）。

---

### 2.3 公有函数（对外接口）

#### ① `GetPrimaryPlayerController`（普通函数，非 override）
```cpp
ALyraPlayerController* GetPrimaryPlayerController() const;
```
- 拿"主玩家"的 PlayerController，返回类型是 **Lyra 专用**的 `ALyraPlayerController`（内部会把父类结果 `Cast` 过来）。
- `const`：不修改 GameInstance 自身。

#### ② `CanJoinRequestedSession`（override）
```cpp
virtual bool CanJoinRequestedSession() const override;
```
- 判断"能否加入一个被请求的会话"（如别人邀请你加入一局）。
- 返回 `bool`。

#### ③ `HandlerUserInitialized`（override）
```cpp
virtual void HandlerUserInitialized(
    const UCommonUserInfo* UserInfo, bool bSuccess, FText Error,
    ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext) override;
```
- **用户登录初始化完成**的回调（CommonUI 用户系统调用）。
- 参数：用户信息、是否成功、错误文本、请求的权限、在线上下文。

#### ④ `ReceivedNetworkEncryptionToken`（override）★
```cpp
virtual void ReceivedNetworkEncryptionToken(
    const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate) override;
```
- 收到客户端发来的**加密令牌**，服务器据此协商加密密钥。
- `EncryptionToken`：客户端传来的令牌字符串。
- `Delegate`：把协商结果（密钥）回调出去。

#### ⑤ `ReceivedNetworkEncryptionAck`（override）★
```cpp
virtual void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate) override;
```
- 加密握手的**确认**流程（客户端侧对应环节）。
- `Delegate`：同样把密钥结果回调出去。

> ④⑤ 是**网络加密密钥协商**的两个钩子。从头文件签名看，它们都接收一个 `FOnEncryptionKeyResponse` 委托用于返回密钥响应。

---

### 2.4 保护成员（生命周期 + 内部逻辑）

```cpp
protected:
    virtual void Init() override;      // 初始化钩子（对象创建后调用）
    virtual void Shutdown() override;  // 销毁钩子（对象销毁前调用）
    void OnPreClientTravelToSession(FString& URL);  // 客户端"旅行到会话"前处理 URL
```

| 函数 | 作用（看签名） |
|------|--------------|
| `Init` | 游戏实例初始化时调用（重写父类钩子） |
| `Shutdown` | 游戏实例销毁前调用（重写父类钩子） |
| `OnPreClientTravelToSession` | 客户端即将连接到会话服务器前，可往 `URL` 里塞参数（如加密令牌） |

> `OnPreClientTravelToSession` 参数是 `FString& URL`（引用），说明它要**修改**这个 URL。

---

### 2.5 私有成员

```cpp
private:
    /** A hard-coded encryption key used to try out the encryption code.
        This is NOT SECURE, do not use this technique in production! */
    TArray<uint8> DebugTestEncryptionKey;
```
- 一个字节数组，存**硬编码的调试加密密钥**。
- 注释明确警告：**这是演示用，不安全，生产环境别用**。
- 配合上面的 `ReceivedNetworkEncryptionToken/Ack` 使用。

---

## 三、从头文件能读出的全部信息（不猜实现）

```
ULyraGameInstance : UCommonGameInstance
│
├─ 身份：全局游戏实例（跨关卡单例），可读写 DefaultGame.ini
│
├─ 对外接口（public）
│   ├─ GetPrimaryPlayerController()      拿主玩家控制器（Lyra 类型）
│   ├─ CanJoinRequestedSession()         能否加入会话
│   ├─ HandlerUserInitialized(...)       用户登录完成回调
│   ├─ ReceivedNetworkEncryptionToken()  加密令牌→密钥协商（服务器侧）
│   └─ ReceivedNetworkEncryptionAck()    加密密钥确认
│
├─ 生命周期/内部（protected）
│   ├─ Init()                            初始化
│   ├─ Shutdown()                        销毁清理
│   └─ OnPreClientTravelToSession(URL)   旅行前改 URL
│
└─ 数据（private）
    └─ DebugTestEncryptionKey            硬编码加密密钥（演示用，不安全）
```

**三类职责**（仅从头文件签名推断）：
1. **会话/登录**：`CanJoinRequestedSession` + `HandlerUserInitialized`。
2. **网络加密**：`ReceivedNetworkEncryptionToken/Ack` + `DebugTestEncryptionKey` + `OnPreClientTravelToSession`。
3. **便捷访问**：`GetPrimaryPlayerController`。

---

## 四、核心要点（只记头文件层面）

```
1. 继承 UCommonGameInstance，是跨关卡存在的全局单例。

2. 头文件只声明、不实现；重写的全是父类虚函数（override）。

3. 5 个 override 分三组：
   - 会话：CanJoinRequestedSession
   - 登录：HandlerUserInitialized
   - 加密：ReceivedNetworkEncryptionToken / ReceivedNetworkEncryptionAck

4. 2 个生命周期钩子：Init / Shutdown。

5. 1 个私有成员 DebugTestEncryptionKey：硬编码加密密钥，注释标明"仅演示、不安全"。

6. 1 个便捷函数 GetPrimaryPlayerController：返回 Lyra 专用控制器类型。
```

**一句话**：`ULyraGameInstance.h` 声明了一个继承 `UCommonGameInstance` 的全局游戏实例，重写了**会话加入、用户登录、网络加密密钥协商**共 5 个父类虚函数，加了 `Init/Shutdown` 生命周期钩子，并用一个私有字节数组 `DebugTestEncryptionKey` 存演示用的加密密钥——从头文件即可看出它负责"登录 + 会话 + 加密"这三块全局事务。

---

## 五、下一步

- 想看每个函数**具体怎么实现**，另见 `.cpp` 详解（如需可单独再写一篇）。
- 对比父类 `UCommonGameInstance` 提供了哪些会话/用户能力。
- 看 `ReceivedNetworkEncryptionToken/Ack` 的父类定义（引擎网络加密握手流程）。
