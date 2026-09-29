# 99 — `ALyraPawn` 学完总结：`.h` 和 `.cpp` 各干嘛的

> **定位**：把整个 `ALyraPawn` 系列串成一张全景图。你发现"大部分都在操作 team"——这个观察非常准，这篇就从这个角度收尾。
>
> **不讲代码**，只讲"每个文件在干什么、这些 team 相关的东西是怎么串起来的"。

---

## 一、一句话看懂 `ALyraPawn` 是个啥

> **`ALyraPawn` = Lyra 里"能被玩家/AI 控制的角色身体"，它最核心的一件事，就是管理"队伍（Team）"。**

它是 `AModularPawn` 的子类，同时实现了 `ILyraTeamAgentInterface` 接口——也就是说，它既是一个游戏里的 Pawn，又具备了"拥有队伍身份"的能力。

```
ALyraPawn 的两个身份：
  • 身体（Pawn）：能被控制器附身/退出（PossessedBy / UnPossessed）
  • 队员（TeamAgent）：有队伍 ID，能广播"我换队了"
```

---

## 二、你的核心观察：为什么"大部分都在操作 team"？

因为 **`ALyraPawn` 存在的意义之一，就是把"队伍"这件事挂在 Pawn 身上**。整个类的逻辑几乎都围绕一个中心变量转：

```
                    ┌──────────────┐
                    │   MyTeamID   │  ← 一切的核心：这个 Pawn 属于哪一队
                    └──────┬───────┘
           ┌───────────────┼───────────────┐
           ▼               ▼               ▼
      被附身时跟队伍走   变化了就广播    同步给所有客户端
     (Possessed/UnPossessed) (Broadcast)  (Replication)
```

**为什么会这样？** 因为 Lyra 是团队竞技游戏，"谁在哪一队"是最基础的信息——队友识别、友伤判定、胜负归属全靠它。所以引擎把这件最重要的事，直接做进了 Pawn 里。

---

## 三、`.h`（头文件）在干嘛？——" declarations（声明）"

`.h` 是**蓝图**，负责"**告诉世界'我有什么'**"，不关心"怎么实现"。它干了这几件事：

### ① 定义类的骨架 + 两个身份

- 声明 `ALyraPawn` 继承自 `AModularPawn`、实现 `ILyraTeamAgentInterface`。
- 也就是确立"我是个有队伍身份的 Pawn"。

### ② 暴露给外界看的函数（接口）

- `SetGenericTeamId` / `GetGenericTeamId` / `GetOnTeamIndexChangedDelegate` —— 这三个是**接口要求实现的**，让外部能"改队伍、查队伍、监听队伍变化"。
- `PossessedBy` / `UnPossessed` —— 重写引擎的"被附身/退出"时机。

### ③ 存放核心成员变量

- **`MyTeamID`** —— 全场最重要的变量，这个 Pawn 的队伍身份就存这里。
- **`OnTeamChangedDelegate`** —— "队伍变了"的通知器（委托），别人可以订阅它。

### ④ 前向声明（打招呼）

- `class AController;` / `class UObject;` —— 业务真用到（函数参数）。
- `struct FFrame;` —— UE 反射体系的通用占位，UHT 生成的代码会用到。

> **`.h` 的一句话总结**：**列清单**——"我有这些函数、这两个变量、认识这几个类型"。至于每个函数内部具体怎么算，`.h` 不管。

---

## 四、`.cpp`（源文件）在干嘛？——"implementations（实现）"

`.cpp` 是**施工图**，负责"**具体怎么做**"。它把 `.h` 里声明的每个函数，写出真正的执行逻辑。按功能分成几块：

### ① 网络复制注册（开头那几行）

- `GetLifetimeReplicatedProps` + `DOREPLIFETIME(MyTeamID)` —— 把 `MyTeamID` 登记成"要全网同步"。
- **目的**：让所有客户端看到的队伍一致。

### ② 被附身 / 退出（PossessedBy / UnPossessed）

