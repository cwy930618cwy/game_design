# 05 - Enhanced Input 与 Control Flows（输入与控制）

> 涉及插件：`EnhancedInput` + `ControlFlows` + `GameInput`
> Lyra 使用度：⭐⭐⭐ **核心**（所有玩家输入都走 Enhanced Input）

---

## 一、Enhanced Input 是什么？

**Enhanced Input** 是 UE5 的新一代输入系统，替代了旧的 `UInputComponent` 绑定方式。

### 旧系统的问题
- 绑定繁琐（每个按键手动绑定）
- 难以支持组合键
- 不支持输入缓冲
- 手柄/键盘切换麻烦

### Enhanced Input 的优势
- ✅ 数据驱动（Input Action 是资产）
- ✅ 支持输入映射上下文（Context）
- ✅ 内置修饰器（Modifier）和处理程序（Trigger）
- ✅ 跨平台统一

---

## 二、核心概念

### 2.1 Input Action（IA）
**一个输入动作**，如：
- `IA_Move` — 移动
- `IA_Look` — 视角
- `IA_Jump` — 跳跃
- `IA_Shoot` — 射击

每个 IA 是一个 `UInputAction` 资产，定义了：
- 值类型（Axis1D / Axis2D / Boolean）
- 修饰器（取反、死区、缩放）
- 触发器（按下、释放、长按、连点）

### 2.2 Input Mapping Context（IMC）
**输入映射上下文**，如：
- `IMC_Default` — 默认（WASD 移动、鼠标视角）
- `IMC_UI` — UI 模式（方向键导航、确认取消）
- `IMC_Vehicle` — 载具（油门、刹车、转向）

同一个 IA 在不同 Context 下可以映射到不同按键。

### 2.3 Input Component
**增强输入组件** `ULyraInputComponent`：

```cpp
// Lyra 的输入绑定
void ULyraInputComponent::BindNativeAction(const ULyraInputConfig* Config, 
    const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, 
    AActor* TargetActor, const TCHAR* FunctionName);
```

---

## 三、工作流程

### 3.1 输入处理流程

```
1. 玩家按下 WASD
2. Input System 查找当前激活的 IMC
3. IMC 里找到 IA_Move 映射到 WASD
4. 应用 Modifier（死区、取反等）
5. 应用 Trigger（持续按下 = TriggersPressed）
6. 调用绑定的函数（如 OnMove）
7. 传入 FInputActionValue（包含方向和力度）
```

### 3.2 与 GAS 集成

Lyra 把输入和 GAS 打通：

```cpp
// InputConfig 里配置 AbilityInputActions
UPROPERTY()
TArray<FLyraAbilityInputAction> AbilityInputActions;

// 每个 AbilityInputAction 包含：
// - InputTag（GameplayTag）
// - AbilityClass（对应的 GA）
// - InputPressedEvent / InputReleasedEvent
```

这样按攻击键 → 触发 InputTag → 激活对应 GA。

---

## 四、Control Flows 是什么？

**Control Flows** 是 Lyra 的一个抽象层，用于处理**控制流重定向**：

### 4.1 核心概念

| 类 | 作用 |
|----|------|
| `ULyraControllerFragment` | 控制器片段基类 |
| `ULyraRebirthComponent` | 重生组件 |
| `ALyraPlayerStart` | 玩家出生点 |

### 4.2 用途
- 玩家死亡后选择重生点
- 游戏开始时选择出生位置
- 传送玩家到指定位置

---

## 五、目录结构

```
EnhancedInput/
├── Source/
│   └── EnhancedInput/
│       ├── Public/
│       │   ├── InputAction.h           ← IA 资产
│       │   ├── InputMappingContext.h   ← IMC 资产
│       │   ├── InputTrigger.h          ← 触发器
│       │   ├── InputModifier.h         ← 修饰器
│       │   └── EnhancedInputComponent.h
│       └── Private/
└── EnhancedInput.uplugin
```

---

## 六、Lyra 中的实际例子

### 6.1 移动输入

```cpp
// Lyra 的移动处理
void ALyraCharacter::OnMove(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    
    // 根据相机朝向计算移动方向
    const FRotator Rotation = GetController()->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);
    
    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    
    AddMovementInput(ForwardDirection, MovementVector.Y);
    AddMovementInput(RightDirection, MovementVector.X);
}
```

### 6.2 技能输入

```cpp
// 输入配置映射到技能
// IA_Shoot → AbilityInputTag.Shoot → GA_RangedWeapon
// IA_Melee → AbilityInputTag.Melee → GA_Melee
```

---

## 七、学习建议

1. **先理解 IA 和 IMC** — 数据驱动的输入定义
2. **看 Lyra 的 InputConfig** — 理解输入到技能的映射
3. **跟踪一个完整流程** — 从按键到角色移动
4. **动手实践** — 为新模式添加自定义输入

## 八、下一步

- [01_GameplayAbilities_GAS](./01_GameplayAbilities_GAS技能系统.md) — GAS 技能系统
- [02_CommonUI与UMG](./02_CommonUI与UMG.md) — UI 框架
- [03_ModularGameplay组件化](./03_ModularGameplay组件化.md) — 角色组件化
- [00_插件体系总览](./00_插件体系总览.md) — 回到总览
