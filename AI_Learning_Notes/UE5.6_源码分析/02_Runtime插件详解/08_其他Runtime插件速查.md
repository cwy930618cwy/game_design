# 08 - 其他 Runtime 插件速查

> 涉及插件：`CommonUser` / `CommonConversation` / `UIExtension` / `SmartObjects` / `GameplayInteractions` / `GameplayStateTree` / `AnimationWarping` / `AnimationLocomotionLibrary` / `AsyncMixin` / `GameplayMessageRouter` / `GameSettings` / `PocketWorlds` / `Water`
> Lyra 使用度：⭐（辅助功能）

---

## 一、CommonUser — 用户身份抽象

### 1.1 核心功能

跨平台用户身份管理：

```cpp
// 获取本地玩家
ICommonUser* LocalUser = CommonUserSubsystem->GetLocalUser(PlayerIndex);

// 检查登录状态
bool bIsLoggedIn = LocalUser->IsLoggedIn();

// 获取在线身份
FUniqueOnlineId OnlineId = LocalUser->GetOnlineId();
```

### 1.2 Lyra 中的用途

- 分屏玩家识别
- 在线/离线模式判断
- 用户数据关联

---

## 二、CommonConversation — 对话系统

### 2.1 核心功能

提供对话/聊天的基础框架：

| 类 | 作用 |
|----|------|
| `UCommonConversationComponent` | 对话组件 |
| `UCommonConversationInstance` | 对话实例 |
| `FCommonConversationHandle` | 对话句柄 |

### 2.2 Lyra 中的用途

- 游戏内聊天
- 语音通话集成
- 消息通知

---

## 三、UIExtension — UI 扩展点

### 3.1 核心功能

允许 GameFeature **动态注入 UI** 到现有布局：

```cpp
// GameFeature 里添加 UI 扩展
UIExtensionSystem->AddWidgetExtension(
    "LyraHUDLayout",           // 目标布局
    "HealthBar",                // 扩展点名称
    HealthBarWidgetClass        // 要注入的控件
);
```

### 3.2 Lyra 中的用途

- ShooterCore 注入准心
- 不同 GameFeature 注入各自的 HUD 元素

---

## 四、SmartObjects — 智能对象

### 4.1 核心功能

为场景中的物体提供**可交互的行为槽**：

```cpp
// 定义一个 SmartObject（如椅子）
FSmartObjectSlot Slot;
Slot.AddBehavior(USeatBehavior::StaticClass());

// AI 或玩家查询可用槽位
FSmartObjectClaimQuery Query;
Query.Filter = FSmartObjectSlotFilter().SetBehavior(SEAT);
```

### 4.2 Lyra 中的用途

- 可坐的椅子
- 可拾取的物品
- 可交互的门

---

## 五、GameplayInteractions — 交互系统

### 5.1 核心功能

统一的交互框架：

| 类 | 作用 |
|----|------|
| `ULyraInteractionOptionComponent` | 交互选项组件 |
| `FLyraInteractionOption` | 交互选项 |
| `ILyraInteractable` | 可交互接口 |

### 5.2 Lyra 中的用途

- 拾取物品
- 开门
- 使用机关

---

## 六、GameplayStateTree — 状态树

### 6.1 核心功能

可视化的状态机工具，用于：

- AI 行为
- 游戏流程
- 动画状态

### 6.2 Lyra 中的用途

- Bot AI 行为
- 游戏阶段转换
- 复杂状态管理

---

## 七、AnimationWarping — 动画扭曲

### 7.1 核心功能

让动画适配不同的地形/目标：

- 脚步适配台阶
- 手部抓取定位
- 翻越障碍

### 7.2 Lyra 中的用途

- 上下楼梯脚步
- 拾取物品手部定位

---

## 八、AnimationLocomotionLibrary — 移动动画库

### 8.1 核心功能

预制的移动动画混合逻辑：

- 走/跑/跳过渡
- 急停/转身
- 斜坡适应

### 8.2 Lyra 中的用途

- 角色移动动画
- 运动匹配基础

---

## 九、AsyncMixin — 异步混入

### 9.1 核心功能

简化异步操作混入：

```cpp
// 混入异步加载能力
class UMyComponent : public UActorComponent, public IAsyncMixin
{
    virtual void OnLoadComplete() override;
};
```

### 9.2 Lyra 中的用途

- 异步加载资产
- 异步初始化

---

## 十、GameplayMessageRouter — 消息路由

### 10.1 核心功能

解耦的游戏内消息系统：

```cpp
// 发消息
MessageSystem->BroadcastMessage(MSGKEY("Health.Changed"), Msg);

// 收消息
MessageSystem->RegisterListener<FHealthMsg>(MSGKEY("Health.Changed"), 
    this, &ThisClass::OnHealthChanged);
```

### 10.2 Lyra 中的用途

- UI 更新
- 组件通信
- 事件广播

---

## 十一、GameSettings — 游戏设置

### 11.1 核心功能

游戏设置的框架：

| 类 | 作用 |
|----|------|
| `UGameSetting` | 设置项基类 |
| `UGameSettingRegistry` | 设置注册表 |
| `UGameSettingsList` | 设置列表 |

### 11.2 Lyra 中的用途

- 画质设置
- 音量设置
- 按键绑定

---

## 十二、PocketWorlds — 口袋世界

### 12.1 核心功能

在主世界中嵌入**小型独立世界**：

- 预览场景
- 加载画面
- 彩蛋空间

### 12.2 Lyra 中的用途

- 角色选择界面背景
- 加载画面场景

---

## 十三、Water — 水体系统

### 13.1 核心功能

简单的水体模拟：

- 水面渲染
- 波浪效果
- 浮力支持

### 13.2 Lyra 中的用途

- 地图中的水域
- 游泳区域

---

## 十四、快速索引

| 插件 | 一句话总结 |
|------|-----------|
| CommonUser | 用户身份 |
| CommonConversation | 对话聊天 |
| UIExtension | UI 动态注入 |
| SmartObjects | 智能交互对象 |
| GameplayInteractions | 交互框架 |
| GameplayStateTree | 状态树 |
| AnimationWarping | 动画扭曲 |
| AnimationLocomotionLibrary | 移动动画库 |
| AsyncMixin | 异步混入 |
| GameplayMessageRouter | 消息路由 |
| GameSettings | 游戏设置 |
| PocketWorlds | 口袋世界 |
| Water | 水体 |

---

## 十五、下一步

- [01_GameplayAbilities_GAS](./01_GameplayAbilities_GAS技能系统.md) — GAS 技能系统
- [02_CommonUI与UMG](./02_CommonUI与UMG.md) — UI 框架
- [03_ModularGameplay组件化](./03_ModularGameplay组件化.md) — 角色组件化
- [00_插件体系总览](./00_插件体系总览.md) — 回到总览
