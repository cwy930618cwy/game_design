# 答疑：81 个插件里的「Epic 账号依赖」和「第三方 SDK」到底是什么

> **来源**：你问「严格符合 23/26 但会引入一堆射击示例 + 第三方 SDK + Epic 账号依赖」这句话里的「Epic 账号依赖」和「第三方 SDK」具体指哪些。
> **结论先行**：这俩都是 **81 个插件里「在线服务 / 平台」那一类插件带来的**。它们「只是被声明启用」，但**编译不依赖、跑起来也不用你配账号**——除非你走到 L7 联机阶段，那时才需要真的去 Epic/Steam 后台申请 ID、配密钥。

---

## 一、先分清：「启用插件」≠「现在就要连账号」

关键认知（别被吓到）：

- 你在 `.uproject` 里写 `"OnlineSubsystemEOS": { "Enabled": true }`，**只是告诉 UE「这个插件可以加载」**。
- 它**不会**在你打开工程/编译时去连 Epic 服务器、不会弹登录框、不会要求你输入账号。
- 真正「连 Epic 账号」发生在**运行时**——而且只有当你写的代码调用了 EOS 的登录/联机接口、并且配好了 `DefaultEngine.ini` 里的 EOS 凭证时才会发生。

**所以：L1~L6 全程，你碰不到这些账号/SDK，它们躺在 `.uproject` 里睡觉。只有 L7（联机）才会唤醒它们。**

---

## 二、「Epic 账号依赖」= EOS 系列插件

EOS = **Epic Online Services**（Epic 的在线服务平台）。要连 EOS，需要 **Epic 账号** + 在 Epic Dev Portal 建一个「产品」，拿到一串凭证。

你的 81 个插件里，属于 EOS 生态（Epic 账号依赖）的有：

| 插件名 | 作用 | 需要的「Epic 账号」相关东西 |
|--------|------|---------------------------|
| `OnlineSubsystemEOS` | 提供 EOS 在线子系统 | Dev Portal 产品 + `ProductId` / `SandboxId` / `DeploymentId` |
| `OnlineServicesEOS` | 新架构的 EOS 在线服务 | 同上 |
| `EOSShared` | 负责 EOS SDK 运行时库的 init/shutdown（**就是 EOS SDK 本体**） | 同上 |
| `SocketSubsystemEOS` | EOS 的 Socket 子系统 | 同上 |
| `EOSReservedHooks` | Epic 预留的 EOS 钩子（标记「这个位置留给 EOS 用」） | 同上 |
| `EOSOverlayInputProvider` / `EOSVoiceChat` | EOS 悬浮层输入 / 语音 | 同上 |

> **一句话**：这一串带 `EOS` 的插件，全都指向同一个东西——「你的游戏要接 Epic 的在线服务，得有 Epic 账号和产品凭证」。

### 关键事实（源码依据）

我读了引擎里的真实 `.uplugin`：

- `OnlineSubsystemEOS.uplugin`：`"Description": "Online Subsystem for Epic Online Services"`，`"CreatedByURL": "https://dev.epicgames.com/services"`，依赖 `EOSShared`、`EOSVoiceChat`、`SocketSubsystemEOS`、`EOSOverlayInputProvider`。
- `EOSShared.uplugin`：`"Description": "Responsible for init/shutdown of the EOSSDK runtime library."` —— 直接点名了 **EOSSDK**。

> 这两个文件证明：EOS 系列 = Epic 自家在线服务 SDK + 需要 Epic 账号。**不是引擎自带的免费功能，是要申请账号的。**

---

## 三、「第三方 SDK」= 非 Epic 的 SDK

「第三方」= 不是 Epic 官方的东西，是别的公司的 SDK。你的 81 个插件里属于「第三方 SDK」的有：

### 1. Steam 系列（Valve 的 Steamworks SDK）

| 插件名 | 作用 | 第三方 SDK |
|--------|------|-----------|
| `OnlineSubsystemSteam` | Steam 在线子系统 | **Steamworks SDK**（要 Steam 账号 + AppID） |
| `SocketSubsystemSteamIP` | Steam 的 Socket | 同上 |
| `SteamSockets` | Steam 的网络套接字 | 同上 |

> 源码依据：`OnlineSubsystemSteam.uplugin` 依赖 `SteamShared`（Steamworks 共享层），`"Description": "Access to Steam platform"`。

