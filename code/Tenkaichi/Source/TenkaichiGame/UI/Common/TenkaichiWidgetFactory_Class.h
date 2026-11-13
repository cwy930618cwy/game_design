// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "TenkaichiWidgetFactory.h"
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"

#include "TenkaichiWidgetFactory_Class.generated.h"

#define UE_API TENKAICHIGAME_API

class UObject;
class UUserWidget;

// 控件工厂实现：按"数据类 → 控件类"映射表找。
UCLASS(MinimalAPI)
class UTenkaichiWidgetFactory_Class : public UTenkaichiWidgetFactory
{
	GENERATED_BODY()

public:
	// 构造函数：空实现。
	UTenkaichiWidgetFactory_Class() { }

	// 找控件类（沿继承链往上查映射表）。
	UE_API virtual TSubclassOf<UUserWidget> FindWidgetClassForData_Implementation(const UObject* Data) const override;
	
protected:
	// 数据类 → 控件类 的映射表。
	UPROPERTY(EditAnywhere, Category = ListEntries, meta = (AllowAbstract))
	TMap<TSoftClassPtr<UObject>, TSubclassOf<UUserWidget>> EntryWidgetForClass;
};

#undef UE_API