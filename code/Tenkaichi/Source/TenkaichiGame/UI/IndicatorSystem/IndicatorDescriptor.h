// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Components/SceneComponent.h"
#include "Types/SlateEnums.h"

#include "IndicatorDescriptor.generated.h"

#define UE_API TENKAICHIGAME_API

class SWidget;
class UIndicatorDescriptor;
class UTenkaichiIndicatorManagerComponent;
class UUserWidget;
struct FFrame;
struct FSceneViewProjectionData;

// 投影工具：把指示器的世界位置换算成屏幕位置（含深度）。
struct FIndicatorProjection
{
	// 执行投影：成功返回 true，结果写入 OutScreenPositionWithDepth（XY=屏幕位置，Z=深度）。
	bool Project(const UIndicatorDescriptor& IndicatorDescriptor, const FSceneViewProjectionData& InProjectionData, const FVector2f& ScreenSize, FVector& OutScreenPositionWithDepth);
};

// 指示器的投影模式（决定"用什么来算屏幕位置"）。
UENUM(BlueprintType)
enum class EActorCanvasProjectionMode : uint8
{
	ComponentPoint,             // 组件上的一个点
	ComponentBoundingBox,       // 组件的包围盒（算中心点）
	ComponentScreenBoundingBox, // 组件的"屏幕空间包围盒"（算包围盒的屏幕矩形）
	ActorBoundingBox,           // 整个 Actor 的包围盒（算中心点）
	ActorScreenBoundingBox      // 整个 Actor 的"屏幕空间包围盒"
};

/**
 * 描述并控制一条活跃的指示器。强烈建议你的控件实现
 * IIndicatorWidgetInterface，这样它才能"绑定"到关联数据上。
 */
UCLASS(MinimalAPI, BlueprintType)
class UIndicatorDescriptor : public UObject
{
	GENERATED_BODY()
	
public:
	// 构造函数（空实现）。
	UIndicatorDescriptor() { }

public:
	// 取绑定的数据对象（任意业务数据）。
	UFUNCTION(BlueprintCallable)
	UObject* GetDataObject() const { return DataObject; }
	// 设置绑定的数据对象。
	UFUNCTION(BlueprintCallable)
	void SetDataObject(UObject* InDataObject) { DataObject = InDataObject; }
	
	// 取要追踪的场景组件（世界位置来源）。
	UFUNCTION(BlueprintCallable)
	USceneComponent* GetSceneComponent() const { return Component; }
	// 设置要追踪的场景组件。
	UFUNCTION(BlueprintCallable)
	void SetSceneComponent(USceneComponent* InComponent) { Component = InComponent; }

	// 取组件上的 socket 名（空 = 用组件原点）。
	UFUNCTION(BlueprintCallable)
	FName GetComponentSocketName() const { return ComponentSocketName; }
	// 设置组件上的 socket 名。
	UFUNCTION(BlueprintCallable)
	void SetComponentSocketName(FName SocketName) { ComponentSocketName = SocketName; }

	// 取指示器控件类（软引用）。
	UFUNCTION(BlueprintCallable)
	TSoftClassPtr<UUserWidget> GetIndicatorClass() const { return IndicatorWidgetClass; }
	// 设置指示器控件类。
	UFUNCTION(BlueprintCallable)
	void SetIndicatorClass(TSoftClassPtr<UUserWidget> InIndicatorWidgetClass)
	{
		IndicatorWidgetClass = InIndicatorWidgetClass;
	}

public:
	// TODO 这里需要更好的组织方式。
	// 已创建的指示器控件实例（弱引用，防悬空）。
	TWeakObjectPtr<UUserWidget> IndicatorWidget;

public:
	// 设置"组件为空时自动移除"。
	UFUNCTION(BlueprintCallable)
	void SetAutoRemoveWhenIndicatorComponentIsNull(bool CanAutomaticallyRemove)
	{
		bAutoRemoveWhenIndicatorComponentIsNull = CanAutomaticallyRemove;
	}
	// 取"组件为空时自动移除"。
	UFUNCTION(BlueprintCallable)
	bool GetAutoRemoveWhenIndicatorComponentIsNull() const { return bAutoRemoveWhenIndicatorComponentIsNull; }

	// 是否可以自动移除（开了开关 + 组件已失效）。
	bool CanAutomaticallyRemove() const
	{
		return bAutoRemoveWhenIndicatorComponentIsNull && !IsValid(GetSceneComponent());
	}

public:
	// 布局属性
	//=======================

	// 是否可见（组件有效 + 期望可见）。
	UFUNCTION(BlueprintCallable)
	bool GetIsVisible() const { return IsValid(GetSceneComponent()) && bVisible; }
	
	// 设置期望可见性。
	UFUNCTION(BlueprintCallable)
	void SetDesiredVisibility(bool InVisible)
	{
		bVisible = InVisible;
	}

	// 取投影模式。
	UFUNCTION(BlueprintCallable)
	EActorCanvasProjectionMode GetProjectionMode() const { return ProjectionMode; }
	// 设置投影模式。
	UFUNCTION(BlueprintCallable)
	void SetProjectionMode(EActorCanvasProjectionMode InProjectionMode)
	{
		ProjectionMode = InProjectionMode;
	}

	// 相对于空间点的水平对齐方式。
	UFUNCTION(BlueprintCallable)
	EHorizontalAlignment GetHAlign() const { return HAlignment; }
	// 设置水平对齐。
	UFUNCTION(BlueprintCallable)
	void SetHAlign(EHorizontalAlignment InHAlignment)
	{
		HAlignment = InHAlignment;
	}

