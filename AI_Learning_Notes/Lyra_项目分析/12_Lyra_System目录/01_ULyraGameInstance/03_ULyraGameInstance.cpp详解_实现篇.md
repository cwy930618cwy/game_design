# ULyraGameInstance.cpp 详解 —— 实现篇

> **定位**：讲透 `LyraGameInstance.cpp` 的**实现逻辑**（配合 `.h` 头文件看）。
>
> **源码位置**：`Source/LyraGame/System/LyraGameInstance.cpp`
>
> **一句话**：这个 `.cpp` 实现了三块逻辑——**① 启动时注册 InitState 状态链 + 绑定回调、② 用户登录加载设置、③ 网络加密密钥协商（含 DTLS）**，并处理客户端旅行时往 URL 塞加密令牌。

---

## 一、文件结构总览

`.cpp` 从上到下分 4 块：

```
1. #include 区          （引入依赖）
2. namespace Lyra {}    （CVar 控制台变量 + DTLS 命令）
3. 构造 / Init / Shutdown（生命周期）
4. 各个成员函数实现      （登录、加密、旅行等）
```

---

## 二、第 1 块：include 依赖

```cpp
#include "LyraGameInstance.h"
#include "CommonSessionSubsystem.h"      // 会话子系统
#include "CommonUserSubsystem.h"         // 用户子系统（登录）
#include "Components/GameFrameworkComponentManager.h"  // 大管家 ★
#include "LyraGameplayTags.h"            // InitState 那些 Tag ★
#include "Player/LyraPlayerController.h"
#include "Player/LyraLocalPlayer.h"
...
#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameInstance)  // 反射生成代码
```

**关键**：引入了 `GameFrameworkComponentManager.h`（大管家）和 `LyraGameplayTags.h`——因为 `Init()` 里要注册初始化状态。

---

## 三、第 2 块：`namespace Lyra` —— CVar 与 DTLS 命令

这一整块是**控制台变量（CVar）和调试命令**，控制加密行为。

### 3.1 三个 CVar（开关）

```cpp
static bool bTestEncryption = false;
static FAutoConsoleVariableRef CVarLyraTestEncryption(
    TEXT("Lyra.TestEncryption"), bTestEncryption,
    TEXT("If true, clients will send an encryption token..."), ECVF_Default);
```

| CVar | 变量 | 作用 | 默认 |
|------|------|------|------|
| `Lyra.TestEncryption` | `bTestEncryption` | 开启加密令牌测试 | false |
| `Lyra.UseDTLSEncryption` | `bUseDTLSEncryption` | 启用 DTLS 证书加密（需 `UE_WITH_DTLS`） | false |
| `Lyra.TestDTLSFingerprint` | `bTestDTLSFingerprint` | 每连接生成唯一证书，指纹写文件 | false |

> **`FAutoConsoleVariableRef`**：UE 的机制，把 C++ 变量和控制台命令自动绑定。你在游戏里按 `~` 输入 `Lyra.TestEncryption 1` 就能改它。

### 3.2 一个调试命令（仅非 Shipping）

```cpp
#if !UE_BUILD_SHIPPING
static FAutoConsoleCommandWithWorldAndArgs CmdGenerateDTLSCertificate(
    TEXT("GenerateDTLSCertificate"),
    TEXT("Generate a DTLS self-signed certificate..."),
    FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](...){
        // 生成证书并导出 PEM 到 Content/DTLS/
    }));
#endif
```

- 控制台命令 `GenerateDTLSCertificate <名字>`，生成自签名 DTLS 证书。
- `#if !UE_BUILD_SHIPPING`：**只在开发/测试版本生效**，正式发行版编译时会被剔除。

### 3.3 条件编译 `#if UE_WITH_DTLS`

- DTLS（数据报传输层安全）相关代码全被 `#if UE_WITH_DTLS` 包着。
- 只有引擎启用了 DTLS 支持时，这些代码才参与编译。

---

## 四、第 3 块：生命周期（构造 / Init / Shutdown）★重点

### 4.1 构造函数

```cpp
ULyraGameInstance::ULyraGameInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)   // 把初始化器传给父类
{
}
```
- 空实现，只把 `ObjectInitializer` 转交给父类 `UCommonGameInstance`。

---

### 4.2 `Init()` —— 干了三件大事 ★

```cpp
void ULyraGameInstance::Init()
{
    Super::Init();

    // ① 注册自定义初始化状态（4 个，串成链）
    UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);
    if (ensure(ComponentManager))
    {
        ComponentManager->RegisterInitState(LyraGameplayTags::InitState_Spawned,        false, FGameplayTag());
        ComponentManager->RegisterInitState(LyraGameplayTags::InitState_DataAvailable,  false, LyraGameplayTags::InitState_Spawned);
        ComponentManager->RegisterInitState(LyraGameplayTags::InitState_DataInitialized,false, LyraGameplayTags::InitState_DataAvailable);
        ComponentManager->RegisterInitState(LyraGameplayTags::InitState_GameplayReady,  false, LyraGameplayTags::InitState_DataInitialized);
    }

    // ② 初始化硬编码加密密钥（32字节 = AES256，值为 0~31）
    DebugTestEncryptionKey.SetNum(32);
    for (int32 i = 0; i < DebugTestEncryptionKey.Num(); ++i)
        DebugTestEncryptionKey[i] = uint8(i);

    // ③ 绑定"客户端旅行前"回调
    if (UCommonSessionSubsystem* SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>())
    {
        SessionSubsystem->OnPreClientTravelEvent.AddUObject(this, &ULyraGameInstance::OnPreClientTravelToSession);
    }
}
```

