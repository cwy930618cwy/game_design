# 答疑 — `AESGCMHandlerComponent` 是干嘛的？有它没它有什么区别？

> 触发：看 `.uproject` 的插件列表，`AESGCMHandlerComponent` 只写了 `"Enabled": true`，看不出它到底在干什么。
> 实查：`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\PacketHandlers\AESGCMHandlerComponent\`（插件源）
> 　　　`D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\PacketHandlers\PacketHandler\Private\PacketHandler.cpp:207`（关键代码）
> 结论：**光启用插件 = 零区别。必须有 `DefaultEngine.ini` 里的 `[PacketHandlerComponents]` 段才真正生效；而我们和 Lyra 都没配 → 现在完全没作用。**

---

## 一、先说结论（3 句话）

1. **它是「网络封包加密组件」** —— 把客户端和服务端之间传输的 UDP 包用 **AES-256-GCM** 加密，防抓包 / 防篡改。
2. **启用插件 ≠ 开启加密**。加密组件必须在 `DefaultEngine.ini` 里显式登记，才会被挂到网络链路上。
3. **你的工程和 Lyra 都没登记** → 现在**有它没它，运行时完全没区别**（只多编一个 DLL）。

---

## 二、它到底是什么

### 2.1 官方描述（`.uplugin` 原文）

`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\PacketHandlers\AESGCMHandlerComponent\AESGCMHandlerComponent.uplugin`

```json
"FriendlyName" : "AES GCM network packet handler",
"Description" : "Provides a packet handler component to do AES GCM encryption and decryption.",
"EnabledByDefault" : false,
```

翻译：**「AES GCM 网络封包处理器 —— 提供一个封包处理组件，做 AES GCM 加解密。」**

`"EnabledByDefault": false` —— 引擎默认**不开**，必须工程自己写 `"Enabled": true` 才启用。

### 2.2 算法参数（头文件实查）

`AESGCMHandlerComponent\Source\Public\AESGCMHandlerComponent.h:27-34`

```cpp
// This handler uses AES256, which has 32-byte keys.
static const int32 KeySizeInBytes = 32;      // 密钥 32 字节 = AES-256
static const int32 BlockSizeInBytes = 16;    // 分组 16 字节
static const int32 IVSizeInBytes = 12;       // 初始向量 12 字节
static const int32 AuthTagSizeInBytes = 16;  // 认证标签 16 字节（GCM 的完整性校验）
```

**AES-256-GCM 是什么档次**：

| 项 | 说明 |
|---|---|
| AES-256 | 对称加密，256 位密钥 —— 目前商用到军用级的主流强度 |
| GCM 模式 | 加密 + **完整性认证**一体（auth tag 16 字节）→ 改一个 bit 就解不开，防篡改 |
| 用途 | 游戏网络防抓包：防止外挂直接读 UDP 包算敌人位置，防止伪造封包 |

### 2.3 核心接口（`AESGCMHandlerComponent.h:17-46`）

```cpp
class AESGCMHANDLERCOMPONENT_API FAESGCMHandlerComponent : public FEncryptionComponent
{
    // 设置密钥（必须正好 32 字节）
    virtual void SetEncryptionData(const FEncryptionData& EncryptionData) override;
    // 开启加密：之后的发送包全部加密
    virtual void EnableEncryption() override;
    // 关闭加密
    virtual void DisableEncryption() override;
    // 当前是否加密中
    virtual bool IsEncryptionEnabled() const override;
    // 收包时解密（HandlerComponent 接口）
    virtual void Incoming(FIncomingPacketRef PacketRef) override;
    // 发包时加密
    virtual void Outgoing(FBitWriter& Packet, FOutPacketTraits& Traits) override;
};
```

继承自 `FEncryptionComponent` —— 这是 UE 网络栈的「加密组件基类」，和 `FCompression`、`FPacketHandler` 是一类东西。

---

## 三、★ 关键：光启用插件不起作用

### 3.1 引擎怎么决定「要不要挂加密组件」

`D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\PacketHandlers\PacketHandler\Private\PacketHandler.cpp:207`

```cpp
FString EncryptionComponentName;
// ★ 从 DefaultEngine.ini 的 [PacketHandlerComponents] 段读 EncryptionComponent 键
if (GConfig->GetString(TEXT("PacketHandlerComponents"), TEXT("EncryptionComponent"),
                       EncryptionComponentName, GEngineIni) && !EncryptionComponentName.IsEmpty())
{
    // 还得看 cvar net.AllowEncryption 不为 0
    static IConsoleVariable* const AllowEncryptionCVar =
        IConsoleManager::Get().FindConsoleVariable(TEXT("net.AllowEncryption"));
    if (AllowEncryptionCVar == nullptr || AllowEncryptionCVar->GetInt() != 0)
    {
        // ★ 只有走到这里，才真的把加密组件插进 Handler 链
        EncryptionComponent = StaticCastSharedPtr<FEncryptionComponent>(AddHandler(EncryptionComponentName, true));
    }
    else
    {
        UE_LOG(PacketHandlerLog, Warning,
            TEXT("PacketHandler encryption component is configured as %s, but it won't be used because the cvar net.AllowEncryption is false."),
            *EncryptionComponentName);
    }
}
```

拆解成三个条件，**缺一不可**：

```
条件 ①：.uproject 里 "Enabled": true                    ← ✅ 我们有
条件 ②：DefaultEngine.ini 里 [PacketHandlerComponents]   ← ❌ 我们没有（Lyra 也没有）
          EncryptionComponent=AESGCMHandlerComponent
