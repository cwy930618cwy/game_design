# DTLS 是什么？—— 数据报传输层安全

> **定位**：解释 `LyraGameInstance.cpp` 第 32-46 行里反复出现的 **DTLS** 到底是什么。
>
> **一句话**：DTLS（Datagram Transport Layer Security，数据报传输层安全）是一种**给"不可靠传输"（UDP）做加密**的协议——相当于"给 UDP 穿上一件 TLS 加密铠甲"，让实时网络游戏的流量既加密、又不必像 TCP 那样慢。

---

## 一、先拆缩写

```
DTLS = Datagram Transport Layer Security
        数据报    传输层        安全
```

| 词 | 含义 |
|----|------|
| **Datagram（数据报）** | 指 **UDP** 这类"不保证送达、不保证顺序"的传输方式 |
| **Transport Layer Security（TLS）** | 就是 HTTPS 用的那套加密（TLS/SSL） |
| 合起来 | **把 TLS 加密搬到 UDP 上** |

> **一句话**：TLS 是给 TCP（可靠传输）加密的；DTLS 是给 UDP（数据报）加密的。

---

## 二、为什么需要 DTLS？（背景）

### 2.1 两种网络传输方式

| | TCP | UDP |
|---|---|---|
| 可靠送达 | ✅ 保证 | ❌ 不保证（可能丢包） |
| 顺序到达 | ✅ 保证 | ❌ 乱序 |
| 速度 | 较慢（有握手/重传开销） | **快**（轻量） |
| 典型用途 | 网页、文件、HTTPS | **实时游戏、语音、视频** |

### 2.2 游戏的两难

实时多人游戏（如射击、MOBA）**追求低延迟**，用 UDP（丢个包无所谓，下一帧补上就行，比重传快）。

但 UDP **不加密**——裸奔的流量能被窃听/篡改。

而 TLS（HTTPS 那套）**只支持 TCP**，直接用会引入 TCP 的"队头阻塞"，拖慢游戏。

> **矛盾**：游戏想要 **UDP 的快** + **TLS 的加密**，但两者天生不搭。

### 2.3 DTLS 就是解决方案

DTLS 把 TLS 改造成**能在 UDP 上跑**的版本：

```
TCP + TLS  =  HTTPS（安全但慢，适合网页）
UDP + DTLS =  安全且快，适合实时游戏 ★
```

> **类比**：
> - TLS 是"给邮政信件（TCP，保证送到）贴封条"。
> - DTLS 是"给飞镖/明信片（UDP，可能丢）也配上同样的封条"——既快又加密，丢了就丢了，但每张到的都是密封安全的。

---

## 三、DTLS 加密的"握手"过程（简化）

DTLS 建立加密连接，大致三步：

```
1. 客户端 → 服务器：我要连，这是我的"令牌/证书请求"
2. 服务器 → 客户端：这是我的"证书"（证明我是正版服务器）
3. 双方协商出"共享密钥" → 后续流量都用它加密
```

> 这就是为什么 `LyraGameInstance.cpp` 里有 `ReceivedNetworkEncryptionToken`（收令牌）、`ReceivedNetworkEncryptionAck`（确认）——它们正是这个握手在 UE 里的钩子。

**关键点**：DTLS 用**证书（Certificate）**来证明身份、交换密钥。证书有"指纹（Fingerprint）"——一串唯一标识，用来核对"这张证书是不是我信任的那张"。

---

## 四、回到代码：DTLS 相关的三个 CVar

第 32-46 行在 `#if UE_WITH_DTLS` 里，定义了**两个** DTLS 开关（都在 `Lyra.UseDTLSEncryption` 那段）：

### 4.1 `Lyra.UseDTLSEncryption`（总开关）

```cpp
static bool bUseDTLSEncryption = false;
static FAutoConsoleVariableRef CVarLyraUseDTLSEncryption(
    TEXT("Lyra.UseDTLSEncryption"),
    bUseDTLSEncryption,
    TEXT("Set to true if using Lyra.TestEncryption and the DTLS packet handler."),
    ECVF_Default);
```

- **作用**：DTLS 加密的**总开关**。
- 注释：要配合 `Lyra.TestEncryption` 和 DTLS 数据包处理器一起用。
- 默认 `false`（不开）。

### 4.2 `Lyra.TestDTLSFingerprint`（指纹测试开关）

```cpp
static bool bTestDTLSFingerprint = false;
static FAutoConsoleVariableRef CVarLyraTestDTLSFingerprint(
    TEXT("Lyra.TestDTLSFingerprint"),
    bTestDTLSFingerprint,
    TEXT("If true and using DTLS encryption, generate unique cert per connection and fingerprint will be written to file to simulate passing through an online service."),
    ECVF_Default);
```

- **作用**：模拟"线上服务"的证书指纹传递。
- 注释翻译：开启后，**为每个连接生成唯一证书**，并把**指纹写到文件**，用来模拟"通过一个在线服务传递指纹"的真实流程。
- 默认 `false`。