**① 是核心**——向**大管家**注册 4 个 `InitState` 状态，并串成一条**状态链**：

```
Spawned → DataAvailable → DataInitialized → GameplayReady
（每个状态的"前置"是上一个）
```

`RegisterInitState(状态, false, 前置状态)` 三个参数：
- 第 1 个：要注册的状态 Tag
- 第 2 个：`false`（是否作为"通知状态"，这里都是 false）
- 第 3 个：前置状态（依赖谁）——`FGameplayTag()` 空表示 `Spawned` 没有前置

> **意义**：这就是把 `LyraGameplayTags` 里那 4 个 `InitState.*` Tag **真正启用**的地方！之后角色/组件的初始化就按这条链推进。这是 Lyra 初始化架构的**启动点**。

**②** 把密钥设成 `0,1,2,...,31`（演示用，极不安全）。

**③** 用 `AddUObject` 把 `OnPreClientTravelToSession` 绑定到会话子系统的"旅行前"事件。

> **`GetSubsystem<T>()`**：UE 的子系统获取方式——从当前对象拿到某个子系统实例（大管家、会话子系统都是子系统）。

---

### 4.3 `Shutdown()` —— 对称清理

```cpp
void ULyraGameInstance::Shutdown()
{
    if (UCommonSessionSubsystem* SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>())
    {
        SessionSubsystem->OnPreClientTravelEvent.RemoveAll(this);  // 解绑③里绑的回调
    }
    Super::Shutdown();
}
```

- 把 `Init()` ③ 里绑定的回调**移除**（`RemoveAll(this)` 移除所有属于本对象的绑定）。
- **为什么**：防止对象销毁后，事件触发时还去调一个已失效的对象（悬空指针崩溃）。
- `Init` 绑、`Shutdown` 解——**对称清理**是好习惯。

---

## 五、第 4 块：成员函数实现

### 5.1 `GetPrimaryPlayerController()`

```cpp
ALyraPlayerController* ULyraGameInstance::GetPrimaryPlayerController() const
{
    return Cast<ALyraPlayerController>(Super::GetPrimaryPlayerController(false));
}
```

- 调父类 `UCommonGameInstance::GetPrimaryPlayerController(false)` 拿到主控制器。
- `Cast<ALyraPlayerController>`：UE 的**安全向下转型**——把父类指针转成 Lyra 子类指针（如果不是该类型会返回 null，不会崩）。
- 目的：让调用方直接拿到 Lyra 专用控制器，能用其特有功能。

> 上一篇专门讲过这行的语法（`UE_API`、返回类型、`const`），这里只看实现。

---

### 5.2 `CanJoinRequestedSession()`

```cpp
bool ULyraGameInstance::CanJoinRequestedSession() const
{
    if (!Super::CanJoinRequestedSession())  // 父类先判断
        return false;
    return true;                            // 目前无脑返回 true
}
```

- 注释写着 *"Temporary first pass: Always return true"*——Epic 留的**扩展点**。
- 将来会在这里检查玩家状态（是否在局中、是否被封禁）再决定能否加入。

---

### 5.3 `HandlerUserInitialized()` —— 登录回调

```cpp
void ULyraGameInstance::HandlerUserInitialized(
    const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ...)
{
    Super::HandlerUserInitialized(...);

    if (bSuccess && ensure(UserInfo))   // 登录成功
    {
        ULyraLocalPlayer* LocalPlayer = Cast<ULyraLocalPlayer>(GetLocalPlayerByIndex(UserInfo->LocalPlayerIndex));
        if (LocalPlayer)                // 有本地玩家（专用服务器上没有）
        {
            LocalPlayer->LoadSharedSettingsFromDisk();  // 从磁盘加载设置（键位/画质等）
        }
    }
}
```

- 登录成功后，找到对应的 `ULyraLocalPlayer`，让它**从磁盘加载共享设置**。
- 两层判空：`ensure(UserInfo)` + `if (LocalPlayer)`——因为**专用服务器没有本地玩家**。

---

### 5.4 `ReceivedNetworkEncryptionToken()` —— 服务器侧密钥协商 ★

这是**最复杂**的函数。逻辑骨架（简化）：

```cpp
void ULyraGameInstance::ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate)
{
    FEncryptionKeyResponse Response(EEncryptionResponse::Failure, TEXT("Unknown..."));

    if (EncryptionToken.IsEmpty())          // 令牌为空 → 返回无效
    {
        Response.Response = EEncryptionResponse::InvalidToken;
    }
    else
    {
        #if UE_WITH_DTLS
        if (Lyra::bUseDTLSEncryption)       // 用 DTLS 证书加密
        {
            // 创建/加载证书 → 设置 Identifier + Key
            Response.Response = EEncryptionResponse::Success;
        }
        else
        #endif
        {
            Response.Response = EEncryptionResponse::Success;
            Response.EncryptionData.Key = DebugTestEncryptionKey;  // 直接用硬编码密钥
        }
    }
    Delegate.ExecuteIfBound(Response);   // 通过委托返回结果
}
```