- 这是 team 逻辑的**触发点**。
- 被附身时：Pawn 跟着控制器的队伍走。
- 退出时：`DetermineNewTeamAfterPossessionEnds` 决定新队伍（默认变无队伍），然后 `ConditionalBroadcastTeamChanged` 广播变化。
- **核心**：队伍一变，就发通知。

### ③ 队伍接口的实现（Set / Get / GetDelegate）

- `SetGenericTeamId` —— 改 `MyTeamID`，如果真变了就广播。
- `GetGenericTeamId` —— 返回 `MyTeamID`。
- `GetOnTeamIndexChangedDelegate` —— 把 `OnTeamChangedDelegate` 交出去给别人订阅。

### ④ 回调函数（真正干活的地方）

- `OnControllerChangedTeam` —— 控制器的队伍变了时被触发，进而更新 Pawn 自己的队伍并广播。
- `OnRep_MyTeamID` —— 客户端收到同步来的 `MyTeamID` 时被触发（配合网络复制）。

> **`.cpp` 的一句话总结**：**把 team 这条线跑通**——登记同步、在附身时机更新队伍、变化时广播、响应回调。

---

## 五、把这些串起来：team 信息的一生

```
【服务器】                                              【所有客户端】

① 控制器附身 Pawn（PossessedBy）
      ↓
② Pawn 跟着拿到队伍 MyTeamID = X
      ↓
③ SetGenericTeamId / 或 OnControllerChangedTeam 更新它
      ↓
④ 发现队伍变了 → ConditionalBroadcastTeamChanged
      ↓                        ↓
   本地监听者收到          DOREPLIFETIME 已登记
   （UI 等更新）          → 引擎打包发送 ───────► ⑤ 客户端收到
                                                          ↓
                                                     ⑥ OnRep_MyTeamID
                                                          ↓
                                                     ⑦ 同样广播"我变了"
                                                        → 客户端 UI 更新
```

**贯穿始终的中心**：`MyTeamID`。`.h` 负责把它和相关的函数"声明出来"，`.cpp` 负责在合适的时机"读写它、同步它、广播它"。

---

## 六、`.h` vs `.cpp` 对照表

| | `.h`（头文件 / 蓝图） | `.cpp`（源文件 / 施工图） |
|---|---|---|
| **职责** | 声明"我有什么" | 实现"具体怎么做" |
| **关于 team** | 声明 `MyTeamID`、队伍接口、委托 | 读写 `MyTeamID`、同步、广播 |
| **关于网络** | 用 `UPROPERTY(ReplicatedUsing=...)` 标注 | 用 `DOREPLIFETIME` 真正登记 |
| **关于回调** | 声明 `OnControllerChangedTeam`、`OnRep_MyTeamID` | 写出它们的触发逻辑 |
| **关于类型** | 前向声明 AController/UObject/FFrame | include 完整头文件使用 |
| **类比** | 菜单（告诉你有哪些菜） | 后厨（实际做菜） |

---

## 七、学完这一篇，你应该记住的

1. **`ALyraPawn` = 有队伍身份的角色身体**，核心是管理 `MyTeamID`。
2. **"大部分都在操作 team"是对的**——因为团队竞技里"属于哪一队"是最基础的信息，引擎把它做进了 Pawn。
3. **`.h` 管声明（有什么），`.cpp` 管实现（怎么做）**——`.h` 是菜单，`.cpp` 是后厨。
4. **team 的逻辑闭环**：附身时更新 → 变化时广播 → 网络同步 → 客户端回调。中心永远是 `MyTeamID`。
5. **那些"等别人调我"的函数**（PossessedBy、OnRep_、GetLifetimeReplicatedProps）都是引擎在特定时机回调你，你只负责"被问到时代替回答"。

---

## 八、下一步

`ALyraPawn` 到此告一段落。接下来可以往这些方向深入：

- **队伍系统的另一侧**：`LyraTeamSubsystem`（队伍子系统，管理所有队伍）。
- **控制器侧**：`ALyraPlayerController` / `ALyraController`（大脑怎么决定队伍）。
- **模块化**：`AModularPawn` + `PawnData` 的组件化机制（Lyra 的核心设计模式）。
- **战斗/能力**：GAS（GameplayAbilitySystem）如何与队伍交互（比如友伤判定）。