### 2. PlayFab 系列（微软的 PlayFab SDK）

| 插件名 | 作用 | 第三方 SDK |
|--------|------|-----------|
| `PlayFabParty` | 派对/语音（跨平台组队+聊天） | **PlayFab Party SDK**（微软，要 PlayFab 账号） |

> 它的 `PlatformAllowList` 是 `XB1`/`XSX`/`WinGDK`（主机 + GamePass），主要给 Xbox 平台用的。

### 3. 平台/VR SDK（各硬件厂商的 SDK）

| 插件名 | 第三方 SDK | 状态 |
|--------|-----------|------|
| `MagicLeap` / `MagicLeapMedia` / `MagicLeapPassableWorld` | Magic Leap 眼镜 SDK | `Enabled: false` |
| `MLSDK` | Magic Leap SDK | `false` |
| `OpenXR` 系列（`OpenXRHMD`/`OpenXRHandTracking`/`OpenXREyeTracker`） | OpenXR 运行时 | `false` |
| `SteamVR` | Valve SteamVR | `false` |
| `GearVR` | Oculus GearVR | `false` |
| `LuminPlatformFeatures` | Magic Leap Lumin 平台 | `false` |

> 这些 VR/AR 的第三方 SDK，Lyra 本来就是 `Enabled: false`，**你照抄成 `false` 就等于「没启用」**，完全不碰它们。

---

## 四、总结表：三类「依赖」一句话对照

| 类别 | 代表插件 | 什么时候需要真的配 | 现在（L1~L6）要不要管 |
|------|---------|------------------|---------------------|
| **Epic 账号依赖** | `OnlineSubsystemEOS`、`OnlineServicesEOS`、`EOSShared`、`EOSReservedHooks` | L7 联机时，去 Epic Dev Portal 申请产品 + 凭证 | ❌ 不用管 |
| **第三方 SDK（Steam）** | `OnlineSubsystemSteam`、`SteamSockets`、`SocketSubsystemSteamIP` | L7 联机时，去 Steamworks 申请 AppID | ❌ 不用管 |
| **第三方 SDK（PlayFab）** | `PlayFabParty` | 做 Xbox/跨平台派对时 | ❌ 不用管 |
| **第三方 SDK（VR/AR）** | `MagicLeap`、`OpenXR`、`SteamVR`、`GearVR`、`MLSDK` | 做 VR/AR 平台时 | ❌ 本来就 `false` |
| **射击示例内容** | `ShooterCore`、`ShooterMaps`、`TopDownArena`、`ShooterExplorer`、`ShooterTests` | 参考学习（不删，看 Lyra 怎么做的） | ✅ 要留着（教程当参考） |

---

## 五、回到你的核心疑问

你担心「严格符合 23/26 会引入一堆 Epic 账号依赖 + 第三方 SDK」。

**答案是：会引入，但只是「声明层面」引入，不是「实际接入」。**

- 它们在 `.uproject` 里占一行，**不增加编译负担**（不启用就不参与编译——虽然它们大多是 `true`，但只有真正被 Build.cs 依赖到 `OnlineSubsystem` 之类模块时才编译，而 L1~L6 的 `TenkaichiGame.Build.cs` **没依赖任何 Online 模块**）。
- 它们**不会让你现在就去注册 Epic/Steam/PlayFab 账号**。
- 只有 L7（联机）才会用到，到时再讲「怎么申请凭证、怎么配 `DefaultEngine.ini`」。

> 这也正是「一比一还原」的意义：Lyra 是**完整可联机的范例**，它的 `.uproject` 把这些在线服务都声明好了。你照抄，是为了「工程结构和 Lyra 一模一样」，而不是「现在就要连 Epic 服务器」。等 L7 需要时，这些插件早就在位，不用回头补。

---

## 六、一句话记住

- **Epic 账号依赖** = 带 `EOS` 的那串插件（连 Epic Online Services，要 Epic 账号 + 产品凭证）。
- **第三方 SDK** = Steam（Steamworks）、PlayFab（Party）、MagicLeap/OpenXR/SteamVR/GearVR（VR/AR 厂商 SDK）。
- **现在都不用管**，躺在 `.uproject` 里睡觉；**L7 联机才唤醒**；**VR/AR 那批本来就是 `false`**。