条件 ③：cvar net.AllowEncryption != 0                    ← ✅ 默认是 1
```

**条件 ② 没满足 → 整段 `if` 直接跳过 → 加密组件从来没被创建过。**

### 3.2 引擎自带的示例（就在注释里）

`D:\ue5\Epic Games\UE_5.4\Engine\Config\BaseEngine.ini:3295`

```ini
[PacketHandlerComponents]
; Options can be set in this section of DefaultEngine.ini to enable different types of network packet encruption plugins
; EncryptionComponent=AESGCMHandlerComponent
```

注意：**这两行是注释**（前面有 `;`）。引擎只是告诉你「想启用就写这一行」，默认是不启用的。

### 3.3 我们和 Lyra 的实查

```
我们：E:\ue5\game_design\code\Tenkaichi\Config\DefaultEngine.ini
      搜索 PacketHandler / EncryptionComponent / AESGCM  →  0 处

Lyra：E:\ue5\LyraStarterGame\Config\DefaultEngine.ini
      搜索 PacketHandler / EncryptionComponent / AESGCM  →  0 处

Lyra 源码里唯一提到 AESGCM 的地方：
      E:\ue5\LyraStarterGame\LyraStarterGame.uproject:28   "Name": "AESGCMHandlerComponent",
```

**Lyra 只是「启用插件」，也没配 ini** —— 说明 Lyra 把加密当「预留能力」，自己没开。

---

## 四、有它 vs 没它：分三层看

### 4.1 编译层（唯一真实差别）

| | 有 `AESGCMHandlerComponent` | 没有 |
|---|---|---|
| 编译产物 | 多一个 `UnrealEditor-AESGCMHandlerComponent.dll` | 没有这个 dll |
| 编译时间 | 略增（就 2 个 .cpp） | — |
| 打包体积 | 略增 | — |
| **是否报错** | 否 | 否（只要 `.uproject` 里不写它） |

实际源文件只有 2 个：

```
AESGCMHandlerComponent\Source\Private\AESGCMHandlerComponent.cpp
AESGCMHandlerComponent\Source\Private\AESGCMFaultHandler.cpp
```

> 编译开销可以忽略（几秒钟）。

### 4.2 运行时层（**现在 = 零区别**）

```
条件 ② 未满足（没配 ini）
   ↓
FAESGCMHandlerComponent 从未被 new 出来
   ↓
网络包明文传输，与没启用插件完全一样
```

**验证方法**（跑起来看日志）：

```
控制台输入：net.AllowEncryption
默认输出：net.AllowEncryption = "1"

但 Output Log 里搜 "AESGCM" → 0 条
搜 "PacketHandler" → 看不到 "EncryptionComponent" 相关行

