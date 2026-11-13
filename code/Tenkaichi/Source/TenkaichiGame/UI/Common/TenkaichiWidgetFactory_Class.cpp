// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiWidgetFactory_Class.h"

#include "Blueprint/UserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiWidgetFactory_Class)

// 找控件类（沿继承链往上查映射表）。
TSubclassOf<UUserWidget> UTenkaichiWidgetFactory_Class::FindWidgetClassForData_Implementation(const UObject* Data) const
{
	// 从当前类开始，一路往父类找，看有没有这个类的构造规则。
	for (UClass* Class = Data->GetClass(); Class; Class = Class->GetSuperClass())
	{
		TSoftClassPtr<UObject> ClassPtr(Class);
		if (const TSubclassOf<UUserWidget> EntryWidgetClassPtr = EntryWidgetForClass.FindRef(ClassPtr))
		{
			return EntryWidgetClassPtr;
		}
	}

	return TSubclassOf<UUserWidget>();
}