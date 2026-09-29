# 02 — `LyraGameInstance` 的 .h 和 .cpp 各干了什么（附 GameSession）

> **定位**：`01` 讲了进程第一站 `LyraGameEngine`。这一篇按"**先 .h 后 .cpp**"的读法，把第二站 `LyraGameInstance` 两个文件各自干了什么拆干净（附同目录 `LyraGameSession`）。
>
> **总读法**：`.h` = 菜单（声明有哪些菜）；`.cpp` = 后厨（每道菜怎么做，外加自己备料）。先看 .h 全貌，再看 .cpp 实现。

---

## 一、骨架大图：.h 声明了什么 ↔ .cpp 实现了什么

```
              ULyraGameInstance（进程第二站 · 总经理办公室）
 ═══════════════════════════════════════════════════════════════════
  .h（43行）名片：只声明                          .cpp（339行）后厨：逐个实现
 ┌────────────────────────────┐    ┌────────────────────────────┐
 │ class ULyraGameInstance    │    │ ① 顶部备料 L23-81          │
 │   : UCommonGameInstance    │    │   演示开关 CVar ×3 + 证书命令 │
 │────────────────────────────│    │    (纯测试用,非正式)          │
 │ public 方法 ×5：            │    │────────────────────────────│
 │ · GetPrimaryPlayerController│    │ ② 生命周期 L83-125          │
 │ · CanJoinRequestedSession   │    │   Init(): 注册组件状态机/    │
 │ · HandlerUserInitialized    │    │   填演示key/订阅进服事件      │
 │ · ReceivedNetworkEncryption │    │   Shutdown(): 退订收尾       │
 │   Token / Ack              │    │────────────────────────────│
 │────────────────────────────│    │ ③ 玩家相关 L127-158          │
 │ protected：                 │    │   GetPrimaryController: 强转 │
 │ · Init() / Shutdown()      │    │   CanJoinSession: 恒 true   │
 │ · OnPreClientTravel(私有)   │    │   HandlerUserInitialized:   │
 │────────────────────────────│    │   登录成功→玩家读磁盘设置     │
 │ private：                   │    │────────────────────────────│
 │ · DebugTestEncryptionKey    │    │ ④ 网络加密 L160-311         │
 │                            │    │   收到token→回key(演示)     │
 └────────────────────────────┘    │   收到Ack→确认(演示)         │
                                   │────────────────────────────│
                                   │ ⑤ 进服前 L313-339           │
                                   │   往 URL 塞加密token(可选)   │
                                   └────────────────────────────┘
 ═══════════════════════════════════════════════════════════════════
 一句话：.h 说"我有哪些接口"，.cpp 把每个接口做成真事；
        每个公开方法在 .cpp 里都有一个同名实现。
```

---

## 二、先读 `.h`：它只是"声明我有什么"

### `LyraGameInstance.h` 完整解剖

```
 LyraGameInstance.h（43行）                        │ 批注
 ═══════════════════════════════════════════════════┼════════════════
 #pragma once                                      │ 防重复包含
 #include "CommonGameInstance.h"                   │ 基类来自 CommonGame
 #include "LyraGameInstance.generated.h"            │ UHT 生成物
                                                   │
 UCLASS(MinimalAPI, Config=Game)                   │ 可配 Game 设置
 class ULyraGameInstance : public UCommonGameInstance │ 继承"通用实例"
 {                                                 │
   GENERATED_BODY()
 public:
   GetPrimaryPlayerController() const              │ 拿主玩家控制器
   CanJoinRequestedSession() const override        │ 能否加入目标会话
   HandlerUserInitialized(...) override            │ 平台账号初始化完回调
   ReceivedNetworkEncryptionToken(...) override    │ 服务端下发加密令牌
   ReceivedNetworkEncryptionAck(...) override      │ 加密确认回调
 protected:
   Init() override                                 │ ★进程启动初始化
   Shutdown() override                             │ 进程关闭收尾
   OnPreClientTravelToSession(URL)                 │ 私有：进服前改 URL
 private:
   TArray<uint8> DebugTestEncryptionKey            │ 演示用加密 key
 };
 ═══════════════════════════════════════════════════┴════════════════
```

**要点**：.h 只做三件事——**继承谁、暴露哪些方法、留哪些覆写点**。函数体一个都没有。

> 💡 **读法**：先看 `override` 关键字——凡是带它的（Init/Shutdown/加密回调…）都是"引擎到点会来敲门"的钩子；不带 override 的（GetPrimaryPlayerController）是给别人调用的工具方法。

---

## 三、再读 `.cpp`：每个方法在做什么

### ① 顶部"备料"（L23-81）：一堆演示开关

