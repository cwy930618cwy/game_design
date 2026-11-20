// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonTabListWidgetBase.h"

#include "TenkaichiTabListWidgetBase.generated.h"

#define UE_API TENKAICHIGAME_API

class UCommonButtonBase;
class UCommonUserWidget;
class UObject;
class UWidget;
struct FFrame;

// 一个标签的描述：ID / 标题 / 图标 / 是否隐藏 / 按钮类 / 内容类 / 已创建的内容控件。
USTRUCT(BlueprintType)
struct FTenkaichiTabDescriptor
{
	GENERATED_BODY()

public:
	// 构造函数：默认不隐藏、内容控件为空。
	FTenkaichiTabDescriptor()
	: bHidden(false)
	, CreatedTabContentWidget(nullptr)
	{ }

	// 标签 ID（唯一标识）。
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName TabId;

	// 标签标题文字。
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TabText;

	// 标签图标画刷。
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateBrush IconBrush;

	// 是否隐藏这个标签。
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHidden;

	// 标签按钮类。
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UCommonButtonBase> TabButtonType;

	//TODO NDarnell - 这个应该改成 TSoftClassPtr<>，底层通用标签列表要能处理懒加载的标签内容构造。
	// 标签内容控件类。
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UCommonUserWidget> TabContentType;

	// 已创建的标签内容控件（运行时生成）。
	UPROPERTY(Transient)
	TObjectPtr<UWidget> CreatedTabContentWidget;
};

// 标签按钮接口：让标签按钮接收自己的标签描述。
UINTERFACE(BlueprintType)
class UTenkaichiTabButtonInterface : public UInterface
{
	GENERATED_BODY()
};

// 标签按钮接口的 C++ 侧（实现它就能接收标签描述）。
class ITenkaichiTabButtonInterface
{
	GENERATED_BODY()

public:
	// 设置标签信息（蓝图原生事件）。
	UFUNCTION(BlueprintNativeEvent, Category = "Tab Button")
	void SetTabLabelInfo(const FTenkaichiTabDescriptor& TabDescriptor);
};

// 标签列表控件：带预注册标签信息。
UCLASS(MinimalAPI, Blueprintable, BlueprintType, Abstract, meta = (DisableNativeTick))
class UTenkaichiTabListWidgetBase : public UCommonTabListWidgetBase
{
	GENERATED_BODY()
	
public:
	// 获取某个预注册标签的信息。
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tab List")
	UE_API bool GetPreregisteredTabInfo(const FName TabNameId, FTenkaichiTabDescriptor& OutTabInfo);

	/** 获取所有预注册标签信息的辅助方法 */
	const TArray<FTenkaichiTabDescriptor>& GetAllPreregisteredTabInfos() { return PreregisteredTabInfoArray; }

	// 切换某个标签的隐藏状态（只能在 switcher 关联之前调）。
	UFUNCTION(BlueprintCallable, Category = "Tab List")
	UE_API void SetTabHiddenState(FName TabNameId, bool bHidden);

	// 运行时注册一个新标签。
	UFUNCTION(BlueprintCallable, Category = "Tab List")
	UE_API bool RegisterDynamicTab(const FTenkaichiTabDescriptor& TabDescriptor);

	// 第一个标签是否是激活状态。
	UFUNCTION(BlueprintCallable, Category = "Tab List")
	UE_API bool IsFirstTabActive() const;

	// 最后一个标签是否是激活状态。
	UFUNCTION(BlueprintCallable, Category = "Tab List")
	UE_API bool IsLastTabActive() const;

	// 某个标签是否可见。
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tab List")
	UE_API bool IsTabVisible(FName TabId);

	// 可见标签数量。
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tab List")
	UE_API int32 GetVisibleTabCount();

	/** 新标签创建时广播。允许创建后挂钩。 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabContentCreated, FName, TabId, UCommonUserWidget*, TabWidget);
	// C++ 原生版委托。
	DECLARE_EVENT_TwoParams(UTenkaichiTabListWidgetBase, FOnTabContentCreatedNative, FName /* TabId */, UCommonUserWidget* /* TabWidget */);

	/** 新标签创建时广播（蓝图可绑）。 */
	UPROPERTY(BlueprintAssignable, Category = "Tab List")
	FOnTabContentCreated OnTabContentCreated;
	// C++ 原生版事件。
	FOnTabContentCreatedNative OnTabContentCreatedNative;

protected:
	// UUserWidget 接口开始
	// 控件初始化。
	UE_API virtual void NativeOnInitialized() override;
	// 控件构造完成（建立标签）。
	UE_API virtual void NativeConstruct() override;
	// 控件销毁（移除标签内容）。
	UE_API virtual void NativeDestruct() override;
	// UUserWidget 接口结束

	// switcher 变化前的回调（移除旧内容）。
	UE_API virtual void HandlePreLinkedSwitcherChanged() override;
	// switcher 变化后的回调（重建标签）。
	UE_API virtual void HandlePostLinkedSwitcherChanged() override;

	// 标签创建完成的回调（填标签信息）。
	UE_API virtual void HandleTabCreation_Implementation(FName TabId, UCommonButtonBase* TabButton) override;

private:
	// 建立所有标签（创建内容 + 注册）。
	UE_API void SetupTabs();

	// 预注册的标签信息数组。
	UPROPERTY(EditAnywhere, meta=(TitleProperty="TabId"))
	TArray<FTenkaichiTabDescriptor> PreregisteredTabInfoArray;
	
	/**
	 * 存储运行时注册但还没创建的标签信息。
	 * 元素在创建后移除。
	 */
	UPROPERTY()
	TMap<FName, FTenkaichiTabDescriptor> PendingTabLabelInfoMap;
};

#undef UE_API