	// 相对于空间点的垂直对齐方式。
	UFUNCTION(BlueprintCallable)
	EVerticalAlignment GetVAlign() const { return VAlignment; }
	// 设置垂直对齐。
	UFUNCTION(BlueprintCallable)
	void SetVAlign(EVerticalAlignment InVAlignment)
	{
		VAlignment = InVAlignment;
	}

	// 是否把指示器钳制在屏幕边缘？
	UFUNCTION(BlueprintCallable)
	bool GetClampToScreen() const { return bClampToScreen; }
	// 设置是否钳制到屏幕。
	UFUNCTION(BlueprintCallable)
	void SetClampToScreen(bool bValue)
	{
		bClampToScreen = bValue;
	}

	// 钳制到屏幕边缘时是否显示箭头？
	UFUNCTION(BlueprintCallable)
	bool GetShowClampToScreenArrow() const { return bShowClampToScreenArrow; }
	// 设置是否显示钳制箭头。
	UFUNCTION(BlueprintCallable)
	void SetShowClampToScreenArrow(bool bValue)
	{
		bShowClampToScreenArrow = bValue;
	}

	// 指示器的世界空间位置偏移。
	UFUNCTION(BlueprintCallable)
	FVector GetWorldPositionOffset() const { return WorldPositionOffset; }
	// 设置世界空间偏移。
	UFUNCTION(BlueprintCallable)
	void SetWorldPositionOffset(FVector Offset)
	{
		WorldPositionOffset = Offset;
	}

	// 指示器的屏幕空间位置偏移。
	UFUNCTION(BlueprintCallable)
	FVector2D GetScreenSpaceOffset() const { return ScreenSpaceOffset; }
	// 设置屏幕空间偏移。
	UFUNCTION(BlueprintCallable)
	void SetScreenSpaceOffset(FVector2D Offset)
	{
		ScreenSpaceOffset = Offset;
	}

	// 取包围盒锚点（0~1，指示器贴包围盒的哪个位置）。
	UFUNCTION(BlueprintCallable)
	FVector GetBoundingBoxAnchor() const { return BoundingBoxAnchor; }
	// 设置包围盒锚点。
	UFUNCTION(BlueprintCallable)
	void SetBoundingBoxAnchor(FVector InBoundingBoxAnchor)
	{
		BoundingBoxAnchor = InBoundingBoxAnchor;
	}

public:
	// 排序属性
	//=======================

	// 允许给指示器排序（在按深度排序之后），让某组指示器永远显示在其他组前面。
	UFUNCTION(BlueprintCallable)
	int32 GetPriority() const { return Priority; }
	// 设置排序优先级。
	UFUNCTION(BlueprintCallable)
	void SetPriority(int32 InPriority)
	{
		Priority = InPriority;
	}

public:
	// 取所属的指示器管理器（弱引用，防悬空）。
	UTenkaichiIndicatorManagerComponent* GetIndicatorManagerComponent() { return ManagerPtr.Get(); }
	// 设置所属的指示器管理器（只能设一次）。
	UE_API void SetIndicatorManagerComponent(UTenkaichiIndicatorManagerComponent* InManager);
	
	// 从管理器注销自己（移除指示器）。
	UFUNCTION(BlueprintCallable)
	UE_API void UnregisterIndicator();

private:
	// 是否可见。
	UPROPERTY()
	bool bVisible = true;
	// 是否钳制到屏幕边缘。
	UPROPERTY()
	bool bClampToScreen = false;
	// 钳制时是否显示箭头。
	UPROPERTY()
	bool bShowClampToScreenArrow = false;
	// 是否覆盖屏幕位置（当前未使用）。
	UPROPERTY()
	bool bOverrideScreenPosition = false;
	// 组件为空时是否自动移除。
	UPROPERTY()
	bool bAutoRemoveWhenIndicatorComponentIsNull = false;

	// 投影模式。
	UPROPERTY()
	EActorCanvasProjectionMode ProjectionMode = EActorCanvasProjectionMode::ComponentPoint;
	// 水平对齐（TEnumAsByte 兼容旧枚举存储）。
	UPROPERTY()
	TEnumAsByte<EHorizontalAlignment> HAlignment = HAlign_Center;
	// 垂直对齐。
	UPROPERTY()
	TEnumAsByte<EVerticalAlignment> VAlignment = VAlign_Center;

	// 排序优先级。
	UPROPERTY()
	int32 Priority = 0;

	// 包围盒锚点（默认中心 0.5,0.5,0.5）。
	UPROPERTY()
	FVector BoundingBoxAnchor = FVector(0.5, 0.5, 0.5);
	// 屏幕空间偏移。
	UPROPERTY()
	FVector2D ScreenSpaceOffset = FVector2D(0, 0);
	// 世界空间偏移。
	UPROPERTY()
	FVector WorldPositionOffset = FVector(0, 0, 0);

private:
	// 友元：允许 SActorCanvas 访问私有成员。
	friend class SActorCanvas;

	// 绑定的业务数据对象。
	UPROPERTY()
	TObjectPtr<UObject> DataObject;
	
	// 要追踪的场景组件。
	UPROPERTY()
	TObjectPtr<USceneComponent> Component;

	// 组件上的 socket 名。
	UPROPERTY()
	FName ComponentSocketName = NAME_None;

	// 指示器控件类（软引用）。
	UPROPERTY()
	TSoftClassPtr<UUserWidget> IndicatorWidgetClass;

	// 所属的指示器管理器（弱引用）。
	UPROPERTY()
	TWeakObjectPtr<UTenkaichiIndicatorManagerComponent> ManagerPtr;

	// 指示器控件实例（Slate）。
	TWeakPtr<SWidget> Content;
	// 承载指示器的 Canvas 宿主（Slate）。
	TWeakPtr<SWidget> CanvasHost;
};

#undef UE_API