```cpp
static bool bTestEncryption = false;
static FAutoConsoleVariableRef CVarLyraTestEncryption(
	TEXT("Lyra.TestEncryption"), bTestEncryption,
	TEXT("If true, clients will send an encryption token..."), ...);
```

- `Lyra.TestEncryption`：打开后客户端进服时带加密 token（演示加密流程）
- `Lyra.UseDTLSEncryption`：改用 DTLS 证书加密（需配合上面的）
- `GenerateDTLSCertificate` 控制台命令：生成测试证书（非 Shipping 版才有）

> ⚠️ 整段都在文件注释和代码里反复标"NOT SECURE / for demonstration only"——**这是教学演示区，正式项目要把 key 换成 HTTPS 安全获取**。

### ② 生命周期（L83-125）：Init 是全场重点

| 方法 | 做了什么 |
|---|---|
| `构造函数` | 空，只传给父类 |
| `Init()` | ① 通过 `UGameFrameworkComponentManager` **注册组件初始化状态机** 4 阶段（`Spawned→DataAvailable→DataInitialized→GameplayReady`）；② 填 32 字节演示 AES key；③ 订阅 `CommonSessionSubsystem` 的 `OnPreClientTravelEvent` |
| `Shutdown()` | 反注册上面的订阅，干净收尾 |

> 💡 **重点**：`Init()` 第一件事注册的 4 个 InitState 标签，就是**全 Lyra 所有组件/Pawn"按顺序初始化"的总源头**——Lyra 每个组件都从 `Spawned` 一步步升到 `GameplayReady`，这个规矩在这里被"立"起来。

### ③ 玩家相关（L127-158）

| 方法 | 做了什么 |
|---|---|
| `GetPrimaryPlayerController()` | 把主控制器 `Cast` 成 `ALyraPlayerController` 返回（方便调用方直接用 Lyra 版接口） |
| `CanJoinRequestedSession()` | 调父类后**恒返回 true**（注释：以后要检查玩家状态——预留口） |
| `HandlerUserInitialized()` | 登录成功回调：找到 `ULyraLocalPlayer` → 调 `LoadSharedSettingsFromDisk()` 让玩家读上次保存的设置 |

> 💡 **场景**：玩家在平台登录成功 → 这一行就自动把他的画质/音量/按键设置读回来了——你在游戏里改过设置再重进，能记住，就是这条路。

### ④ 网络加密两回调（L160-311）

`ReceivedNetworkEncryptionToken` / `ReceivedNetworkEncryptionAck`：引擎在网络握手时的两个回调。Lyra 的版本逻辑是——token 非空就回 Success + 写死的演示 key；若开了 DTLS 则走证书存储/指纹那套（也都是测试路径）。

> 💡 什么时候会走到：客户端 `Lyra.TestEncryption=1` 进服 → 服务器要求加密 → 引擎回调到这里拿 key。正式项目应在这里改成"向 HTTPS 服务异步要 key"。

### ⑤ 进服前（L313-339）

`OnPreClientTravelToSession`：真正跳转服务器前被调，若开了测试加密就往 URL 追加 `?EncryptionToken=...`。

---

## 四、同目录配角：`LyraGameSession`（也按 h/cpp）

```
 ALyraGameSession : AGameSession（一局比赛的边界）
 ═══════════════════════════════════════════════════════════
  .h（28行）声明：                      .cpp（27行）实现：
 ┌──────────────────────────┐        ┌──────────────────────────┐
 │ 构造                      │        │ ProcessAutoLogin()       │
 │ ProcessAutoLogin() override│        │   return true;           │
 │ HandleMatchHasStarted()   │        │   // 自动登录已被 GameMode │
 │ HandleMatchHasEnded()     │        │   // 的 TryDedicatedServer│
 └──────────────────────────┘        │   // Login 接管,这里关掉  │
                                      │ HandleMatchHasStarted/Ended│
                                      │   → 只调父类(空钩子预留)   │
                                      └──────────────────────────┘
```

> 一句话：Lyra 的现代开局走 GameMode/Experience，所以引擎自带的 `GameSession` 登录被关掉，只剩"开局/终局"两个空钩子供将来插代码。

---

## 五、本篇一句话

**LyraGameInstance.h = 8 个成员的"菜单"；LyraGameInstance.cpp = 每个成员的后厨实现 + 顶部 CVar 备料**。`Init()` 注册全项目状态机是它最值钱的一行；加密相关全是演示。读它的顺序：`.h` 看清单 → `.cpp` 按 `Init → 玩家回调 → 加密 → 进服前` 四段看实现。`.cpp` 永远比 `.h` 长得多——因为它把声明变成真事。