**关键点**：
- 用 `FEncryptionKeyResponse` 封装结果（成功/失败/无效 + 密钥）。
- 最后一定调 `Delegate.ExecuteIfBound(Response)` 把结果返回。
- **非 DTLS 分支**：直接用硬编码的 `DebugTestEncryptionKey`（演示，不安全）。
- **DTLS 分支**：走证书流程（创建/加载证书、可能写指纹文件）。

> 注释反复强调：*"This is NOT SECURE"*。真实项目应从 HTTPS 服务动态拉密钥，而不是硬编码。

---

### 5.5 `ReceivedNetworkEncryptionAck()` —— 客户端侧密钥确认

- 与上一个对称，是**客户端侧**确认密钥的流程。
- 结构类似：先构造 `Response`，DTLS 分支里从玩家唯一 ID 重建令牌、拉取服务器证书指纹，最后 `Delegate.ExecuteIfBound(Response)`。
- 核心仍是：拿到密钥 `DebugTestEncryptionKey` 返回。

---

### 5.6 `OnPreClientTravelToSession()` —— 往 URL 塞令牌

```cpp
void ULyraGameInstance::OnPreClientTravelToSession(FString& URL)
{
    if (Lyra::bTestEncryption)            // CVar 开关
    {
        #if UE_WITH_DTLS
        if (Lyra::bUseDTLSEncryption)
        {
            // 用玩家唯一ID作为令牌
            URL += TEXT("?EncryptionToken=") + EncryptionToken;
        }
        else
        #endif
        {
            URL += TEXT("?EncryptionToken=1");  // 演示用固定值
        }
    }
}
```

- 由 CVar `Lyra.TestEncryption` 控制（默认关）。
- 往连接 URL 后面**拼接** `?EncryptionToken=xxx`（URL 查询参数形式）。
- 服务器收到连接后，在 `ReceivedNetworkEncryptionToken` 里据此令牌协商密钥。
- **呼应**：这个函数正是 `Init()` ③ 里绑定的回调。

---

## 六、整体流程图

```
【启动】
Init()
 ├─ ① 向大管家注册 InitState 4 阶段状态链 ★
 ├─ ② 初始化硬编码密钥（0~31）
 └─ ③ 绑定 OnPreClientTravelToSession 到会话子系统

【运行时】
用户登录 ─→ HandlerUserInitialized ─→ 加载本地玩家设置
客户端要连服务器 ─→ OnPreClientTravelToSession ─→ URL 塞 ?EncryptionToken=
服务器收到令牌 ─→ ReceivedNetworkEncryptionToken ─→ 协商密钥返回
客户端确认   ─→ ReceivedNetworkEncryptionAck    ─→ 确认密钥返回

【退出】
Shutdown()
 └─ 解绑 OnPreClientTravelToSession（对称清理）
```

---

## 七、核心要点

```
1. Init() 是灵魂：
   向大管家注册 InitState 4 状态链（Spawned→...→GameplayReady），
   这是 Lyra 初始化架构的启动点（呼应 LyraGameplayTags 的 4 个 Tag）。

2. 对称清理：Init 绑定回调，Shutdown 解绑，避免悬空崩溃。

3. 加密是"教学示范"：
   硬编码密钥（0~31），注释反复强调不安全；
   真实项目应服务端动态下发，可选 DTLS 证书（CVar 控制）。

4. 三组 CVar：Lyra.TestEncryption / UseDTLSEncryption / TestDTLSFingerprint。

5. 条件编译：#if UE_WITH_DTLS 包 DTLS 代码；#if !UE_BUILD_SHIPPING 包调试命令。

6. 类型转换：GetPrimaryPlayerController 用 Cast<> 把父类控制器转成 Lyra 类型。

7. 两处判空：登录回调里防专用服务器无本地玩家。
```

**一句话**：`LyraGameInstance.cpp` 的核心是 `Init()`——向**大管家**注册 `InitState` 四阶段状态链（Lyra 初始化架构的启动点），并绑定旅行回调；运行时处理登录加载设置、网络加密密钥协商（硬编码密钥的教学示范，可选 DTLS）、旅行时往 URL 塞令牌；`Shutdown()` 对称解绑。整体围绕"**初始化状态 + 登录 + 加密**"三条线展开。

---

## 八、下一步

- 深入 `UGameFrameworkComponentManager::RegisterInitState`（大管家如何存这条状态链）。
- 看角色/组件如何用 `SetInitState` 推进这 4 个状态。
- 了解 UE 网络加密握手的完整流程（引擎层 `ReceivedNetworkEncryptionToken` 的调用时机）。
- 看 `UCommonSessionSubsystem::OnPreClientTravelEvent` 何时触发。
