# `LyraAbilitySimpleFailureMessage.h` 速览

> 一个消息结构体 + 一个全局 Tag。只有 `.h`。

| 成员 | 干嘛的 |
|---|---|
| `TAG_ABILITY_SIMPLE_FAILURE_MESSAGE` | `Ability.UserFacingSimpleActivateFail.Message` |
| `FLyraAbilitySimpleFailureMessage` | 消息体：`PlayerController` + `FailureTags` + `UserFacingReason` |

> 💡 **这条消息是谁发的**：`ULyraGameplayAbility::NativeOnAbilityFailedToActivate` —— 技能激活失败时，从 `FailureTagToUserFacingMessages` 表里查到文字，组装成这条消息广播到 `UGameplayMessageSubsystem`。
>
> **谁在听**：HUD 上的提示（比如"法力不足"）。

**说明**：UI 不需要知道任何技能细节，只要监听这个 Tag 就能显示失败原因。这是 Lyra 解耦 UI 与玩法的典型手法。
