// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Messaging/CommonMessagingSubsystem.h"
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"

#include "TenkaichiUIMessaging.generated.h"

class FSubsystemCollectionBase;
class UCommonGameDialog;
class UCommonGameDialogDescriptor;
class UObject;

/**
 * UI 消息子系统：实现确认框和错误框的弹出。
 */
UCLASS()
class UTenkaichiUIMessaging : public UCommonMessagingSubsystem
{
	GENERATED_BODY()

public:
	// 构造函数：空实现。
	UTenkaichiUIMessaging() { }

	// 初始化：同步加载对话框类。
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 显示确认框。
	virtual void ShowConfirmation(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback = FCommonMessagingResultDelegate()) override;
	// 显示错误框。
	virtual void ShowError(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback = FCommonMessagingResultDelegate()) override;

private:
	// 确认框类（运行时缓存，硬引用）。
	UPROPERTY()
	TSubclassOf<UCommonGameDialog> ConfirmationDialogClassPtr;

	// 错误框类（运行时缓存，硬引用）。
	UPROPERTY()
	TSubclassOf<UCommonGameDialog> ErrorDialogClassPtr;

	// 确认框类（配置文件指定，软引用）。
	UPROPERTY(config)
	TSoftClassPtr<UCommonGameDialog> ConfirmationDialogClass;

	// 错误框类（配置文件指定，软引用）。
	UPROPERTY(config)
	TSoftClassPtr<UCommonGameDialog> ErrorDialogClass;
};