因为引擎只有在 ini 配了之后才会 Log 相关内容
```

### 4.3 配了 ini 之后（才有区别）

如果你在 `DefaultEngine.ini` 加：

```ini
[PacketHandlerComponents]
EncryptionComponent=AESGCMHandlerComponent
```

那么：

| 场景 | 没配（现在） | 配了 |
|---|---|---|
| 客户端连服务器 | 明文 UDP | 握手时协商密钥 → 之后全加密 |
| 抓包工具（Wireshark） | 能看到完整封包结构 | 只看到密文 |
| 外挂改包 | 直接改数值 | 改了 → auth tag 校验失败 → 断线 |
| 性能开销 | 0 | 每包 AES-256 加解密（现代 CPU 有 AES-NI 指令，开销极小） |
| 联机延迟 | — | 几乎无影响（<0.1ms/包） |

**代价**：

| 代价 | 说明 |
|---|---|
| 调试变难 | 抓包看到的是密文，不能直接读字段 |
| 需要密钥管理 | 服务端生成密钥 → 通过其他安全通道（如 EOS / Steam）发给客户端 |
| `LyraGameEOS` 一类插件通常自带加密协商流程 | 单机 / 局域网红线开发时没必要开 |

---

## 五、那为什么 Lyra 还要启用它

三个理由，按重要性排：

| # | 理由 | 说明 |
|---|---|---|
| 1 | **源码准备好了** | 插件编译进 DLL → 将来要开只需改 1 行 ini，不用重新配工程 |
| 2 | **`LyraGame.Target.cs` 的 `-DTLSHandlerComponent` 配套** | 第 02 轮讲义里 `PrivateDependencyModuleNames` 最后一个是 `"DTLSHandlerComponent"` —— 这两个是**同一类插件**（`Runtime\PacketHandlers\` 下），Lyra 把整套网络加密方案都编进去了 |
| 3 | **官方示例工程的示范性** | Lyra 是「怎么用 UE 做 3A 级游戏」的范例，网络加密属于「该有的能力」 |

### 顺带：`DTLSHandlerComponent` 是它的姊妹插件

```
Engine\Plugins\Runtime\PacketHandlers\
├─ AESGCMHandlerComponent\     ← AES-256-GCM（对称加密，防抓包）
└─ DTLSHandlerComponent\       ← DTLS（基于 TLS 的 UDP 加密，用于 VoIP / 语音）
```

**两者都在 `LyraGame.Build.cs` 的依赖里**（`AESGCMHandlerComponent` 在公共依赖？不，实际是 `.uproject` 启用；`DTLSHandlerComponent` 在 `PrivateDependencyModuleNames` 最后一行）。

> 📌 也就是说：**插件启用 + `Build.cs` 依赖** 是两件事。
> - `.uproject` 的 `"Enabled": true` → 编译这个插件
> - `Build.cs` 的依赖 → 本模块的代码能 `#include` 这个插件的头文件
>
> `DTLSHandlerComponent` 两样都有（所以 `LyraGame` 代码里能引用它的类）；`AESGCMHandlerComponent` 只有 `.uproject` 启用（`LyraGame` 代码里不能直接 include，靠 ini 配置使用）。

---

## 六、顺带提一句

你贴的那段：

```jsonc
{
    // Actor调色板： 编辑器里把常用Actor存成面板，一键拖进场景
    "Name": "ActorPalette",
    "Enabled": true
},
```

这是**另一个插件**，跟 AESGCM 无关。它的详细讲解见同目录 `ActorPalette是干嘛的.md`。

> 📌 两者在 `.uproject` 里是相邻两项（第 1 / 第 2），但功能毫不相干：
> - `ActorPalette` = **Editor** 模块，编辑器素材面板，运行时零影响
> - `AESGCMHandlerComponent` = **Runtime** 模块，网络加密，配了 ini 才生效

---

## 七、一句话总结

**`AESGCMHandlerComponent` = AES-256-GCM 网络封包加密组件（防抓包 / 防改包）。但引擎只在 `DefaultEngine.ini` 的 `[PacketHandlerComponents]` 段写了 `EncryptionComponent=AESGCMHandlerComponent` 时才真正挂上去（`PacketHandler.cpp:207`）——我们和 Lyra 都没写，所以现在「有它没它」运行时零区别，只多编一个 DLL。保留它是 Lyra 原样 + 为将来开加密留的口子。**

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\PacketHandlers\AESGCMHandlerComponent\AESGCMHandlerComponent.uplugin:1-45`（`FriendlyName` / `Description` / `EnabledByDefault:false` / 依赖 `PlatformCrypto`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\PacketHandlers\AESGCMHandlerComponent\Source\Public\AESGCMHandlerComponent.h:14-76`（`FAESGCMHandlerComponent` 类、AES256 参数、5 个虚函数）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\PacketHandlers\AESGCMHandlerComponent\Source\Public\AESGCMHandlerComponent.h:82-87`（`FAESGCMHandlerComponentModule`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\PacketHandlers\PacketHandler\Private\PacketHandler.cpp:207-216`（从 `[PacketHandlerComponents]` 读 `EncryptionComponent` 的代码）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Private\NetConnection.cpp:807-824`（`UNetConnection::EnableEncryption`，握手时启用加密）
- `D:\ue5\Epic Games\UE_5.4\Engine\Config\BaseEngine.ini:3295-3297`（`[PacketHandlerComponents]` 段，`; EncryptionComponent=AESGCMHandlerComponent` 是注释）
- `E:\ue5\game_design\code\Tenkaichi\Config\DefaultEngine.ini`（实查：**无** `[PacketHandlerComponents]` 段）
- `E:\ue5\LyraStarterGame\Config\DefaultEngine.ini`（实查：**无** `[PacketHandlerComponents]` 段）
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject:28`（`"Name": "AESGCMHandlerComponent"`，全 Lyra 唯一一处）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ActorPalette\ActorPalette.uplugin:1-25`（`Actor Palette` 的描述与 `"Type": "Editor"`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ActorPalette\Source\ActorPalette\`（11 个源文件：`ActorPalette.cpp` / `ActorPaletteViewport.cpp` / `ActorPaletteSettings.cpp` 等）