> **为什么写文件？** 真实项目里，服务器证书指纹应该通过**安全的在线服务**分发给客户端核对。本地测试没那个服务，就用"写文件→读文件"来模拟这个传递过程。

---

## 五、为什么这些代码被 `#if UE_WITH_DTLS` 包着？

（呼应上一篇 `#if` 预处理）

- **DTLS 是可选功能**，不是所有引擎编译时都启用了它。
- 只有引擎**编译时开启了 DTLS 支持**，`UE_WITH_DTLS` 宏才有值。
- 没开启时，这些 DTLS 代码被**编译前删除**，避免因找不到 DTLS 头文件（如 `DTLSCertStore.h`）而报错。

> 所以：`#if UE_WITH_DTLS` = "只有你的引擎带了 DTLS 功能，这些代码才存在"。

---

## 六、Lyra 的加密是"教学示范"（重要提醒）

代码注释反复强调：

> *"This is NOT SECURE, do not use this technique in production!"*（这不安全，生产别用）

Lyra 用 DTLS 演示**加密握手的流程**，但实现是简化的：
- 用**硬编码密钥**（`DebugTestEncryptionKey`）。
- 用**写文件**模拟在线证书服务。

**真实项目应该**：
- 从 **HTTPS 服务动态拉取密钥/证书**（而不是硬编码）。
- 用真正的证书颁发机构（CA）签发证书。
- 指纹通过**安全信道**传递核对。

> Lyra 是"教你 DTLS 怎么接"的样板，不是"可以直接上线"的安全实现。

---

## 七、一张图看懂 DTLS 在 Lyra 里的位置

```
【普通模式】（默认，bTestEncryption=false）
  客户端 ──明文UDP──→ 服务器     （不加密，裸奔）

【测试加密模式】（bTestEncryption=true, bUseDTLSEncryption=false）
  客户端 ──硬编码密钥加密──→ 服务器   （加密，但密钥写死，不安全）

【DTLS 模式】（两个都 true，且引擎编了 DTLS）★
  客户端 ──证书握手──→ 服务器
     │  用 DTLS 协商密钥
     │  校验证书指纹
     └──加密UDP流量──→ 服务器    （正规加密，接近生产可用）
```

---

## 八、常见疑问速答

| 疑问 | 答案 |
|------|------|
| DTLS 全称？ | Datagram Transport Layer Security（数据报传输层安全） |
| 和 TLS 啥关系？ | DTLS 是 TLS 的"UDP 版"，TLS 用于 TCP，DTLS 用于 UDP |
| 为什么游戏要用它？ | 游戏要 UDP 的低延迟 + 加密安全，DTLS 两者兼得 |
| DTLS 加密靠什么？ | 靠"证书"交换密钥；证书有"指纹"用于核对身份 |
| Lyra 的 DTLS 能直接上线吗？ | 不能，是教学示范（硬编码密钥），真实项目要接正规证书服务 |
| 为什么代码用 `#if UE_WITH_DTLS` 包着？ | DTLS 是可选功能，没启用时删掉这些代码避免编译报错 |
| `bTestDTLSFingerprint` 干嘛的？ | 模拟线上服务，每连接生成唯一证书并把指纹写文件 |

---

## 九、总结

```
DTLS = Datagram Transport Layer Security（数据报传输层安全）
  = 把 TLS 加密搬到 UDP 上
  = 让实时游戏既快（UDP）又安全（加密）

背景：
  TCP+TLS = HTTPS（安全但慢）
  UDP     = 快但不加密、裸奔
  UDP+DTLS = 又快又安全 ★（游戏想要的）

在 Lyra 里：
  Lyra.UseDTLSEncryption    DTLS 总开关
  Lyra.TestDTLSFingerprint  模拟证书指纹传递（写文件）
  全被 #if UE_WITH_DTLS 包着（可选功能，没编就删）

重要：Lyra 的 DTLS 是"教学示范"，硬编码密钥+写文件模拟，
     生产环境要接正规 HTTPS 证书服务，不能直接用。
```

**一句话**：DTLS 是**给 UDP 做加密的协议**（TLS 的 UDP 版），让实时网络游戏同时拥有 UDP 的低延迟和加密的安全性；Lyra 用它演示完整的加密握手流程（证书、指纹、密钥协商），但实现是硬编码密钥的教学版，生产环境需替换成正规证书服务。

---

## 十、下一步

- 看 UE 的 DTLS 相关类：`FDTLSCertStore`、`FDTLSCertificate`、`UDTLSHandlerComponent`（代码里出现的）。
- 回顾 `ReceivedNetworkEncryptionToken/Ack` 如何走 DTLS 握手（见 `03_ULyraGameInstance.cpp详解_实现篇.md`）。
- 了解 TLS/DTLS 的握手细节（证书链、非对称加密换对称密钥）。
- 对比 UE 的网络加密两种模式：普通包处理器 vs DTLS 包处理器。
