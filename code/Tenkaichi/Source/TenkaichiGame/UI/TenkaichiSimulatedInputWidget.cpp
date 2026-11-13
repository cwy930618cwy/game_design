// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiSimulatedInputWidget.h"
#include "EnhancedInputSubsystems.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "TenkaichiLogChannels.h"
#include "InputKeyEventArgs.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiSimulatedInputWidget)

#define LOCTEXT_NAMESPACE "TenkaichiSimulatedInputWidget"

// 构造函数：开启"消费指针输入"。
UTenkaichiSimulatedInputWidget::UTenkaichiSimulatedInputWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetConsumePointerInput(true);
}

#if WITH_EDITOR
// 编辑器调色板分类。
const FText UTenkaichiSimulatedInputWidget::GetPaletteCategory()
{
	return LOCTEXT("PalleteCategory", "Input");
}
#endif // WITH_EDITOR

// 控件构造完成（查初始键 + 监听控制映射变化）。
void UTenkaichiSimulatedInputWidget::NativeConstruct()
{
	// 找初始键，然后监听控制映射的任何变化
	QueryKeyToSimulate();

	if (UEnhancedInputLocalPlayerSubsystem* System = GetEnhancedInputSubsystem())
	{
		System->ControlMappingsRebuiltDelegate.AddUniqueDynamic(this, &UTenkaichiSimulatedInputWidget::OnControlMappingsRebuilt);
	}
	
	Super::NativeConstruct();
}

// 控件销毁（解绑控制映射变化）。
void UTenkaichiSimulatedInputWidget::NativeDestruct()
{
	if (UEnhancedInputLocalPlayerSubsystem* System = GetEnhancedInputSubsystem())
	{
		System->ControlMappingsRebuiltDelegate.RemoveAll(this);
	}

	Super::NativeDestruct();
}

// 触摸结束（清空模拟输入）。
FReply UTenkaichiSimulatedInputWidget::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	FlushSimulatedInput();
	
	return Super::NativeOnTouchEnded(InGeometry, InGestureEvent);
}

// 基于本控件的拥有本地玩家取增强输入子系统。
UEnhancedInputLocalPlayerSubsystem* UTenkaichiSimulatedInputWidget::GetEnhancedInputSubsystem() const
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ULocalPlayer* LP = GetOwningLocalPlayer())
		{
			return LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		}
	}
	return nullptr;
}

// 从当前输入子系统取玩家输入。
UEnhancedPlayerInput* UTenkaichiSimulatedInputWidget::GetPlayerInput() const
{
	if (UEnhancedInputLocalPlayerSubsystem* System = GetEnhancedInputSubsystem())
	{
		return System->GetPlayerInput();
	}
	return nullptr;
}

// 把给定的向量作为输入注入到当前模拟键。
void UTenkaichiSimulatedInputWidget::InputKeyValue(const FVector& Value)
{
	const APlayerController* PC = GetOwningPlayer();
	const FPlatformUserId UserId = PC ? PC->GetPlatformUserId() : PLATFORMUSERID_NONE;
	// 如果有配关联输入动作，就用它
	if (AssociatedAction)
	{
		if (UEnhancedInputLocalPlayerSubsystem* System = GetEnhancedInputSubsystem())
		{
			// 我们不想给这个动作应用任何修饰器或触发器，但函数签名要求传
			TArray<UInputModifier*> Modifiers;
			TArray<UInputTrigger*> Triggers;
			System->InjectInputVectorForAction(AssociatedAction, Value, Modifiers, Triggers);
		}
	}
	// 没有关联动作，就尝试在备用键上模拟输入
	else if (UEnhancedPlayerInput* Input = GetPlayerInput())
	{
		const FInputDeviceId DeviceToSimulate = IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(UserId);
		if(KeyToSimulate.IsValid())
		{
			const float DeltaTime = GetWorld()->GetDeltaSeconds();
			auto SimulateKeyPress = [Input, DeltaTime, DeviceToSimulate](const FKey& KeyToSim, const float Value, const EInputEvent Event)
			{
				FInputKeyEventArgs Args = FInputKeyEventArgs::CreateSimulated(
					KeyToSim,
					Event,
					Value,
					KeyToSim.IsAnalog() ? 1 : 0,
					DeviceToSimulate);

				Args.DeltaTime = DeltaTime;
			
				Input->InputKey(Args);
			};
			
			// 对于"键对"的根键（比如 Mouse2D 由 MouseX 和 MouseY 组成），
			// 我们应该对键对里的每个键调 InputKey，而不是对配对键本身。
			// 这样事件才能在 UPlayerInput 的键状态表里正确累积。
			// 来自消息处理器和视口客户端的所有输入事件都是这样工作的，
			// 所以我们模拟键输入时也应该这样做。
			if (const EKeys::FPairedKeyDetails* PairDetails = EKeys::GetPairedKeyDetails(KeyToSimulate))
			{
				SimulateKeyPress(PairDetails->XKeyDetails->GetKey(), Value.X, IE_Axis);
				SimulateKeyPress(PairDetails->YKeyDetails->GetKey(), Value.Y, IE_Axis);
			}
			else
			{
				SimulateKeyPress(KeyToSimulate, Value.X, IE_Pressed);
			}
		}
	}
	else
	{
		UE_LOG(LogTenkaichi, Error, TEXT("'%s' is attempting to simulate input but has no player input!"), *GetNameSafe(this));
	}
}

// 把给定的二维向量作为输入注入到当前模拟键。
void UTenkaichiSimulatedInputWidget::InputKeyValue2D(const FVector2D& Value)
{
	InputKeyValue(FVector(Value.X, Value.Y, 0.0));
}

// 清空所有模拟输入（松开所有键）。
void UTenkaichiSimulatedInputWidget::FlushSimulatedInput()
{
	if (UEnhancedPlayerInput* Input = GetPlayerInput())
	{
		Input->FlushPressedKeys();
	}
}

// 根据增强输入查询"关联动作绑了哪些键"来设置 KeyToSimulate。
void UTenkaichiSimulatedInputWidget::QueryKeyToSimulate()
{
	if (UEnhancedInputLocalPlayerSubsystem* System = GetEnhancedInputSubsystem())
	{
		TArray<FKey> Keys = System->QueryKeysMappedToAction(AssociatedAction);
		if(!Keys.IsEmpty() && Keys[0].IsValid())
		{
			KeyToSimulate = Keys[0];
		}
		else
		{
			KeyToSimulate = FallbackBindingKey;
		}
	}
}

// 控制映射变化时调用，让我们有机会适配自己的键。
void UTenkaichiSimulatedInputWidget::OnControlMappingsRebuilt()
{
	QueryKeyToSimulate();
}

#undef LOCTEXT_NAMESPACE