// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入基础类型层：uint8 / uint32 / int32 / TCHAR 都来自这里。
#include "CoreTypes.h"
// 引入访问检测：UE::AccessDetection::ReportAccess（检测 CVar 跨线程误用）。
#include "Misc/AccessDetection.h"
// 引入断言宏：check / ensure / ensureMsgf。
#include "Misc/AssertionMacros.h"
// 引入模板工具：TIdentity / TMemFunPtrType 等。
#include "Templates/UnrealTemplate.h"
// 引入 FString：字符串类型。
#include "Containers/UnrealString.h"
// 引入日志宏：DECLARE_LOG_CATEGORY_EXTERN。
#include "Logging/LogMacros.h"
// 引入委托句柄：FDelegateHandle。
#include "Delegates/IDelegateInstance.h"
// 引入委托宏：DECLARE_DELEGATE / DECLARE_MULTICAST_DELEGATE。
#include "Delegates/Delegate.h"
// 引入模块化特性基类：IModularFeature。
#include "Features/IModularFeature.h"
// 引入 EnableIf：模板条件编译。
#include "Templates/EnableIf.h"
// 引入 FText：本地化文本。
#include "Internationalization/Text.h"

// TRACK_CONSOLE_FIND_COUNT：是否统计 FindConsoleObject 调用次数（Shipping/Test 下关闭）。
#define TRACK_CONSOLE_FIND_COUNT !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

// 如果开启了 check 且不是 Shipping 构建（即使 Shipping 开了 check 也禁用）。
#if DO_CHECK && (!UE_BUILD_SHIPPING) // Disable even if checks in shipping are enabled.
// cvarCheckCode：包装 checkCode，只在开发构建里生效。
	#define cvarCheckCode( Code )		checkCode( Code )
// 否则。
#else
// cvarCheckCode：空宏，Shipping 下完全编译掉。
	#define cvarCheckCode(...)
// 条件编译结束。
#endif

// 前置声明 TConsoleVariableData 模板类。
template <class T> class TConsoleVariableData;

// 控制台变量使用指南：变量应在初始化早期创建，但不能太早（不要在全局变量构造期）。选对变量类型；需要更多功能时考虑用控制台命令（见 Exec()）。可用类型：bool、int、float、bool&、int&、float&、string。一定要写好帮助文本，别人读帮助就能理解这个控制台变量的作用。帮助长度应限制在合理宽度内，以便在低分辨率屏幕上也能正常显示。在游戏控制台里的用法：<COMMAND> ? 打印帮助；<COMMAND> 打印当前状态；<COMMAND> x 设置并打印新状态。所有变量都支持自动补全。旧的 Exec() 系统可以用来访问控制台变量。控制台变量只在主线程使用。控制台变量的状态不做网络同步，也不序列化（加载/保存）。

// 位掩码 0x1、0x2、0x4 ……
// EConsoleVariableFlags：控制台变量标志位（位掩码 0x1 / 0x2 / 0x4 …）。
enum EConsoleVariableFlags
// 枚举体开始。
{
	/* 标志位掩码。用它代替 ~ECVF_SetByMask */
// ECVF_FlagMask：标志位掩码（低 16 位）。
	ECVF_FlagMask = 0x0000ffff,

// 默认，无标志，值由构造函数设置
// ECVF_Default：默认，无标志，值由构造函数设置。
	ECVF_Default = 0x0,
// 带此标志的控制台变量在正式发布版里行为不同：它们在控制台里被隐藏，用户不能修改。
// ECVF_Cheat：作弊标志，正式发布版里隐藏且用户不能改。
	ECVF_Cheat = 0x1,
// 用户不能从控制台修改这些控制台变量。从 C++ 或 ini 修改仍然可以。
// ECVF_ReadOnly：用户不能从控制台改（C++ 或 ini 仍可改）。
	ECVF_ReadOnly = 0x4,
// 已对这个对象调用过 UnregisterConsoleObject()。如果以相同类型重新注册，这个对象会被重新激活。这对 DLL 卸载很有用。
// ECVF_Unregistered：已调用 UnregisterConsoleObject；同名同类型重新注册会复活（利于 DLL 卸载）。
	ECVF_Unregistered = 0x8,
// 这个标志由 ini 加载代码在变量还没注册时设置。之后变量注册时，值会被拷过去，这个变量被销毁。
// ECVF_CreatedFromIni：ini 加载时变量还没注册，先建个占位；注册后值拷过去并销毁占位。
	ECVF_CreatedFromIni = 0x10,
// 维护另一份影子副本，用渲染线程命令更新副本以保持正确顺序。可以扩展到更多/其他线程。注意：对控制台变量引用，它假定引用只在渲染线程访问（不要在其他线程用，最好干脆别用引用，避免潜在陷阱）。
// ECVF_RenderThreadSafe：维护一份影子副本，用渲染线程命令更新以保证顺序。
	ECVF_RenderThreadSafe = 0x20,

// ApplyCVarSettingsGroupFromIni 会检查这个标志是否设置；不应与 ECVF_Cheat 和 ECVF_ExcludeFromPreview 组合。它们会自动被加上 ECVF_Preview，除非用了 ECVF_ExcludeFromPreview。
// ECVF_Scalability：可伸缩性设置；ApplyCVarSettingsGroupFromIni 要求设置它。
	ECVF_Scalability = 0x40,

	/* 这些 cvar 控制其他带 ECVF_Scalability 标志的 cvar，名字应以 sg. 开头 */
// ECVF_ScalabilityGroup：控制其他 ECVF_Scalability 变量的变量，名字以 sg. 开头。
	ECVF_ScalabilityGroup = 0x80,

	/* 带此标志的 CVar 会包含在设备配置档预览里。 */
// ECVF_Preview：会包含在设备配置档预览里。
	ECVF_Preview = 0x100,

	/* 带此标志的 CVar 会修改所有平台的 Shader Keystring */
// ECVF_GeneralShaderChange：会修改所有平台的 Shader Keystring。
	ECVF_GeneralShaderChange = 0x200,

	/* 带此标志的 CVar 会修改移动平台的 Shader Keystring */
// ECVF_MobileShaderChange：会修改移动平台的 Shader Keystring。
	ECVF_MobileShaderChange = 0x400,

	/* 带此标志的 CVar 会修改桌面平台的 Shader Keystring */
// ECVF_DesktopShaderChange：会修改桌面平台的 Shader Keystring。
	ECVF_DesktopShaderChange = 0x800,

	/* 带此标志的 CVar 会从设备配置档预览里排除。 */
// ECVF_ExcludeFromPreview：从设备配置档预览里排除。
	ECVF_ExcludeFromPreview = 0x1000,

// 带此标志的 CVar 在热修后会存到本地临时文件，供下次启动读取。通常用于过早的功能开关，甚至早于发起热修 HTTP 请求。使用时请牢记以下约束：警告：这会把配置暴露给最终用户，用户可以在正式客户端里通过本地临时文件修改它，你有责任确保客户端不能通过这类 CVar「合法」作弊；首次热修时，值会在启动时从默认值变成热修后的值，所以如果逻辑不支持运行时改变，请确保在启动时缓存它；发布新游戏版本时，上一版本的本地临时文件可能还在，所以它仍会被读一次，请确保逻辑能兼容旧值，或者如果旧值不再支持就移除该 CVar；只对游戏客户端有效，对动态分配到不同服务器实例的专用服务器无效。
// ECVF_SaveForNextBoot：热修后存到本地临时文件，下次启动读取（仅游戏客户端）。
	ECVF_SaveForNextBoot = 0x2000,

	// ------------------------------------------------

	/* 设置类标志 */
// ECVF_SetFlagMask：设置类标志的掩码（第 16~23 位）。
	ECVF_SetFlagMask =				0x00ff0000,

	// 用于设置 cvar 但不调用所有 cvar sink。快得多，但可能不安全。只在你确定该 cvar/设置不需要 sink 调用时使用
// ECVF_Set_NoSinkCall_Unsafe：设置时不调用 cvar sink，快但可能不安全。
	ECVF_Set_NoSinkCall_Unsafe =	0x00010000,
	
	// 类似 ECVF_Set_NoSinkCall_Unsafe，但只设置 cvar 并把更新推给渲染线程，不改标志、不调 sink 等
// ECVF_Set_SetOnly_Unsafe：只设置值并推给渲染线程，不改标志、不调 sink。
	ECVF_Set_SetOnly_Unsafe = 		0x00020000,

	// 设置后，会原地覆盖已有的 tagged 数组类型值，而不是追加到数组末尾
// ECVF_Set_ReplaceExistingTag：原地覆盖已有的 tagged 数组值，而不是追加到末尾。
	ECVF_Set_ReplaceExistingTag =	0x00040000,

	// ------------------------------------------------

	/* 用于获取上次值由谁设置的历史（有助于排查 cvar 为什么处于某个状态） */
// ECVF_SetByMask：设置来源的掩码（高 8 位）。
	ECVF_SetByMask =				0xff000000,

	// ECVF_SetBy 标志按覆盖顺序排序（弱到强），值不序列化。它只影响调用 Set() 时的覆盖行为

	// 最低优先级（控制台变量创建后的默认）
// ECVF_SetByConstructor：最低优先级（cvar 创建后的默认）。
	ECVF_SetByConstructor =			0x00000000,
	// 由 Scalability.ini 的可伸缩性组设置（优先级低于游戏设置，便于部分覆盖）。这是数组类型
// ECVF_SetByScalability：由 Scalability.ini 的可伸缩性组设置（数组类型）。
	ECVF_SetByScalability =			0x01000000,
	// 引擎级游戏用户设置的默认优先级，平台专属设置会覆盖它
// ECVF_SetByGameSetting：引擎级游戏用户设置的默认优先级。
	ECVF_SetByGameSetting =			0x02000000,
	// 由项目设置 UI 或 ini 文件的特定段设置（优先级高于游戏设置，便于为项目强制某些设置）
// ECVF_SetByProjectSetting：项目设置 UI 或 ini 特定段设置。
	ECVF_SetByProjectSetting =		0x03000000,
	// Engine.ini 的 [ConsoleVariables] 段以及 FSystemSettings 使用
// ECVF_SetBySystemSettingsIni：Engine.ini 的 [ConsoleVariables] 段和 FSystemSettings 用。
	ECVF_SetBySystemSettingsIni =	0x04000000,
	// 动态加载/卸载的插件使用，配合 History 概念在插件卸载时恢复 cvar。这是数组类型，意味着多个同 SetBy 的值会存在历史里。
// ECVF_SetByPluginLowPriority：动态加载/卸载插件用（数组类型）。
	ECVF_SetByPluginLowPriority =	0x05000000,
	// 使用 DeviceProfiles.ini 层级的每设备设置（如特定 iOS 设备，优先级高于每项目，用于设备专属设置）。这是数组类型
// ECVF_SetByDeviceProfile：DeviceProfiles.ini 层级里的每设备设置（数组类型）。
	ECVF_SetByDeviceProfile =		0x06000000,
	// 动态加载/卸载的插件使用，配合 History 概念在插件卸载时恢复 cvar。这是数组类型，意味着多个同 SetBy 的值会存在历史里。
// ECVF_SetByPluginHighPriority：动态加载/卸载插件用（数组类型）。
	ECVF_SetByPluginHighPriority =	0x07000000,
	// 用户可设的游戏覆盖值，用于需要覆盖设备专属设置的 GameUserSettings 字段
// ECVF_SetByGameOverride：用户可设的游戏覆盖值。
	ECVF_SetByGameOverride =		0x08000000,
	// 由本地 consolevariables.ini 设置，主要用于测试多个项目
// ECVF_SetByConsoleVariablesIni：本地 consolevariables.ini 设置。
	ECVF_SetByConsoleVariablesIni = 0x09000000,
	// 由热修设置。这是数组类型，意味着多个同 SetBy 的值会存在历史里
// ECVF_SetByHotfix：热修设置（数组类型）。
	ECVF_SetByHotfix =				0x0A000000,
	// 编辑器预览用。这是数组类型，意味着多个同 SetBy 的值会存在历史里
// ECVF_SetByPreview：编辑器预览用（数组类型）。
	ECVF_SetByPreview =				0x0B000000,
	// 部分命令行参数使用，其他用 Console 优先级
// ECVF_SetByCommandline：部分命令行参数用。
	ECVF_SetByCommandline =			0x0C000000,
	// 用于高优先级临时调试或操作模式
// ECVF_SetByCode：高优先级临时调试或操作模式用。
	ECVF_SetByCode =				0x0D000000,
	// 最高优先级，通过编辑器 UI 或游戏/编辑器交互式控制台使用
// ECVF_SetByConsole：最高优先级，编辑器 UI 或交互式控制台用。
	ECVF_SetByConsole =				0x0E000000,


	// ------------------------------------------------
// 枚举体结束。
};


// ENUMERATE_SET_BY：把 15 个 SetBy 优先级逐个套一遍 op(...)。
#define ENUMERATE_SET_BY(op) \
	op(Constructor) \
	op(Scalability) \
	op(GameSetting) \
	op(ProjectSetting) \
	op(SystemSettingsIni) \
	op(PluginLowPriority) \
	op(DeviceProfile) \
	op(PluginHighPriority) \
	op(GameOverride) \
	op(ConsoleVariablesIni) \
	op(Hotfix) \
	op(Preview) \
	op(Commandline) \
	op(Code) \
	op(Console)


// 说明：把 ECVF_SetByMask 位转成人类可读字符串。
/** Returns human readable ECVF_SetByMask bits of the console variable flags. */
// GetConsoleVariableSetByName：标志位转字符串。
extern CORE_API const TCHAR* GetConsoleVariableSetByName(EConsoleVariableFlags ConsoleVariableFlags);

// 说明：GetConsoleVariableSetByName 的逆运算。
/** Inverse of GetConsoleVariableSetByName() */
// GetConsoleVariableSetByValue：字符串转标志位。
extern CORE_API EConsoleVariableFlags GetConsoleVariableSetByValue(const TCHAR* SetByName);

// 命名空间 ConsoleVariablePlatformMapping：ShaderPlatform / Platform / DeviceProfile 的映射。
namespace ConsoleVariablePlatformMapping
{
	// 存 EShaderPlatform（整数）到「正在为该 ShaderPlatform cook 的 Platform」的映射
// RegisterShaderPlatformToPlatformMapping：注册 ShaderPlatform → Platform 映射。
	CORE_API void RegisterShaderPlatformToPlatformMapping(int ShaderPlatform, FName PlatformName);
	// 存 Platform 到「为该 Platform cook 时用的 DeviceProfile」的映射
// RegisterPlatformToDeviceProfileMapping：注册 Platform → DeviceProfile 映射。
	CORE_API void RegisterPlatformToDeviceProfileMapping(FName PlatformName, FName DeviceProfileName);

	// 取之前用 RegisterShaderPlatformToPlatformMapping 设置的映射
	// 没有映射时返回 NAME_None
// GetShaderPlatformToPlatformMapping：取映射。
	CORE_API FName GetShaderPlatformToPlatformMapping(int ShaderPlatform);
	// 取之前用 RegisterPlatformToDeviceProfileMapping 设置的映射
	// 没有映射时返回 NAME_None
// GetPlatformToDeviceProfileMapping：取映射。
	CORE_API FName GetPlatformToDeviceProfileMapping(FName PlatformName);
// 命名空间结束。
}

// 前置声明 IConsoleVariable。
class IConsoleVariable;

// 如果没定义 NO_CVARS（即启用了控制台变量）。
#if !NO_CVARS

// 说明：控制台变量委托类型，void 回调，参数是 IConsoleVariable*。
/** Console variable delegate type  This is a void callback function. */
// FConsoleVariableDelegate：单播委托（1 个参数）。
DECLARE_DELEGATE_OneParam( FConsoleVariableDelegate, IConsoleVariable* );

// 说明：控制台变量多播委托类型。
/** Console variable multicast delegate type. */
// FConsoleVariableMulticastDelegate：多播委托（1 个参数）。
DECLARE_MULTICAST_DELEGATE_OneParam(FConsoleVariableMulticastDelegate, IConsoleVariable*);

// 说明：带名字的控制台对象多播委托。
/** Console object with name multicast delegate type. */
// FConsoleObjectWithNameMulticastDelegate：多播委托（2 个参数）。
DECLARE_MULTICAST_DELEGATE_TwoParams(FConsoleObjectWithNameMulticastDelegate, const TCHAR*, IConsoleObject*);

// 说明：控制台命令委托类型（无参数）。
/** Console command delegate type (takes no arguments.)  This is a void callback function. */
// FConsoleCommandDelegate：无参委托。
DECLARE_DELEGATE( FConsoleCommandDelegate );

// 说明：控制台命令委托类型（带参数列表）。
/** Console command delegate type (with arguments.)  This is a void callback function that always takes a list of arguments. */
// FConsoleCommandWithArgsDelegate：1 个参数（字符串数组）。
DECLARE_DELEGATE_OneParam( FConsoleCommandWithArgsDelegate, const TArray< FString >& );

// 说明：带 World 参数的控制台命令委托。
/** Console command delegate type with a world argument. This is a void callback function that always takes a world. */
// FConsoleCommandWithWorldDelegate：1 个参数（UWorld*）。
DECLARE_DELEGATE_OneParam( FConsoleCommandWithWorldDelegate, UWorld* );

// 说明：带 World 和参数列表的控制台命令委托。
/** Console command delegate type (with a world and arguments.)  This is a void callback function that always takes a list of arguments and a world. */
// FConsoleCommandWithWorldAndArgsDelegate：2 个参数。
DECLARE_DELEGATE_TwoParams(FConsoleCommandWithWorldAndArgsDelegate, const TArray< FString >&, UWorld*);

// 说明：带参数列表和输出设备的控制台命令委托。
/** Console command delegate type (with arguments and output device.)  This is a void callback function that always takes a list of arguments and output device. */
// FConsoleCommandWithArgsAndOutputDeviceDelegate：2 个参数。
DECLARE_DELEGATE_TwoParams(FConsoleCommandWithArgsAndOutputDeviceDelegate, const TArray< FString >&, FOutputDevice&);

// 说明：带参数列表、World 和输出设备的控制台命令委托。
/** Console command delegate type (with a world arguments and output device.)  This is a void callback function that always takes a list of arguments, a world and output device. */
// FConsoleCommandWithWorldArgsAndOutputDeviceDelegate：3 个参数。
DECLARE_DELEGATE_ThreeParams(FConsoleCommandWithWorldArgsAndOutputDeviceDelegate, const TArray< FString >&, UWorld*, FOutputDevice&);

// 说明：带输出设备的控制台命令委托。
/** Console command delegate type with the output device passed through. */
// FConsoleCommandWithOutputDeviceDelegate：1 个参数（FOutputDevice&）。
DECLARE_DELEGATE_OneParam( FConsoleCommandWithOutputDeviceDelegate, FOutputDevice& );

// 否则（NO_CVARS 定义了，即禁用控制台变量）。
#else

// FNullConsoleVariableDelegate：空壳委托模板，所有 Create* 都返回空对象。
template <typename DerivedType, typename... ParamTypes>
// 结构体开始。
struct FNullConsoleVariableDelegate
{
// 静态：创建裸 C++ 指针的全局函数委托
// CreateStatic：返回空对象。
	template <typename... VarTypes>
	inline static DerivedType CreateStatic(typename TIdentity<void (*)(ParamTypes..., VarTypes...)>::Type, VarTypes...)
	{
// 直接返回空。
		return {};
	}

// CreateLambda：返回空对象。
	template<typename FunctorType, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateLambda(FunctorType&&, VarTypes...)
	{
// 直接返回空。
		return {};
	}

// CreateSPLambda：返回空对象。
	template<typename UserClass, typename FunctorType, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateSPLambda(UserClass*, FunctorType&&, VarTypes...)
	{
// 直接返回空。
		return {};
	}

// CreateWeakLambda：返回空对象。
	template<typename UserClass, typename FunctorType, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateWeakLambda(UserClass*, FunctorType&&, VarTypes...)
	{
// 直接返回空。
		return {};
	}

// CreateRaw（非 const 成员函数）：返回空对象。
	template <typename UserClass, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateRaw(UserClass*, typename TMemFunPtrType<false, UserClass, void (ParamTypes..., VarTypes...)>::Type, VarTypes...)
	{
// 直接返回空。
		return {};
	}
// CreateRaw（const 成员函数）：返回空对象。
	template <typename UserClass, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateRaw(UserClass*, typename TMemFunPtrType<true, UserClass, void (ParamTypes..., VarTypes...)>::Type, VarTypes...)
	{
// 直接返回空。
		return {};
	}

// CreateSP（TSharedRef，非 const）：返回空对象。
	template <typename UserClass, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateSP(const TSharedRef<UserClass>&, typename TMemFunPtrType<false, UserClass, void (ParamTypes..., VarTypes...)>::Type, VarTypes...)
	{
// 直接返回空。
		return {};
	}
// CreateSP（TSharedRef，const）：返回空对象。
	template <typename UserClass, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateSP(const TSharedRef<UserClass>&, typename TMemFunPtrType<true, UserClass, void (ParamTypes..., VarTypes...)>::Type, VarTypes...)
	{
// 直接返回空。
		return {};
	}

// CreateSP（裸指针，非 const）：返回空对象。
	template <typename UserClass, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateSP(UserClass*, typename TMemFunPtrType<false, UserClass, void (ParamTypes..., VarTypes...)>::Type, VarTypes...)
	{
// 直接返回空。
		return {};
	}
// CreateSP（裸指针，const）：返回空对象。
	template <typename UserClass, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateSP(UserClass*, typename TMemFunPtrType<true, UserClass, void (ParamTypes..., VarTypes...)>::Type, VarTypes...)
	{
// 直接返回空。
		return {};
	}
	
// CreateThreadSafeSP（TSharedRef，非 const）：返回空对象。
	template <typename UserClass, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateThreadSafeSP(const TSharedRef<UserClass, ESPMode::ThreadSafe>&, typename TMemFunPtrType<false, UserClass, void (ParamTypes..., VarTypes...)>::Type, VarTypes...)
	{
// 直接返回空。
		return {};
	}
// CreateThreadSafeSP（TSharedRef，const）：返回空对象。
	template <typename UserClass, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateThreadSafeSP(const TSharedRef<UserClass, ESPMode::ThreadSafe>&, typename TMemFunPtrType<true, UserClass, void (ParamTypes..., VarTypes...)>::Type, VarTypes...)
	{
// 直接返回空。
		return {};
	}

// CreateThreadSafeSP（裸指针，非 const）：返回空对象。
	template <typename UserClass, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateThreadSafeSP(UserClass*, typename TMemFunPtrType<false, UserClass, void (ParamTypes..., VarTypes...)>::Type, VarTypes...)
	{
// 直接返回空。
		return {};
	}
// CreateThreadSafeSP（裸指针，const）：返回空对象。
	template <typename UserClass, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateThreadSafeSP(UserClass*, typename TMemFunPtrType<true, UserClass, void (ParamTypes..., VarTypes...)>::Type, VarTypes...)
	{
// 直接返回空。
		return {};
	}

// CreateUFunction：返回空对象。
	template <typename UObjectTemplate, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateUFunction(UObjectTemplate*, const FName&, VarTypes...)
	{
// 直接返回空。
		return {};
	}

// CreateUObject（非 const）：返回空对象。
	template <typename UserClass, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateUObject(UserClass*, typename TMemFunPtrType<false, UserClass, void (ParamTypes..., VarTypes...)>::Type, VarTypes...)
	{
// 直接返回空。
		return {};
	}
// CreateUObject（const）：返回空对象。
	template <typename UserClass, typename... VarTypes>
// 参数被忽略。
	inline static DerivedType CreateUObject(UserClass*, typename TMemFunPtrType<true, UserClass, void (ParamTypes..., VarTypes...)>::Type, VarTypes...)
	{
// 直接返回空。
		return {};
	}

// GetHandle：返回空句柄。
	FDelegateHandle GetHandle() const
// 函数体开始。
	{
// 返回空句柄。
		return {};
// 函数体结束。
	}

// ExecuteIfBound：永远返回 false（不执行）。
	bool ExecuteIfBound(ParamTypes...)
// 函数体开始。
	{
// 返回 false。
		return false;
// 函数体结束。
	}
// 结构体结束。
};

// FConsoleVariableDelegate：空壳版（NO_CVARS 下）。
struct FConsoleVariableDelegate								: FNullConsoleVariableDelegate<FConsoleVariableDelegate, IConsoleVariable*> {};
// FConsoleCommandDelegate：空壳版。
struct FConsoleCommandDelegate								: FNullConsoleVariableDelegate<FConsoleCommandDelegate> {};
// FConsoleCommandWithArgsDelegate：空壳版。
struct FConsoleCommandWithArgsDelegate						: FNullConsoleVariableDelegate<FConsoleCommandWithArgsDelegate, const TArray<FString>&> {};
// FConsoleCommandWithWorldDelegate：空壳版。
struct FConsoleCommandWithWorldDelegate						: FNullConsoleVariableDelegate<FConsoleCommandWithWorldDelegate, UWorld*> {};
// FConsoleCommandWithWorldAndArgsDelegate：空壳版。
struct FConsoleCommandWithWorldAndArgsDelegate				: FNullConsoleVariableDelegate<FConsoleCommandWithWorldAndArgsDelegate, const TArray<FString>&, UWorld*> {};
// FConsoleCommandWithArgsAndOutputDeviceDelegate：空壳版。
struct FConsoleCommandWithArgsAndOutputDeviceDelegate		: FNullConsoleVariableDelegate<FConsoleCommandWithArgsAndOutputDeviceDelegate, const TArray<FString>&, FOutputDevice&> {};
// FConsoleCommandWithWorldArgsAndOutputDeviceDelegate：空壳版。
struct FConsoleCommandWithWorldArgsAndOutputDeviceDelegate	: FNullConsoleVariableDelegate<FConsoleCommandWithWorldArgsAndOutputDeviceDelegate, const TArray<FString>&, UWorld*, FOutputDevice&> {};
// FConsoleCommandWithOutputDeviceDelegate：空壳版。
struct FConsoleCommandWithOutputDeviceDelegate				: FNullConsoleVariableDelegate<FConsoleCommandWithOutputDeviceDelegate, FOutputDevice&> {};

// 条件编译结束（NO_CVARS）。
#endif

// 前置声明 TConsoleVariableData 模板类。
template <class T> class TConsoleVariableData;

// 控制台对象（变量和命令）的接口
// IConsoleObject：控制台对象基类。
class IConsoleObject
{

// 公有区。
public:

// 构造函数。
	IConsoleObject()
// 如果开启了 Find 调用计数。
#if TRACK_CONSOLE_FIND_COUNT
// 初始化列表：FindCallCount 置 0。
		: FindCallCount(0)
// 条件编译结束。
#endif
// 构造函数体（空）。
	{}

// 虚析构函数。
	virtual ~IConsoleObject() {}

// 返回永不为 0，可以多行（用换行符分隔）
// GetHelp：纯虚，取帮助文本。
	virtual const TCHAR* GetHelp() const = 0;
// 返回永不为 0，可以多行（用换行符分隔）
// SetHelp：纯虚，设置帮助文本。
	virtual void SetHelp(const TCHAR* Value) = 0;
// 取标志位的内部状态。
// GetFlags：纯虚，取标志位。
	virtual EConsoleVariableFlags GetFlags() const = 0;
// 把标志位内部状态设为指定值。
// SetFlags：纯虚，设置标志位。
	virtual void SetFlags(const EConsoleVariableFlags Value) = 0;
// 返回比 GetHelp 更详细的帮助（如控制台变量的当前值）
// GetDetailedHelp：虚函数，默认返回 GetHelp。
	virtual FText GetDetailedHelp() const
// 函数体开始。
	{
// 默认只返回标准帮助消息。
		// By default, just return the standard help message :
// 把 GetHelp 的字符串转成 FText。
		return FText::FromString(GetHelp());
// 函数体结束。
	}

	// 便捷方法 -------------------------------------

// 清除内部状态里的指定标志位。
// ClearFlags：清除标志位。
	void ClearFlags(const EConsoleVariableFlags Value)
// 函数体开始。
	{
// 取反后与运算，清掉指定位。
		uint32 New = (uint32)GetFlags() & ~(uint32)Value;
	
// 写回。
		SetFlags((EConsoleVariableFlags)New);
// 函数体结束。
	}
// 测试内部状态里是否有任一指定标志位被设置。
// TestFlags：测试标志位。
	bool TestFlags(const EConsoleVariableFlags Value) const
// 函数体开始。
	{
// 与运算后非 0 即为真。
		return ((uint32)GetFlags() & (uint32)Value) != 0;
// 函数体结束。
	}

// 按标志位和当前构建类型判断变量是否有效（如 ECVF_Cheat 变量在某些目标上禁用）
// IsEnabled：判断是否启用。
	bool IsEnabled() const
// 函数体开始。
	{
// 如果禁用了作弊 cvar。
#if DISABLE_CHEAT_CVARS
// 如果带 ECVF_Cheat 标志。
		if (TestFlags(ECVF_Cheat))
// 条件体开始。
		{
// 返回 false（禁用）。
			return false;
// 条件体结束。
		}
// 条件编译结束。
#endif // DISABLE_CHEAT_CVARS
// 如果带 ECVF_Unregistered 标志。
		if (TestFlags(ECVF_Unregistered))
// 条件体开始。
		{
// 返回 false（已注销）。
			return false;
// 条件体结束。
		}
// 返回 true（启用）。
		return true;
// 函数体结束。
	}


// 如果对象有父对象（例如主 cvar 拥有其他平台的 cvar），返回它
// GetParentObject：虚函数，默认返回空。
	virtual IConsoleObject* GetParentObject() const
// 函数体开始。
	{
// 返回空指针。
		return nullptr;
// 函数体结束。
	}
	
// 把本对象转成 IConsoleVariable，不是则返回 0
// AsVariable：虚函数，默认返回 0。
	virtual class IConsoleVariable* AsVariable()
// 函数体开始。
	{
// 返回 0。
		return 0; 
// 函数体结束。
	}

// IsVariableBool：默认 false。
	virtual bool IsVariableBool() const { return false; }
// IsVariableInt：默认 false。
	virtual bool IsVariableInt() const { return false; }
// IsVariableFloat：默认 false。
	virtual bool IsVariableFloat() const { return false; }
// IsVariableString：默认 false。
	virtual bool IsVariableString() const { return false; }

// AsVariableBool：默认断言失败并返回 0。
	virtual class TConsoleVariableData<bool>* AsVariableBool()
// 函数体开始。
	{
// 断言：试图访问不支持该类型的 cvar 数据。
		ensureMsgf(false, TEXT("Attempted to access variable data of a console variable type that doesn't support it.  For example FindTConsoleVariableData* on a FAutoConsoleVariableRef."));
// 返回 0。
		return 0;
// 函数体结束。
	}

// AsVariableInt：默认断言失败并返回 0。
	virtual class TConsoleVariableData<int32>* AsVariableInt()
// 函数体开始。
	{
// 断言：试图访问不支持该类型的 cvar 数据。
		ensureMsgf(false, TEXT("Attempted to access variable data of a console variable type that doesn't support it.  For example FindTConsoleVariableData* on a FAutoConsoleVariableRef."));
// 返回 0。
		return 0; 
// 函数体结束。
	}

// AsVariableFloat：默认断言失败并返回 0。
	virtual class TConsoleVariableData<float>* AsVariableFloat()
// 函数体开始。
	{
// 断言：试图访问不支持该类型的 cvar 数据。
		ensureMsgf(false, TEXT("Attempted to access variable data of a console variable type that doesn't support it.  For example FindTConsoleVariableData* on a FAutoConsoleVariableRef."));
// 返回 0。
		return 0; 
// 函数体结束。
	}

// AsVariableString：默认断言失败并返回 0。
	virtual class TConsoleVariableData<FString>* AsVariableString()
// 函数体开始。
	{
// 断言：试图访问不支持该类型的 cvar 数据。
		ensureMsgf(false, TEXT("Attempted to access variable data of a console variable type that doesn't support it.  For example FindTConsoleVariableData* on a FAutoConsoleVariableRef."));
// 返回 0。
		return 0;
// 函数体结束。
	}

// 把本对象转成 IConsoleCommand，先验证是否安全
// AsCommand：虚函数，默认返回 0。
	virtual struct IConsoleCommand* AsCommand()
// 函数体开始。
	{
// 返回 0。
		return 0; 
// 函数体结束。
	}

// 影子 ConsoleObject 是存在且可用的，但在遍历所有对象并对其操作时不应被迭代
// IsShadowObject：虚函数，默认 false。
	virtual bool IsShadowObject() const
// 函数体开始。
	{
// 返回 false。
		return false;
// 函数体结束。
	}

// 如果 IsShadowObject() 返回 true，本函数返回影子 cvar 所影子的「真实」变量。一般不需要，因为大多数函数（GetInt、Set 等）已经操作真实变量
// GetShadowedObject：虚函数，默认返回空。
	virtual IConsoleObject* GetShadowedObject() const
// 函数体开始。
	{
// 返回空指针。
		return nullptr;
// 函数体结束。
	}

// IsDeprecated：虚函数，默认 false。
	virtual bool IsDeprecated() const
// 函数体开始。
	{
// 返回 false。
		return false;
// 函数体结束。
	}

private: // -----------------------------------------

// 如果开启了 Find 调用计数。
#if TRACK_CONSOLE_FIND_COUNT
	// 不再是纯视觉用途；如果出问题可以改接口来追踪 FindConsoleObject/FindConsoleVariable 调用
	// to track down FindConsoleObject/FindConsoleVariable calls without static
// FindCallCount：Find 调用计数。
	uint32 FindCallCount;
// 条件编译结束。
#endif

// 只应由管理器调用，每个实例都要实现
// Release：纯虚，释放对象。
	virtual void Release() = 0;

// 友元：FConsoleManager 可访问私有成员。
	friend class FConsoleManager;
// 类结束。
};

// 控制台变量的接口
// IConsoleVariable：控制台变量接口，继承 IConsoleObject。
class IConsoleVariable : public IConsoleObject
{
// 公有区。
public:

// 从指定字符串设置内部值。参数 SetBy：ECVF_LastSetMask 里的任意值，如 ECVF_SetByScalability。参数 Tag：可选标签，随值一起设置，只在 UE_ALLOW_CVAR_HISTORY 开启且 SetBy 是数组类型时有用。
// Set：纯虚，从字符串设置值。
	virtual void Set(const TCHAR* InValue, EConsoleVariableFlags SetBy = ECVF_SetByCode, FName Tag = NAME_None) = 0;

// 取消某个 SetBy 优先级上的值（只在 UE_ALLOW_CVAR_HISTORY 开启时有用）。CVar 的值会根据剩余的历史层级重新计算。参数 SetBy：ECVF_LastSetMask 里的任意值。参数 Tag：用于从数组类型 SetBy 里移除某个设置的标签。
// Unset：纯虚，取消某个优先级的值。
	virtual void Unset(EConsoleVariableFlags SetBy, FName Tag = NAME_None) = 0;

// 取内部值为 bool，对 bool、int、float 都有效。
// GetBool：纯虚。
	virtual bool GetBool() const = 0;
// 取内部值为 int（不应用于字符串）。返回值不四舍五入（简单强转）。
// GetInt：纯虚。
	virtual int32 GetInt() const = 0;
	/** 取内部值为 float（对所有类型都有效）。 */
// GetFloat：纯虚。
	virtual float GetFloat() const = 0;
	/** 取内部值为字符串（对所有类型都有效）。 */
// GetString：纯虚。
	virtual FString GetString() const = 0;

	/** 模板代码用的通用版本 */
// GetValue（int32 版）：转调 GetInt。
	void GetValue(int32& OutIntValue) const
// 函数体开始。
	{
// 写入输出参数。
		OutIntValue = GetInt();
// 函数体结束。
	}
// GetValue（bool 版）：转调 GetBool。
	void GetValue(bool& OutBoolValue) const
// 函数体开始。
	{
// 写入输出参数。
		OutBoolValue = GetBool();
// 函数体结束。
	}
// GetValue（float 版）：转调 GetFloat。
	void GetValue(float& OutFloatValue) const
// 函数体开始。
	{
// 写入输出参数。
		OutFloatValue = GetFloat();
// 函数体结束。
	}
// GetValue（FString 版）：转调 GetString。
	void GetValue(FString& OutStringValue) const
// 函数体开始。
	{
// 写入输出参数。
		OutStringValue = GetString();
// 函数体结束。
	}

// 允许指定一个回调函数，在控制台变量值变化时调用。即使值和之前相同也会调用。永远在游戏线程。这可能危险（改用 RegisterConsoleVariableSink()）：在委托里设置其他控制台变量可能导致死循环；设置很多控制台变量可能浪费周期；调用可能发生在初始化期间的任何时刻。因为不能在构造期间指定，所以创建时不会调用你。SetOnChangedCallback() 调用本身也不会触发回调，这由调用方负责。
// SetOnChangedCallback：纯虚，设置变化回调。
	virtual void SetOnChangedCallback(const FConsoleVariableDelegate& Callback) = 0;

// OnChangedDelegate：纯虚，取多播委托。
	virtual FConsoleVariableMulticastDelegate& OnChangedDelegate() = 0;

// 取保存的默认值（如果创建时保存过）。
// 标记 GetDefaultValueVariable 已废弃（5.4），改用 GetDefaultValue()。
	UE_DEPRECATED(5.4, "Use GetDefaultValue() instead")
// GetDefaultValueVariable：虚函数，默认返回空。
	virtual IConsoleVariable* GetDefaultValueVariable()
// 函数体开始。
	{
// 返回空指针。
		return nullptr;
// 函数体结束。
	}

// 取这个 CVar 构造时的值
// GetDefaultValue：纯虚。
	virtual FString GetDefaultValue() = 0;

// 如果允许其他平台配置。
#if ALLOW_OTHER_PLATFORM_CONFIG

// 取一个匹配本 cvar 但包含指定平台值的 CVar 对象。如果该平台/DP 的 cvar 不存在，会触发大量 cvar 加载。注意：如果这导致纯虚编译错误，确保你的 IConsoleVariable 子类继承 FConsoleVariableExtendedData。
// GetPlatformValueVariable：纯虚。
	virtual TSharedPtr<IConsoleVariable> GetPlatformValueVariable(FName PlatformName, const FString& DeviceProfileName=FString()) = 0;

// 检查 CVar 是否有指定平台的缓存值
// HasPlatformValueVariable：纯虚。
	virtual bool HasPlatformValueVariable(FName PlatformName, const FString& DeviceProfileName=FString()) = 0;

// 只用于调试/遍历，清空所有其他平台的 cvar 对象，这会强制下次调用 GetPlatformValueVariable 时重新查找（可能用新的 ini 文件）
// ClearPlatformVariables：虚函数，默认空实现。
	virtual void ClearPlatformVariables(FName PlatformName=NAME_None)
// 函数体开始。
	{
// 函数体结束。
	}
	
// 条件编译结束（ALLOW_OTHER_PLATFORM_CONFIG）。
#endif

// 把历史打印到日志
// LogHistory：纯虚。
	virtual void LogHistory(FOutputDevice& Ar) = 0;
	
// 统计历史数据占用的内存
// GetHistorySize：纯虚。
	virtual SIZE_T GetHistorySize() = 0;

	// 便捷方法

	/** 从指定 bool 设置内部值。 */
// Set（bool 版）：转成字符串再调 Set。
	void Set(bool InValue, EConsoleVariableFlags SetBy = ECVF_SetByCode, FName Tag=NAME_None)
// 函数体开始。
	{
		// 注意：bool 这里必须用 1 和 0 而不是 true/false，因为底层可能是 int32 之类，
		// 最终会调用 TTypeFromString<T>::FromString，它不处理 true/false，
		// 但 1 和 0 对任何类型都有效。
		// 低效但没有公共代码路径
// 用 1 / 0 而不是 true / false（因为底层可能是 int32，最终会走 TTypeFromString）。
		Set(InValue ? TEXT("1") : TEXT("0"), SetBy, Tag);
// 函数体结束。
	}
	/** 从指定 int 设置内部值。 */
// Set（int32 版）：格式化成字符串再调 Set。
	void Set(int32 InValue, EConsoleVariableFlags SetBy = ECVF_SetByCode, FName Tag=NAME_None)
// 函数体开始。
	{
		// 低效但没有公共代码路径
// 用 %d 格式化。
		Set(*FString::Printf(TEXT("%d"), InValue), SetBy, Tag);
// 函数体结束。
	}
	/** 从指定 float 设置内部值。 */
// Set（float 版）：格式化成字符串再调 Set。
	void Set(float InValue, EConsoleVariableFlags SetBy = ECVF_SetByCode, FName Tag=NAME_None)
// 函数体开始。
	{
		// 低效但没有公共代码路径
// 用 %g 格式化。
		Set(*FString::Printf(TEXT("%g"), InValue), SetBy, Tag);
// 函数体结束。
	}

// SetWithCurrentPriority（bool 版）：保持当前优先级设置。
	void SetWithCurrentPriority(bool InValue, FName Tag=NAME_None)
// 函数体开始。
	{
// 取当前 SetBy 优先级。
		EConsoleVariableFlags CurFlags = (EConsoleVariableFlags)(GetFlags() & ECVF_SetByMask);
// 用该优先级设置。
		Set(InValue, CurFlags, Tag);
// 函数体结束。
	}
// SetWithCurrentPriority（int32 版）。
	void SetWithCurrentPriority(int32 InValue, FName Tag=NAME_None)
// 函数体开始。
	{
// 取当前 SetBy 优先级。
		EConsoleVariableFlags CurFlags = (EConsoleVariableFlags)(GetFlags() & ECVF_SetByMask);
// 用该优先级设置。
		Set(InValue, CurFlags, Tag);
// 函数体结束。
	}
// SetWithCurrentPriority（float 版）。
	void SetWithCurrentPriority(float InValue, FName Tag=NAME_None)
// 函数体开始。
	{
// 取当前 SetBy 优先级。
		EConsoleVariableFlags CurFlags = (EConsoleVariableFlags)(GetFlags() & ECVF_SetByMask);
// 用该优先级设置。
		Set(InValue, CurFlags, Tag);
// 函数体结束。
	}
// SetWithCurrentPriority（字符串版）。
	void SetWithCurrentPriority(const TCHAR* InValue, FName Tag=NAME_None)
// 函数体开始。
	{
// 取当前 SetBy 优先级。
		EConsoleVariableFlags CurFlags = (EConsoleVariableFlags)(GetFlags() & ECVF_SetByMask);
// 用该优先级设置。
		Set(InValue, CurFlags, Tag);
// 函数体结束。
	}


};

// 控制台命令的接口
// IConsoleCommand：控制台命令接口，继承 IConsoleObject。
struct IConsoleCommand : public IConsoleObject
{
// 执行本命令（可选带参数）。参数 Args：本命令的参数列表。参数 InWorld：本命令的 World 上下文。返回：本命令的委托执行成功返回 true。
// Execute：纯虚。
	virtual bool Execute( const TArray< FString >& Args, UWorld* InWorld, class FOutputDevice& OutputDevice ) = 0;
};

// 把控制台变量变化传播到其他线程的接口
// IConsoleThreadPropagation：线程传播接口。
struct IConsoleThreadPropagation
{
// OnCVarChange（int32 版）：纯虚。
	virtual void OnCVarChange(int32& Dest, int32 NewValue) = 0;
// OnCVarChange（float 版）：纯虚。
	virtual void OnCVarChange(float& Dest, float NewValue) = 0;
// OnCVarChange（bool 版）：纯虚。
	virtual void OnCVarChange(bool& Dest, bool NewValue) = 0;
// OnCVarChange（FString 版）：纯虚。
	virtual void OnCVarChange(FString& Dest, const FString& NewValue) = 0;
// OnCVarChange（FName 版）：纯虚。
	virtual void OnCVarChange(FName& Dest, const FName& NewValue) = 0;
};

// 声明一个委托类型，控制台管理器用它为每个已知控制台对象回调用户函数。第一个参数是当前控制台对象的名字，第二个参数是当前控制台对象。
// FConsoleObjectVisitor：2 个参数（名字 + 对象）。
DECLARE_DELEGATE_TwoParams( FConsoleObjectVisitor, const TCHAR*, IConsoleObject* );


// 表示一个在线委托句柄的类。
// FConsoleVariableSinkHandle：cvar sink 句柄。
class FConsoleVariableSinkHandle
{
// 公有区。
public:
// 默认构造函数。
	FConsoleVariableSinkHandle()
// 函数体开始。
	{
// 函数体结束。
	}

// 显式构造函数：从 FDelegateHandle 构造。
	explicit FConsoleVariableSinkHandle(FDelegateHandle InHandle)
// 初始化列表：保存句柄。
		: Handle(InHandle)
// 函数体开始。
	{
// 函数体结束。
	}

// RemoveFromDelegate：从多播委托里移除本句柄。
	template <typename MulticastDelegateType>
// 函数体开始。
	void RemoveFromDelegate(MulticastDelegateType& MulticastDelegate)
	{
// 调多播委托的 Remove。
		MulticastDelegate.Remove(Handle);
// 函数体结束。
	}

// HasSameHandle：判断委托的句柄是否与本句柄相同。
	template <typename DelegateType>
// 函数体开始。
	bool HasSameHandle(const DelegateType& Delegate) const
	{
// 比较句柄。
		return Delegate.GetHandle() == Handle;
// 函数体结束。
	}

// 私有区。
private:
// Handle：底层委托句柄。
	FDelegateHandle Handle;
// 类结束。
};

// FConsoleSuggestion：控制台自动补全建议。
struct FConsoleSuggestion
// 结构体开始。
{
// Name：建议的名字。
	FString Name;
// Help：建议的帮助文本。
	FString Help;

// 构造函数：带名字和帮助。
	FConsoleSuggestion(FString InName, FString InHelp)
// 初始化列表：保存名字。
	: Name(InName)
// 初始化列表：保存帮助。
	, Help(InHelp)
// 构造函数体（空）。
	{}

// 默认构造函数。
	FConsoleSuggestion()
// 构造函数体（空）。
	{}
// 结构体结束。
};

// 处理控制台命令的执行
// IConsoleCommandExecutor：控制台命令执行器，继承 IModularFeature。
class IConsoleCommandExecutor : public IModularFeature
{
// 公有区。
public:
// 虚析构函数（默认）。
	virtual ~IConsoleCommandExecutor() = default;

// 取标识本模块化特性集的名称。
// ModularFeatureName：静态方法，返回固定名字。
	static FName ModularFeatureName()
// 函数体开始。
	{
// 静态常量：ConsoleCommandExecutor。
		static const FName Name = TEXT("ConsoleCommandExecutor");
// 返回该名字。
		return Name;
// 函数体结束。
	}

// 取本执行器的名字。
// GetName：纯虚。
	virtual FName GetName() const = 0;

// 取本执行器的显示名。
// GetDisplayName：纯虚。
	virtual FText GetDisplayName() const = 0;

// 取本执行器的描述。
// GetDescription：纯虚。
	virtual FText GetDescription() const = 0;

// 取本执行器的提示文本。
// GetHintText：纯虚。
	virtual FText GetHintText() const = 0;

// 取给定命令的自动补全建议列表。
// GetSuggestedCompletions：纯虚。
	virtual void GetSuggestedCompletions(const TCHAR* Input, TArray<FConsoleSuggestion>& Out) = 0;

// 取给定命令的自动补全建议列表。
// 标记 GetAutoCompleteSuggestions 已废弃（5.5），改用 GetSuggestedCompletions。
	UE_DEPRECATED(5.5, "Use GetSuggestedCompletions instead")
// GetAutoCompleteSuggestions：虚函数，默认空实现。
	virtual void GetAutoCompleteSuggestions(const TCHAR* Input, TArray<FString>& Out){};

// 取本执行器最近处理过的命令列表。
// GetExecHistory：纯虚。
	virtual void GetExecHistory(TArray<FString>& Out) = 0;

// 用本执行器执行给定命令。返回：命令被识别返回 true。
// Exec：纯虚。
	virtual bool Exec(const TCHAR* Input) = 0;

// 是否允许用「打开控制台」热键关闭控制台。注意：某些脚本语言在代码里用了默认的「打开控制台」热键（~），这些应返回 false。
// AllowHotKeyClose：纯虚。
	virtual bool AllowHotKeyClose() const = 0;

// 是否允许控制台创建多行命令。
// AllowMultiLine：纯虚。
	virtual bool AllowMultiLine() const = 0;

// 返回本执行器的热键
// GetHotKey：纯虚。
	virtual struct FInputChord GetHotKey() const = 0;
// 返回切换到下一个执行器的热键。通常与 GetHotKey 相同但带修饰键，如反引号变成 Ctrl + 反引号。
// GetIterateExecutorHotKey：纯虚。
	virtual struct FInputChord GetIterateExecutorHotKey() const = 0;
// 类结束。
};


// 处理控制台命令和变量，已注册的控制台变量在析构时释放
struct IConsoleManager
{
// 创建一个 bool 控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// RegisterConsoleVariable（bool 版）：纯虚。
	virtual IConsoleVariable* RegisterConsoleVariable(const TCHAR* Name, bool DefaultValue, const TCHAR* Help, uint32 Flags = ECVF_Default) = 0;
// 创建一个 int 控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// RegisterConsoleVariable（int32 版）：纯虚。
	virtual IConsoleVariable* RegisterConsoleVariable(const TCHAR* Name, int32 DefaultValue, const TCHAR* Help, uint32 Flags = ECVF_Default) = 0;
// 创建一个 float 控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// RegisterConsoleVariable（float 版）：纯虚。
	virtual IConsoleVariable* RegisterConsoleVariable(const TCHAR* Name, float DefaultValue, const TCHAR* Help, uint32 Flags = ECVF_Default) = 0;
// 创建一个字符串控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// RegisterConsoleVariable（TCHAR* 版）：纯虚。
	virtual IConsoleVariable* RegisterConsoleVariable(const TCHAR* Name, const TCHAR* DefaultValue, const TCHAR* Help, uint32 Flags = ECVF_Default) = 0;
// 创建一个字符串控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// RegisterConsoleVariable（FString 版）：纯虚。
	virtual IConsoleVariable* RegisterConsoleVariable(const TCHAR* Name, const FString& DefaultValue, const TCHAR* Help, uint32 Flags = ECVF_Default) = 0;
// 创建一个 bool 控制台变量的引用。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// RegisterConsoleVariableRef（bool 版）：纯虚。
	virtual IConsoleVariable* RegisterConsoleVariableRef(const TCHAR* Name, bool& RefValue, const TCHAR* Help, uint32 Flags = ECVF_Default) = 0;
// 创建一个 int 控制台变量的引用。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// RegisterConsoleVariableRef（int32 版）：纯虚。
	virtual IConsoleVariable* RegisterConsoleVariableRef(const TCHAR* Name, int32& RefValue, const TCHAR* Help, uint32 Flags = ECVF_Default) = 0;
// 创建一个 float 控制台变量的引用。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// RegisterConsoleVariableRef（float 版）：纯虚。
	virtual IConsoleVariable* RegisterConsoleVariableRef(const TCHAR* Name, float& RefValue, const TCHAR* Help, uint32 Flags = ECVF_Default) = 0;
// 创建一个字符串控制台变量的引用。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// RegisterConsoleVariableRef（FString 版）：纯虚。
	virtual IConsoleVariable* RegisterConsoleVariableRef(const TCHAR* Name, FString& RefValue, const TCHAR* Help, uint32 Flags = ECVF_Default) = 0;
// 创建一个 FName 控制台变量的引用。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// RegisterConsoleVariableRef（FName 版）：纯虚。
	virtual IConsoleVariable* RegisterConsoleVariableRef(const TCHAR* Name, FName& RefValue, const TCHAR* Help, uint32 Flags = ECVF_Default) = 0;
// 创建一个 show flag 变量的引用。参数 CVarName：不能为 0，如 Show.PostProcessing。参数 FlagName：不能为 0，如 PostProcessing。参数 BitNumber：在 Force0MaskPtr 和 Force1MaskPtr 定义的内存里的位号。参数 Force0MaskPtr：包含「应强制为 0」的位的内存。参数 Force1MaskPtr：包含「应强制为 1」的位的内存。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// RegisterConsoleVariableBitRef：纯虚。
	virtual IConsoleVariable* RegisterConsoleVariableBitRef(const TCHAR* CVarName, const TCHAR* FlagName, uint32 BitNumber, uint8* Force0MaskPtr, uint8* Force1MaskPtr, const TCHAR* Help, uint32 Flags = ECVF_Default) = 0;

	// ----------

// sink 只在自上次以来有变化时才被调用。应在极少数几个点调用：ini 文件加载后；用户控制台输入后；用户主动发起控制台变量变更（要让用户清楚 cvar 会变，如游戏选项菜单）；Tick 开始时（捕获散落的 Set() 调用，通常是不好的）。
// CallAllConsoleVariableSinks：纯虚。
	virtual void CallAllConsoleVariableSinks() = 0;

// 注册的命令在少数几个定义好的点执行（见 CallAllConsoleVariableSinks）。参数 Command：要执行的命令。
// RegisterConsoleVariableSink_Handle：纯虚。
	virtual FConsoleVariableSinkHandle RegisterConsoleVariableSink_Handle(const FConsoleCommandDelegate& Command) = 0;

// 注册的命令在少数几个定义好的点执行（见 CallAllConsoleVariableSinks）。参数 Command：要执行的命令。
// UnregisterConsoleVariableSink_Handle：纯虚。
	virtual void UnregisterConsoleVariableSink_Handle(FConsoleVariableSinkHandle Handle) = 0;

	// ----------

// 注册一个不带参数的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// RegisterConsoleCommand（无参版）：纯虚。
	virtual IConsoleCommand* RegisterConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandDelegate& Command, uint32 Flags = ECVF_Default) = 0;

// 注册一个带参数的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// RegisterConsoleCommand（带参数版）：纯虚。
	virtual IConsoleCommand* RegisterConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithArgsDelegate& Command, uint32 Flags = ECVF_Default) = 0;

// 注册一个带参数的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// RegisterConsoleCommand（带 World 版）：纯虚。
	virtual IConsoleCommand* RegisterConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithWorldDelegate& Command, uint32 Flags = ECVF_Default) = 0;

// 注册一个带参数的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// RegisterConsoleCommand（带 World 和参数版）：纯虚。
	virtual IConsoleCommand* RegisterConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithWorldAndArgsDelegate& Command, uint32 Flags = ECVF_Default) = 0;
	
// 注册一个带参数的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// RegisterConsoleCommand（带参数和输出设备版）：纯虚。
	virtual IConsoleCommand* RegisterConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithArgsAndOutputDeviceDelegate& Command, uint32 Flags = ECVF_Default) = 0;
	
// 注册一个带参数的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// RegisterConsoleCommand（带 World、参数和输出设备版）：纯虚。
	virtual IConsoleCommand* RegisterConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithWorldArgsAndOutputDeviceDelegate& Command, uint32 Flags = ECVF_Default) = 0;

// 注册一个带参数的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// RegisterConsoleCommand（带输出设备版）：纯虚。
	virtual IConsoleCommand* RegisterConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithOutputDeviceDelegate& Command, uint32 Flags = ECVF_Default) = 0;

// 注册一个由 Exec 函数处理的控制台命令（用于自动补全）。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Flags：可选标志位掩码。
// RegisterConsoleCommand（仅名字和帮助版）：纯虚。
	virtual IConsoleCommand* RegisterConsoleCommand(const TCHAR* Name, const TCHAR* Help, uint32 Flags = (uint32)ECVF_Default) = 0;

// 注销一个控制台对象（如果它已注册）。O(n)，n 是控制台对象数量。参数 ConsoleObject：要移除的对象。参数 bKeepState：是否把当前状态保留在内存里，直到同名 cvar 重新注册。
// UnregisterConsoleObject（按对象版）：纯虚。
	virtual void UnregisterConsoleObject( IConsoleObject* ConsoleObject, bool bKeepState = true) = 0;

// 按名字注销控制台变量或命令（如果该名字的对象已注册）。参数 Name：要移除的对象名字。参数 bKeepState：是否把当前状态保留在内存里，直到同名 cvar 重新注册。
// UnregisterConsoleObject（按名字版）：纯虚。
	virtual void UnregisterConsoleObject(const TCHAR* Name, bool bKeepState = true) = 0;

// 查找一个控制台变量。参数 Name：不能为 0。返回：没找到返回 0。
// FindConsoleVariable：纯虚。
	virtual IConsoleVariable* FindConsoleVariable(const TCHAR* Name, bool bTrackFrequentCalls = true) const = 0;

// 查找一个控制台变量或命令。参数 Name：不能为 0。返回：没找到返回 0。
// FindConsoleObject：纯虚。
	virtual IConsoleObject* FindConsoleObject(const TCHAR* Name, bool bTrackFrequentCalls = true) const = 0;

// 按指针查控制台对象的名字。参数 Object：要查的对象，不能为空。返回：对象名字；找不到返回空字符串。
// FindConsoleObjectName：纯虚。
	virtual FString FindConsoleObjectName(const IConsoleObject* Obj) const = 0;

// 查找带类型的控制台变量（访问值更快，无虚函数调用）。参数 Name：不能为 0。返回：没找到返回 0。
// FindTConsoleVariableDataBool：内联实现。
	TConsoleVariableData<bool>* FindTConsoleVariableDataBool(const TCHAR* Name) const
// 函数体开始。
	{
// 先按名字找 IConsoleVariable。
		IConsoleVariable* P = FindConsoleVariable(Name);

// 找到就转成 bool 数据，否则返回 0。
		return P ? P->AsVariableBool() : 0;
// 函数体结束。
	}

// 查找带类型的控制台变量（访问值更快，无虚函数调用）。参数 Name：不能为 0。返回：没找到返回 0。
// FindTConsoleVariableDataInt：内联实现。
	TConsoleVariableData<int32>* FindTConsoleVariableDataInt(const TCHAR* Name) const 
// 函数体开始。
	{ 
// 先按名字找 IConsoleVariable。
		IConsoleVariable* P = FindConsoleVariable(Name); 
		
// 找到就转成 int 数据，否则返回 0。
		return P ? P->AsVariableInt() : 0; 
// 函数体结束。
	}

// 查找带类型的控制台变量（访问值更快，无虚函数调用）。参数 Name：不能为 0。返回：没找到返回 0。
// FindTConsoleVariableDataFloat：内联实现。
	TConsoleVariableData<float>* FindTConsoleVariableDataFloat(const TCHAR* Name) const 
// 函数体开始。
	{ 
// 先按名字找 IConsoleVariable。
		IConsoleVariable* P = FindConsoleVariable(Name); 

// 找到就转成 float 数据，否则返回 0。
		return P ? P->AsVariableFloat() : 0; 
// 函数体结束。
	}


// O(n) 遍历，不区分大小写，不保证循环里 UnregisterConsoleObject() 能工作。参数 Visitor：不能为 0。参数 ThatStartsWith：不能为 0。
// ForEachConsoleObjectThatStartsWith：纯虚。
	virtual void ForEachConsoleObjectThatStartsWith( const FConsoleObjectVisitor& Visitor, const TCHAR* ThatStartsWith = TEXT("")) const = 0;

// 不区分大小写，不保证循环里 UnregisterConsoleObject() 能工作。参数 Visitor：不能为 0。参数 ThatContains：不能为 0。
// ForEachConsoleObjectThatContains：纯虚。
	virtual void ForEachConsoleObjectThatContains(const FConsoleObjectVisitor& Visitor, const TCHAR* ThatContains) const = 0;

// 处理用户输入。例如："MyCVar" 取控制台变量当前值；"MyCVar -5.2" 把值设为 -5.2；"MyCVar ?" 取帮助文本。参数 Input：不能为 0。参数 Ar：归档。参数 InWorld：World 上下文。返回：命令被识别返回 true。
// ProcessUserConsoleInput：纯虚。
	virtual bool ProcessUserConsoleInput(const TCHAR* Input, FOutputDevice& Ar, UWorld* InWorld) = 0;

// 参数 Input：不能为 0。
// AddConsoleHistoryEntry：纯虚。
	virtual void AddConsoleHistoryEntry(const TCHAR* Key, const TCHAR* Input) = 0;
	
// （无说明）
// GetConsoleHistory：纯虚。
	virtual void GetConsoleHistory(const TCHAR* Key, TArray<FString>& Out) = 0; 

// 检查某个名字（命令或变量）是否已注册到控制台管理器。参数 Name：要检查的名字，不能为 0。
// IsNameRegistered：纯虚。
	virtual bool IsNameRegistered(const TCHAR* Name) const = 0;

	// 目前只用于渲染线程
	// @param InCallback 传 0 表示禁用回调
// RegisterThreadPropagation：纯虚。
	virtual void RegisterThreadPropagation(uint32 ThreadId = 0, IConsoleThreadPropagation* InCallback = 0) = 0;

	/** 取控制台管理器的单例 **/
// Get：强制内联的静态方法。
	FORCEINLINE static IConsoleManager& Get()
// 函数体开始。
	{
// 如果单例还没创建。
		if (!Singleton)
// 条件体开始。
		{
// 创建单例。
			SetupSingleton();
// 断言单例非空。
			check(Singleton != nullptr);
// 条件体结束。
		}
// 返回单例引用。
		return *Singleton;
// 函数体结束。
	}

// 如果允许其他平台配置。
#if ALLOW_OTHER_PLATFORM_CONFIG
// 遍历可用于模拟其他平台 cvar 的最佳 ini 段集合，就像该平台自己设置的一样。它也不会包含任何 UserSettings。参数 PlatformName：平台名（ini 名，如 Windows 而不是 Win64）。参数 DeviceProfileName：非空时，从平台 ini 加载该设备配置档并插入 CVars。参数 Visit：对每个找到的 CVar 执行的回调。
// VisitPlatformCVarsForEmulation：静态方法。
	static CORE_API bool VisitPlatformCVarsForEmulation(FName PlatformName, const FString& DeviceProfileName, TFunctionRef<void(const FString& CVarName, const FString& CVarValue, EConsoleVariableFlags SetBy)> Visit);
	
// 加载所有平台 cvar，可用 IConsoleVariable::GetPlatformValueVariable 取。DeviceProfileName 为空时，会用与 Platform 同名的 DevicePlatform。
// LoadAllPlatformCVars：纯虚。
	virtual void LoadAllPlatformCVars(FName PlatformName, const FString& DeviceProfileName=FString()) = 0;
	
// 应用平台/DeviceProfile 对的 cvar，设到 SetByPreview 优先级，带 PreviewModeTag 以便之后取消。
// PreviewPlatformCVars：纯虚。
	virtual void PreviewPlatformCVars(FName PlatformName, const FString& DeviceProfileName, FName PreviewModeTag) = 0;

// 应用平台/DeviceProfile 对的 cvar，设到指定 SetBy 优先级，带 PreviewModeTag 以便之后取消。RequiredFlags 必须存在才会设置，除非 DisallowedFlags 存在则不设置。
// StompPlatformCVars：纯虚。
	virtual void StompPlatformCVars(FName PlatformName, const FString& DeviceProfileName, FName Tag, EConsoleVariableFlags SetBy, EConsoleVariableFlags RequiredFlags, EConsoleVariableFlags DisallowedFlags) = 0;

// 清空指定 Platform/DP 的所有 CVar 缓存。Platform 用 NAME_None 会清空所有缓存的平台/DP 值。DeviceProfileName 为空时，用 PlatformName 作为设备配置档名。
// ClearAllPlatformCVars：纯虚。
	virtual void ClearAllPlatformCVars(FName PlatformName=NAME_None, const FString& DeviceProfileName=FString()) = 0;

// 条件编译结束（ALLOW_OTHER_PLATFORM_CONFIG）。
#endif

// 插件卸载时需要取消它挂载时设置的 cvar。这会取消并修正所有带指定 Tag 的变量。参数 Priority：设置后，内部搜索 CVar 限制在该 SetBy。这是优化，只在你确定所有带此 tag 的 CVar 都在该优先级设置时使用。其他优先级无法取消。
// UnsetAllConsoleVariablesWithTag：纯虚。
	virtual void UnsetAllConsoleVariablesWithTag(FName Tag, EConsoleVariableFlags Priority=ECVF_SetByMask) = 0;

// 添加、替换或移除 cvar 以匹配传入的 cvar。
// BatchUpdateTag：纯虚。
	virtual void BatchUpdateTag(FName Tag, const TMap<FName, FString>& CVarsAndValues) = 0;

// OnCVarUnregistered：纯虚，取 cvar 注销多播委托。
	virtual FConsoleVariableMulticastDelegate& OnCVarUnregistered() = 0;
// OnConsoleObjectUnregistered：纯虚，取控制台对象注销多播委托。
	virtual FConsoleObjectWithNameMulticastDelegate& OnConsoleObjectUnregistered() = 0;

protected:
// 虚析构函数（protected，只能派生）。
	virtual ~IConsoleManager() { }

private:
	/** 控制台管理器的单例 **/
// Singleton：静态单例指针。
	static CORE_API IConsoleManager* Singleton;

	/** 创建单例的函数 **/
// SetupSingleton：静态方法。
	static CORE_API void SetupSingleton();
// 结构体结束。
};


// 在某个作用域内修改控制台变量值，之后恢复原值的工具类。类型参数可以是 IConsoleVariable::GetValue 支持的任何类型，即 bool、int32、float、FString。
// TGuardConsoleVariable：作用域守卫模板。
template <typename T>
// 结构体开始。
struct TGuardConsoleVariable
{
// PreviousValue：原值。
	T PreviousValue;
// ConsoleVariable：控制台变量指针。
	IConsoleVariable* ConsoleVariable;

// 构造函数：按名字找 cvar 并保存原值。
	TGuardConsoleVariable(const TCHAR* ConsoleVariableName, const T& NewValue)
// 初始化列表：查找 cvar。
		: ConsoleVariable(IConsoleManager::Get().FindConsoleVariable(ConsoleVariableName))
// 函数体开始。
	{
// 如果找到了 cvar。
		if (ensureMsgf(ConsoleVariable, TEXT("TGuardConsoleVariable: Trying to set console variable \"%s\" but it doesn't exist"), ConsoleVariableName))
// 条件体开始。
		{
// 保存原值。
			ConsoleVariable->GetValue(PreviousValue);
// 设置新值。
			ConsoleVariable->Set(NewValue);
// 条件体结束。
		}
// 函数体结束。
	}
// 析构函数：恢复原值。
	~TGuardConsoleVariable()
// 函数体开始。
	{
// 如果 cvar 存在。
		if (ConsoleVariable)
// 条件体开始。
		{
// 恢复原值。
			ConsoleVariable->Set(PreviousValue);
// 条件体结束。
		}
// 函数体结束。
	}
// 结构体结束。
};

// 自动注册控制台变量 sink（注册一个回调，在 cvar 被用户改变时调用；变化会分组并在帧/主循环的特定点发生）
// FAutoConsoleVariableSink：自动注册 sink。
class FAutoConsoleVariableSink
{
// 公有区。
public:
	/** 构造函数，保存参数以便将来从控制台变量系统移除 **/
// 构造函数：注册 sink。
	FAutoConsoleVariableSink(const FConsoleCommandDelegate& InCommand)
// 初始化列表：保存命令。
		: Command(InCommand)
// 函数体开始。
	{
// 注册 sink 并保存句柄。
		Handle = IConsoleManager::Get().RegisterConsoleVariableSink_Handle(Command);
// 函数体结束。
	}
	/** 析构函数，移除控制台变量 sink **/
// 析构函数。
	virtual ~FAutoConsoleVariableSink()
// 函数体开始。
	{
//目前禁用，析构顺序导致这不一定能工作		IConsoleManager::Get().UnregisterConsoleVariableSink_Handle(Handle);
// 函数体结束。
	}

// Command：保存的命令委托。
	const FConsoleCommandDelegate& Command;
// Handle：sink 句柄。
	FConsoleVariableSinkHandle Handle;
// 类结束。
};


// 自动注册控制台命令的基类。
// FAutoConsoleObject：自动注册控制台对象基类。
class FAutoConsoleObject
{
// 受保护区。
protected:
	/** 构造函数，保存参数以便将来从控制台变量系统移除 **/
// 构造函数：保存目标对象。
	FAutoConsoleObject(IConsoleObject* InTarget)
// 初始化列表：保存目标。
		: Target(InTarget)
// 函数体开始。
	{
// 断言目标非空。
		check(Target);
// 如果带 ECVF_GeneralShaderChange 标志。
		if (Target->TestFlags(ECVF_GeneralShaderChange))
// 条件体开始。
		{
// 加入「通用 Shader 变更」列表。
			AccessGeneralShaderChangeCvars().Add(this);
// 条件体结束。
		}
// 否则如果带 ECVF_MobileShaderChange 标志。
		else if (Target->TestFlags(ECVF_MobileShaderChange))
// 条件体开始。
		{
// 加入「移动 Shader 变更」列表。
			AccessMobileShaderChangeCvars().Add(this);
// 条件体结束。
		}
// 否则如果带 ECVF_DesktopShaderChange 标志。
		else if (Target->TestFlags(ECVF_DesktopShaderChange))
// 条件体开始。
		{
// 加入「桌面 Shader 变更」列表。
			AccessDesktopShaderChangeCvars().Add(this);
// 条件体结束。
		}
// 函数体结束。
	}
// 公有区。
public:
	/** 析构函数，移除控制台对象 **/
// 析构函数。
	virtual ~FAutoConsoleObject()
// 函数体开始。
	{
// 从控制台管理器注销目标对象。
		IConsoleManager::Get().UnregisterConsoleObject(Target);
// 函数体结束。
	}

// AccessGeneralShaderChangeCvars：取「通用 Shader 变更」列表。
	static CORE_API TArray<const FAutoConsoleObject*>& AccessGeneralShaderChangeCvars();
// AccessMobileShaderChangeCvars：取「移动 Shader 变更」列表。
	static CORE_API TArray<const FAutoConsoleObject*>& AccessMobileShaderChangeCvars();
// AccessDesktopShaderChangeCvars：取「桌面 Shader 变更」列表。
	static CORE_API TArray<const FAutoConsoleObject*>& AccessDesktopShaderChangeCvars();

	/** 把内部的控制台对象当 IConsoleVariable 返回 **/
// AsVariable（非 const 版）：强制内联。
	FORCEINLINE IConsoleVariable* AsVariable()
// 函数体开始。
	{
// 慢速断言：目标确实是变量。
		checkSlow(Target->AsVariable());
// 强转成 IConsoleVariable*。
		return static_cast<IConsoleVariable*>(Target);
// 函数体结束。
	}
	/** 把内部的控制台对象当 IConsoleVariable 返回 **/
// AsVariable（const 版）：强制内联。
	FORCEINLINE const IConsoleVariable* AsVariable() const
// 函数体开始。
	{
// 慢速断言：目标确实是变量。
		checkSlow(Target->AsVariable());
// 强转成 const IConsoleVariable*。
		return static_cast<const IConsoleVariable*>(Target);
// 函数体结束。
	}

// 私有区。
private:
	/** 内部的控制台对象，不能为 0 **/
// Target：目标对象指针。
	IConsoleObject* Target;
// 类结束。
};

// 如果没定义 NO_CVARS。
#if !NO_CVARS
// 自动注册的 float、int 或 string 控制台变量
// FAutoConsoleVariable：自动注册 cvar，私有继承 FAutoConsoleObject。
class FAutoConsoleVariable : private FAutoConsoleObject
{
// 公有区。
public:
// 创建一个 bool 控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（bool 版）：注册 cvar。
	FAutoConsoleVariable(const TCHAR* Name, bool DefaultValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
// 初始化列表：调基类构造并注册。
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariable(Name, DefaultValue, Help, Flags))
// 函数体开始。
	{
// 函数体结束。
	}
// 创建一个 int 控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（int32 版）。
	FAutoConsoleVariable(const TCHAR* Name, int32 DefaultValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariable(Name, DefaultValue, Help, Flags))
	{
	}
// 创建一个 float 控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（float 版）。
	FAutoConsoleVariable(const TCHAR* Name, float DefaultValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariable(Name, DefaultValue, Help, Flags))
	{
	}
// 创建一个字符串控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（TCHAR* 版）。
	FAutoConsoleVariable(const TCHAR* Name, const TCHAR* DefaultValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariable(Name, DefaultValue, Help, Flags))
	{
	}

// 创建一个 bool 控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Callback：变量变化时调用的委托（见 IConsoleVariable::SetOnChangedCallback）。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（bool + 回调版）。
	FAutoConsoleVariable(const TCHAR* Name, bool DefaultValue, const TCHAR* Help, const FConsoleVariableDelegate& Callback, uint32 Flags = ECVF_Default)
// 初始化列表：注册 cvar。
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariable(Name, DefaultValue, Help, Flags))
// 函数体开始。
	{
// 设置变化回调。
		AsVariable()->SetOnChangedCallback(Callback);
// 函数体结束。
	}
	
// 创建一个 int 控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Callback：变量变化时调用的委托（见 IConsoleVariable::SetOnChangedCallback）。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（int32 + 回调版）。
	FAutoConsoleVariable(const TCHAR* Name, int32 DefaultValue, const TCHAR* Help, const FConsoleVariableDelegate& Callback, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariable(Name, DefaultValue, Help, Flags))
	{
// 设置变化回调。
		AsVariable()->SetOnChangedCallback(Callback);
	}

// 创建一个 float 控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Callback：变量变化时调用的委托（见 IConsoleVariable::SetOnChangedCallback）。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（float + 回调版）。
	FAutoConsoleVariable(const TCHAR* Name, float DefaultValue, const TCHAR* Help, const FConsoleVariableDelegate& Callback, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariable(Name, DefaultValue, Help, Flags))
	{
// 设置变化回调。
		AsVariable()->SetOnChangedCallback(Callback);
	}

// 创建一个字符串控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Callback：变量变化时调用的委托（见 IConsoleVariable::SetOnChangedCallback）。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（TCHAR* + 回调版）。
	FAutoConsoleVariable(const TCHAR* Name, const TCHAR* DefaultValue, const TCHAR* Help, const FConsoleVariableDelegate& Callback, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariable(Name, DefaultValue, Help, Flags))
	{
// 设置变化回调。
		AsVariable()->SetOnChangedCallback(Callback);
	}

	/** 解引用回控制台变量 **/
// operator*（非 const 版）。
	FORCEINLINE IConsoleVariable& operator*()
// 函数体开始。
	{
// 返回变量引用。
		return *AsVariable();
// 函数体结束。
	}
// operator*（const 版）。
	FORCEINLINE const IConsoleVariable& operator*() const
// 函数体开始。
	{
// 返回变量引用。
		return *AsVariable();
// 函数体结束。
	}
	/** 解引用回控制台变量 **/
// operator->（非 const 版）。
	FORCEINLINE IConsoleVariable* operator->()
// 函数体开始。
	{
// 返回变量指针。
		return AsVariable();
// 函数体结束。
	}
// operator->（const 版）。
	FORCEINLINE const IConsoleVariable* operator->() const
// 函数体开始。
	{
// 返回变量指针。
		return AsVariable();
// 函数体结束。
	}
// 类结束。
};
#else
// FAutoConsoleVariable：空壳版。
class FAutoConsoleVariable
{
// 公有区。
public:
// 构造函数（int32 版）：什么都不做。
	FAutoConsoleVariable(const TCHAR* Name, int32 DefaultValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}

// 构造函数（float 版）：什么都不做。
	FAutoConsoleVariable(const TCHAR* Name, float DefaultValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}

// 构造函数（TCHAR* 版）：什么都不做。
	FAutoConsoleVariable(const TCHAR* Name, const TCHAR* DefaultValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}
// 类结束。
};
#endif

#if !NO_CVARS
// 自动注册的 float、int、bool、FString 引用变量类……cvar 变化时它会改那个值。
// FAutoConsoleVariableRef：自动注册 cvar 引用，私有继承 FAutoConsoleObject。
class FAutoConsoleVariableRef : private FAutoConsoleObject
{
// 公有区。
public:
// 创建一个 int 控制台变量的引用。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（int32 引用版）。
	FAutoConsoleVariableRef(const TCHAR* Name, int32& RefValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
// 初始化列表：注册引用。
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariableRef(Name, RefValue, Help, Flags))
// 函数体开始。
	{
// 函数体结束。
	}
// 创建一个 float 控制台变量的引用。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（float 引用版）。
	FAutoConsoleVariableRef(const TCHAR* Name, float& RefValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariableRef(Name, RefValue, Help, Flags))
	{
	}
// 创建一个 bool 控制台变量的引用。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（bool 引用版）。
	FAutoConsoleVariableRef(const TCHAR* Name, bool& RefValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariableRef(Name, RefValue, Help, Flags))
	{
	}
// 创建一个 FString 控制台变量的引用。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（FString 引用版）。
	FAutoConsoleVariableRef(const TCHAR* Name, FString& RefValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariableRef(Name, RefValue, Help, Flags))
	{
	}

// 创建一个 FName 控制台变量的引用。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（FName 引用版）。
	FAutoConsoleVariableRef(const TCHAR* Name, FName& RefValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariableRef(Name, RefValue, Help, Flags))
	{
	}

// 创建一个 int 控制台变量的引用。参数 Name：不能为 0。参数 Help：不能为 0。参数 Callback：变量变化时调用的委托（见 IConsoleVariable::SetOnChangedCallback）。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（int32 引用 + 回调版）。
	FAutoConsoleVariableRef(const TCHAR* Name, int32& RefValue, const TCHAR* Help, const FConsoleVariableDelegate& Callback, uint32 Flags = ECVF_Default)
// 初始化列表：注册引用。
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariableRef(Name, RefValue, Help, Flags))
// 函数体开始。
	{
// 设置变化回调。
		AsVariable()->SetOnChangedCallback(Callback);
// 函数体结束。
	}

// 创建一个 float 控制台变量的引用。参数 Name：不能为 0。参数 Help：不能为 0。参数 Callback：变量变化时调用的委托（见 IConsoleVariable::SetOnChangedCallback）。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（float 引用 + 回调版）。
	FAutoConsoleVariableRef(const TCHAR* Name, float& RefValue, const TCHAR* Help, const FConsoleVariableDelegate& Callback, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariableRef(Name, RefValue, Help, Flags))
	{
// 设置变化回调。
		AsVariable()->SetOnChangedCallback(Callback);
	}

// 创建一个 bool 控制台变量的引用。参数 Name：不能为 0。参数 Help：不能为 0。参数 Callback：变量变化时调用的委托（见 IConsoleVariable::SetOnChangedCallback）。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（bool 引用 + 回调版）。
	FAutoConsoleVariableRef(const TCHAR* Name, bool& RefValue, const TCHAR* Help, const FConsoleVariableDelegate& Callback, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariableRef(Name, RefValue, Help, Flags))
	{
// 设置变化回调。
		AsVariable()->SetOnChangedCallback(Callback);
	}

// 创建一个 FString 控制台变量的引用。参数 Name：不能为 0。参数 Help：不能为 0。参数 Callback：变量变化时调用的委托（见 IConsoleVariable::SetOnChangedCallback）。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（FString 引用 + 回调版）。
	FAutoConsoleVariableRef(const TCHAR* Name, FString& RefValue, const TCHAR* Help, const FConsoleVariableDelegate& Callback, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleVariableRef(Name, RefValue, Help, Flags))
	{
// 设置变化回调。
		AsVariable()->SetOnChangedCallback(Callback);
	}

// 虚析构函数（空）。
	virtual ~FAutoConsoleVariableRef()
// 函数体开始。
	{
// 函数体结束。
	}
	/** 解引用回变量 **/
// operator*（非 const 版）。
	FORCEINLINE IConsoleVariable& operator*()
// 函数体开始。
	{
// 返回变量引用。
		return *AsVariable();
// 函数体结束。
	}
// operator*（const 版）。
	FORCEINLINE const IConsoleVariable& operator*() const
// 函数体开始。
	{
// 返回变量引用。
		return *AsVariable();
// 函数体结束。
	}
	/** 解引用回变量 **/
// operator->（非 const 版）。
	FORCEINLINE IConsoleVariable* operator->()
// 函数体开始。
	{
// 返回变量指针。
		return AsVariable();
// 函数体结束。
	}
// operator->（const 版）。
	FORCEINLINE const IConsoleVariable* operator->() const
// 函数体开始。
	{
// 返回变量指针。
		return AsVariable();
// 函数体结束。
	}
// 类结束。
};
#else
// FAutoConsoleVariableRef：空壳版。
class FAutoConsoleVariableRef
{
// 公有区。
public:
// 构造函数（int32 引用版）：什么都不做。
	FAutoConsoleVariableRef(const TCHAR* Name, int32& RefValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}

// 构造函数（float 引用版）：什么都不做。
	FAutoConsoleVariableRef(const TCHAR* Name, float& RefValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}

// 构造函数（bool 引用版）：什么都不做。
	FAutoConsoleVariableRef(const TCHAR* Name, bool& RefValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}

// 构造函数（FString 引用版）：什么都不做。
	FAutoConsoleVariableRef(const TCHAR* Name, FString& RefValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}

// 构造函数（int32 引用 + 回调版）：什么都不做。
	FAutoConsoleVariableRef(const TCHAR* Name, int32& RefValue, const TCHAR* Help, const FConsoleVariableDelegate& Callback, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}

// 构造函数（float 引用 + 回调版）：什么都不做。
	FAutoConsoleVariableRef(const TCHAR* Name, float& RefValue, const TCHAR* Help, const FConsoleVariableDelegate& Callback, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}
	
// 构造函数（bool 引用 + 回调版）：什么都不做。
	FAutoConsoleVariableRef(const TCHAR* Name, bool& RefValue, const TCHAR* Help, const FConsoleVariableDelegate& Callback, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}

// 构造函数（FString 引用 + 回调版）：什么都不做。
	FAutoConsoleVariableRef(const TCHAR* Name, FString& RefValue, const TCHAR* Help, const FConsoleVariableDelegate& Callback, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}
// 类结束。
};
#endif // NO_CVARS


// 目前只支持主线程和渲染线程
// 为读访问速度优化（无虚函数调用，用对函数时无线程处理）
// T：int32、float
template <class T>
// TConsoleVariableData：带影子副本的 cvar 数据。
class TConsoleVariableData
{
// 公有区。
public:
	// 构造函数
// 构造函数：用默认值填满影子数组。
	TConsoleVariableData(const T DefaultValue)
// 函数体开始。
	{
// 遍历影子数组。
		for(uint32 i = 0; i < UE_ARRAY_COUNT(ShadowedValue); ++i)
// 循环体开始。
		{
// 每个元素都设为默认值。
			ShadowedValue[i] = DefaultValue;
// 循环体结束。
		}
// 函数体结束。
	}

	// 比 GetValueOnAnyThread() 快
// GetValueOnGameThread：取游戏线程的值。
	T GetValueOnGameThread() const
// 函数体开始。
	{
// 报告访问（检测跨线程误用）。
		UE::AccessDetection::ReportAccess(UE::AccessDetection::EType::CVar);
		// Shipping 下编译掉以提升性能（以后开发版可以改），如果这里触发，说明你该用 GetValueOnRenderThread() 或 GetValueOnAnyThread()，后者稍慢
		cvarCheckCode(ensure(GetShadowIndex() == 0));	// ensure 以免阻塞内容创作者，#if 以便 Shipping 优化
// 返回游戏线程的影子值。
		return ShadowedValue[0];
// 函数体结束。
	}

	// 比 GetValueOnAnyThread() 快
// GetValueOnRenderThread：取渲染线程的值。
	T GetValueOnRenderThread() const
// 函数体开始。
	{
// 报告访问。
		UE::AccessDetection::ReportAccess(UE::AccessDetection::EType::CVar);
#if !defined(__clang__) // @todo Mac：想办法让它编译通过
		// Shipping 下编译掉以提升性能（以后开发版可以改），如果这里触发，说明你该用 GetValueOnGameThread() 或 GetValueOnAnyThread()，后者稍慢
		cvarCheckCode(ensure(IsInParallelRenderingThread()));	// ensure 以免阻塞内容创作者，#if 以便 Shipping 优化
// 条件编译结束。
#endif
// 返回渲染线程的影子值。
		return ShadowedValue[1];
// 函数体结束。
	}

	// 方便，但性能更好请用 GetValueOnGameThread() 或 GetValueOnRenderThread()
// GetValueOnAnyThread：任意线程取值。
	T GetValueOnAnyThread(bool bForceGameThread = false) const
// 函数体开始。
	{
// 报告访问。
		UE::AccessDetection::ReportAccess(UE::AccessDetection::EType::CVar);
// 按当前线程选影子索引。
		return ShadowedValue[GetShadowIndex(bForceGameThread)];
// 函数体结束。
	}

private: // ----------------------------------------------------

	// [0] 主线程，[1] 渲染线程；放在同一缓存行只会在写入时受损，而 cvar 很少写
// ShadowedValue：两个影子值。
	T ShadowedValue[2];

	// @return 0 表示主线程，1 表示渲染线程，以后可能更多
// GetShadowIndex：取影子索引。
	static uint32 GetShadowIndex(bool bForceGameThread = false)
// 函数体开始。
	{	
// 如果强制游戏线程。
		if (bForceGameThread)
// 条件体开始。
		{
// 断言：不能在真正的渲染线程里。
			cvarCheckCode(ensure(!IsInActualRenderingThread()));
// 返回 0（主线程）。
			return 0;
// 条件体结束。
		}
// 在并行游戏线程或游戏线程返回 0，否则返回 1。
		return IsInParallelGameThread() || IsInGameThread() ? 0 : 1;
// 函数体结束。
	}

	// FConsoleVariable 和 FConsoleVariableRef2 需要，故意不公开
// GetReferenceOnAnyThread：取任意线程的引用。
	T& GetReferenceOnAnyThread(bool bForceGameThread = false)
// 函数体开始。
	{
// 报告访问。
		UE::AccessDetection::ReportAccess(UE::AccessDetection::EType::CVar);
// 返回影子值的引用。
		return ShadowedValue[GetShadowIndex(bForceGameThread)];
// 函数体结束。
	}

// 友元：FConsoleVariable 可访问私有成员。
	template<class T2> friend class FConsoleVariable;
// 友元：TAutoConsoleVariable 可访问私有成员。
	template<class T2> friend class TAutoConsoleVariable;
// 类结束。
};

#if !NO_CVARS
// 自动注册的 float、int 变量类……cvar 变化时它会改那个值。
// TAutoConsoleVariable：模板版自动注册 cvar。
template <class T>
// 继承 FAutoConsoleObject。
class TAutoConsoleVariable : public FAutoConsoleObject
{
// 公有区。
public:

// 构造函数：从已有 IConsoleVariable 构造。
	TAutoConsoleVariable(IConsoleVariable* ExistingObject)
// 初始化列表：调基类构造。
		: FAutoConsoleObject(ExistingObject)
// 函数体开始。
	{
// 如果 T 是 bool。
		if constexpr (std::is_same_v<T, bool>)
// 条件体开始。
		{
// 取 bool 数据指针。
			Ref = AsVariable()->AsVariableBool();
// 条件体结束。
		}
// 否则如果 T 是 int32。
		else if constexpr (std::is_same_v<T, int32>)
// 条件体开始。
		{
// 取 int 数据指针。
			Ref = AsVariable()->AsVariableInt();
// 条件体结束。
		}
// 否则如果 T 是 float。
		else if constexpr (std::is_same_v<T, float>)
// 条件体开始。
		{
// 取 float 数据指针。
			Ref = AsVariable()->AsVariableFloat();
// 条件体结束。
		}
// 否则如果 T 是 FString。
		else if constexpr (std::is_same_v<T, FString>)
// 条件体开始。
		{
// 取字符串数据指针。
			Ref = AsVariable()->AsVariableString();
// 条件体结束。
		}
// 否则（不支持的类型）。
		else
// 否则体开始。
		{
// 静态断言失败：不支持该类型。
			static_assert(sizeof(T) == 0, "Not supported");
// 否则体结束。
		}
// 函数体结束。
	}

// 创建一个 float、int 或 string 控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数：注册 cvar 并委托给上面的构造函数。
	TAutoConsoleVariable(const TCHAR* Name, const T& DefaultValue, const TCHAR* Help, uint32 Flags = ECVF_Default)
// 初始化列表：注册并转调。
		: TAutoConsoleVariable(IConsoleManager::Get().RegisterConsoleVariable(Name, DefaultValue, Help, Flags))
// 函数体开始。
	{
// 函数体结束。
	}

// 创建一个 float、int 或 string 控制台变量。参数 Name：不能为 0。参数 Help：不能为 0。参数 Callback：变量变化时调用的委托（见 IConsoleVariable::SetOnChangedCallback）。参数 Flags：由 EConsoleVariableFlags 组合的位掩码。
// 构造函数（带回调版）。
	TAutoConsoleVariable(const TCHAR* Name, const T& DefaultValue, const TCHAR* Help, const FConsoleVariableDelegate& Callback, uint32 Flags = ECVF_Default)
// 初始化列表：转调上面的构造函数。
		: TAutoConsoleVariable(Name, DefaultValue, Help, Flags)
// 函数体开始。
	{
// 设置变化回调。
		AsVariable()->SetOnChangedCallback(Callback);
// 函数体结束。
	}

// GetValueOnGameThread：转调 Ref。
	T GetValueOnGameThread() const
// 函数体开始。
	{
// 转调。
		return Ref->GetValueOnGameThread();
// 函数体结束。
	}

// GetValueOnRenderThread：转调 Ref。
	T GetValueOnRenderThread() const
// 函数体开始。
	{
// 转调。
		return Ref->GetValueOnRenderThread();
// 函数体结束。
	}

// GetValueOnAnyThread：转调 Ref。
	T GetValueOnAnyThread(bool bForceGameThread = false) const
// 函数体开始。
	{
// 转调。
		return Ref->GetValueOnAnyThread(bForceGameThread);
// 函数体结束。
	}
	
	/** 解引用回变量 **/
// operator*（非 const 版）。
	FORCEINLINE IConsoleVariable& operator*()
// 函数体开始。
	{
// 返回变量引用。
		return *AsVariable();
// 函数体结束。
	}
// operator*（const 版）。
	FORCEINLINE const IConsoleVariable& operator*() const
// 函数体开始。
	{
// 返回变量引用。
		return *AsVariable();
// 函数体结束。
	}
	/** 解引用回变量 **/
// operator->（非 const 版）。
	FORCEINLINE IConsoleVariable* operator->()
// 函数体开始。
	{
// 返回变量指针。
		return AsVariable();
// 函数体结束。
	}
// operator->（const 版）。
	FORCEINLINE const IConsoleVariable* operator->() const
// 函数体开始。
	{
// 返回变量指针。
		return AsVariable();
// 函数体结束。
	}
// 私有区。
private:
// Ref：指向 cvar 数据的指针。
	TConsoleVariableData<T>* Ref;
// 类结束。
};

#else
// TAutoConsoleVariable：空壳版，继承 IConsoleVariable。
template <class T>
class TAutoConsoleVariable : public IConsoleVariable
// 公有区。
{
public:
// 构造函数：保存默认值和标志。
	TAutoConsoleVariable(const TCHAR* Name, const T& DefaultValue, const TCHAR* InHelp, uint32 InFlags = ECVF_Default)
// 初始化列表：保存值。
		: Value(DefaultValue), Flags((EConsoleVariableFlags)InFlags)
// 函数体开始。
	{
// 函数体结束。
	}

// GetValueOnGameThread：转调 Value。
	T GetValueOnGameThread() const
// 函数体开始。
	{
// 转调。
		return Value.GetValueOnGameThread();
// 函数体结束。
	}

// GetValueOnRenderThread：转调 Value。
	T GetValueOnRenderThread() const
// 函数体开始。
	{
// 转调。
		return Value.GetValueOnRenderThread();
// 函数体结束。
	}

// GetValueOnAnyThread：转调 Value。
	T GetValueOnAnyThread(bool bForceGameThread = false) const
// 函数体开始。
	{
// 转调。
		return Value.GetValueOnAnyThread(bForceGameThread);
// 函数体结束。
	}
	
// operator*（非 const 版）。
	IConsoleVariable& operator*()
// 函数体开始。
	{
// 返回自身。
		return *AsVariable();
// 函数体结束。
	}

// operator*（const 版）。
	const IConsoleVariable& operator*() const
// 函数体开始。
	{
// 返回自身。
		return *AsVariable();
// 函数体结束。
	}

// operator->（非 const 版）。
	IConsoleVariable* operator->()
// 函数体开始。
	{
// 返回自身。
		return AsVariable();
// 函数体结束。
	}

// operator->（const 版）。
	const IConsoleVariable* operator->() const
// 函数体开始。
	{
// 返回自身。
		return AsVariable();
// 函数体结束。
	}

// AsVariable（非 const 版）：返回自身。
	IConsoleVariable*		AsVariable()		{ return this; }
// AsVariable（const 版）：返回自身。
	const IConsoleVariable* AsVariable() const	{ return this; }

// AsVariableInt：转调 AsImpl<int32>。
	virtual class TConsoleVariableData<int32>*		AsVariableInt()		override { return AsImpl<int32>(); }
// AsVariableFloat：转调 AsImpl<float>。
	virtual class TConsoleVariableData<float>*		AsVariableFloat()	override { return AsImpl<float>(); }
// AsVariableString：转调 AsImpl<FString>。
	virtual class TConsoleVariableData<FString>*	AsVariableString()	override { return AsImpl<FString>(); }

// IsVariableInt：判断 T 是否为 int32。
	virtual bool		IsVariableInt() const override	{ return std::is_same_v<int32, T>; }
// GetInt：转调 GetImpl<int32>。
	virtual int32		GetInt()		const override	{ return GetImpl<int32>(); }
// GetFloat：转调 GetImpl<float>。
	virtual float		GetFloat()		const override	{ return GetImpl<float>(); }
// GetString：转调 GetImpl<FString>。
	virtual FString		GetString()		const override	{ return GetImpl<FString>(); }
// GetBool：转调 GetImpl<bool>。
	virtual bool		GetBool()		const override { return GetImpl<bool>(); }

// GetHelp：返回固定字符串。
	virtual const TCHAR* GetHelp() const override
// 函数体开始。
	{
// 返回 NO_CVARS, no help。
		return TEXT("NO_CVARS, no help");
// 函数体结束。
	}

// GetDetailedHelp：返回固定文本。
	virtual FText GetDetailedHelp() const override
// 函数体开始。
	{
// 返回 NO_CVARS, no help。
		return INVTEXT("NO_CVARS, no help");
// 函数体结束。
	}

// SetHelp：断言失败（不支持）。
	virtual void SetHelp(const TCHAR* InHelp) override
// 函数体开始。
	{
// 断言失败。
		check(false);
// 函数体结束。
	}

// Release：断言失败（不支持）。
	virtual void Release() override
// 函数体开始。
	{
// 断言失败。
		check(false);
// 函数体结束。
	}

// SetOnChangedCallback：断言失败（不支持）。
	virtual void SetOnChangedCallback(const FConsoleVariableDelegate &) override
// 函数体开始。
	{
// 断言失败。
		check(false);
// 函数体结束。
	}

// OnChangedDelegate：断言失败并返回假委托。
	virtual FConsoleVariableMulticastDelegate& OnChangedDelegate()override
// 函数体开始。
	{
// 静态假委托。
		static FConsoleVariableMulticastDelegate Dummy;
// 断言失败。
		check(false);
// 返回假委托。
		return Dummy;
// 函数体结束。
	}

// GetFlags：返回标志。
	virtual EConsoleVariableFlags GetFlags() const override
// 函数体开始。
	{
// 返回 Flags。
		return Flags;
// 函数体结束。
	}

// SetFlags：设置标志。
	virtual void SetFlags(const EConsoleVariableFlags InFlags) override
// 函数体开始。
	{
// 保存标志。
		Flags = InFlags;
// 函数体结束。
	}

// Set：从字符串设置值。
	virtual void Set(const TCHAR* InValue, EConsoleVariableFlags SetBy) override
// 函数体开始。
	{
// 从字符串解析到影子值。
		LexFromString(Value.ShadowedValue[0], InValue);
// 函数体结束。
	}

// 私有区。
private:
// Value：cvar 数据。
	TConsoleVariableData<T> Value;
// Help：帮助文本。
	FString Help;
// Flags：标志位。
	EConsoleVariableFlags Flags = EConsoleVariableFlags::ECVF_Default;

// GetImpl：按类型取值。
	template<class Y>
// 函数体开始。
	Y GetImpl() const
// 如果 T 和 Y 相同。
	{
// 条件体开始。
		if constexpr (std::is_same_v<T, Y>)
// 返回任意线程的值。
		{
// 条件体结束。
			return GetValueOnAnyThread();
// 否则。
		}
// 否则体开始。
		else
// 断言失败。
		{
// 返回默认值。
			check(false);
// 否则体结束。
			return Y();
// 函数体结束。
		}
	}
// AsImpl：按类型取数据指针。

// 函数体开始。
	template<class Y>
// 如果 T 和 Y 相同。
	TConsoleVariableData<T>* AsImpl()
// 条件体开始。
	{
// 返回 Value 地址。
		if constexpr (std::is_same_v<T, Y>)
// 条件体结束。
		{
// 否则。
			return &Value;
// 否则体开始。
		}
// 断言失败。
		else
// 返回空指针。
		{
// 否则体结束。
			check(false);
// 函数体结束。
			return nullptr;
// 类结束。
		}
// 条件编译结束（NO_CVARS）。
	}
};
#endif // NO_CVARS

#if !NO_CVARS
// 自动注册控制台命令
// FAutoConsoleCommand：自动注册命令，私有继承 FAutoConsoleObject。
class FAutoConsoleCommand : private FAutoConsoleObject
{
// 公有区。
public:
// 注册一个不带参数的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// 构造函数（无参版）。
	FAutoConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandDelegate& Command, uint32 Flags = ECVF_Default)
// 初始化列表：注册命令。
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleCommand(Name, Help, Command, Flags))
// 函数体开始。
	{
// 函数体结束。
	}

// 注册一个带参数的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// 构造函数（带参数版）。
	FAutoConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithArgsDelegate& Command, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleCommand(Name, Help, Command, Flags))
	{
	}

// 注册一个带输出设备的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// 构造函数（带输出设备版）。
	FAutoConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithOutputDeviceDelegate& Command, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleCommand(Name, Help, Command, Flags))
	{
	}

// 注册一个带输出设备的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// 构造函数（带参数和输出设备版）。
    FAutoConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithArgsAndOutputDeviceDelegate& Command, uint32 Flags = ECVF_Default)
        : FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleCommand(Name, Help, Command, Flags))
    {
    }

// 注册一个带 World 参数的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// 构造函数（带 World 版）。
	FAutoConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithWorldDelegate& Command, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleCommand(Name, Help, Command, Flags))
	{
	}

// 注册一个带参数、World 参数和输出设备的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// 构造函数（带 World、参数和输出设备版）。
	FAutoConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithWorldArgsAndOutputDeviceDelegate& Command, uint32 Flags = ECVF_Default)
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleCommand(Name, Help, Command, Flags))
	{
	}
// 类结束。
};
#else
// FAutoConsoleCommand：空壳版。
class FAutoConsoleCommand
{
// 公有区。
public:
// 构造函数（无参版）：什么都不做。
	FAutoConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandDelegate& Command, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}

// 构造函数（带参数版）：什么都不做。
	FAutoConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithArgsDelegate& Command, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}

// 构造函数（带输出设备版）：什么都不做。
	FAutoConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithOutputDeviceDelegate& Command, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}

// 构造函数（带参数和输出设备版）：什么都不做。
	FAutoConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithArgsAndOutputDeviceDelegate& Command, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}

// 构造函数（带 World 版）：什么都不做。
	FAutoConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithWorldDelegate& Command, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}

// 构造函数（带 World、参数和输出设备版）：什么都不做。
	FAutoConsoleCommand(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithWorldArgsAndOutputDeviceDelegate& Command, uint32 Flags = ECVF_Default)
// 函数体开始。
	{
// 函数体结束。
	}
// 类结束。
};
#endif


#if !NO_CVARS
// 带 World 的自动注册控制台命令
// FAutoConsoleCommandWithWorld：自动注册命令（带 World）。
class FAutoConsoleCommandWithWorld : private FAutoConsoleObject
{
// 公有区。
public:
// 注册一个带 World 参数的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// 构造函数。
	FAutoConsoleCommandWithWorld(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithWorldDelegate& Command, uint32 Flags = ECVF_Default)
// 初始化列表：注册命令。
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleCommand(Name, Help, Command, Flags))
// 函数体开始。
	{
// 函数体结束。
	}

	
// 类结束。
};

// 带 World 和参数的自动注册控制台命令
// FAutoConsoleCommandWithWorldAndArgs：自动注册命令（带 World 和参数）。
class FAutoConsoleCommandWithWorldAndArgs : private FAutoConsoleObject
{
// 公有区。
public:	
// 注册一个带参数和 World 参数的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// 构造函数。
	FAutoConsoleCommandWithWorldAndArgs(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithWorldAndArgsDelegate& Command, uint32 Flags = ECVF_Default)
// 初始化列表：注册命令。
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleCommand(Name, Help, Command, Flags))
// 函数体开始。
	{
// 函数体结束。
	}
// 类结束。
};

// 带参数和输出设备的自动注册控制台命令
// FAutoConsoleCommandWithArgsAndOutputDevice：自动注册命令（带参数和输出设备）。
class FAutoConsoleCommandWithArgsAndOutputDevice : private FAutoConsoleObject
{
// 公有区。
public:
	
// 构造函数。
	FAutoConsoleCommandWithArgsAndOutputDevice(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithArgsAndOutputDeviceDelegate& Command, uint32 Flags = ECVF_Default)
// 初始化列表：注册命令。
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleCommand(Name, Help, Command, Flags))
// 函数体开始。
	{
// 函数体结束。
	}
// 类结束。
};

// 带输出设备的自动注册控制台命令
// FAutoConsoleCommandWithOutputDevice：自动注册命令（带输出设备）。
class FAutoConsoleCommandWithOutputDevice : private FAutoConsoleObject
{
// 公有区。
public:
// 注册一个带输出设备的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// 构造函数。
	FAutoConsoleCommandWithOutputDevice(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithOutputDeviceDelegate& Command, uint32 Flags = ECVF_Default)
// 初始化列表：注册命令。
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleCommand(Name, Help, Command, Flags))
// 函数体开始。
	{
// 函数体结束。
	}
// 类结束。
};

// 带 World、参数和输出设备的自动注册控制台命令
// FAutoConsoleCommandWithWorldArgsAndOutputDevice：自动注册命令（带 World、参数和输出设备）。
class FAutoConsoleCommandWithWorldArgsAndOutputDevice : private FAutoConsoleObject
{
// 公有区。
public:
// 注册一个带输出设备的控制台命令。参数 Name：本命令的名字（不能为 nullptr）。参数 Help：本命令的帮助文本。参数 Command：命令执行时调用的用户函数。参数 Flags：可选标志位掩码。
// 构造函数。
	FAutoConsoleCommandWithWorldArgsAndOutputDevice(const TCHAR* Name, const TCHAR* Help, const FConsoleCommandWithWorldArgsAndOutputDeviceDelegate& Command, uint32 Flags = ECVF_Default)
// 初始化列表：注册命令。
		: FAutoConsoleObject(IConsoleManager::Get().RegisterConsoleCommand(Name, Help, Command, Flags))
// 函数体开始。
	{
// 函数体结束。
	}
// 类结束。
};

#else

// FAutoConsoleCommandWithWorld：空壳版。
class FAutoConsoleCommandWithWorld
{
// 公有区。
public:
// 可变参数构造函数：什么都不做。
	template<class... Args> FAutoConsoleCommandWithWorld(const Args&...) {}
// 类结束。
};

// FAutoConsoleCommandWithWorldAndArgs：空壳版。
class FAutoConsoleCommandWithWorldAndArgs
{
// 公有区。
public:
// 可变参数构造函数：什么都不做。
	template<class... Args> FAutoConsoleCommandWithWorldAndArgs(const Args&...) {}
// 类结束。
};

// FAutoConsoleCommandWithOutputDevice：空壳版。
class FAutoConsoleCommandWithOutputDevice
{
// 公有区。
public:
// 可变参数构造函数：什么都不做。
	template<class... Args> FAutoConsoleCommandWithOutputDevice(const Args&...) {}
// 类结束。
};

// FAutoConsoleCommandWithWorldArgsAndOutputDevice：空壳版。
class FAutoConsoleCommandWithWorldArgsAndOutputDevice
{
// 公有区。
public:
// 可变参数构造函数：什么都不做。
	template<class... Args> FAutoConsoleCommandWithWorldArgsAndOutputDevice(const Args&...) {}
// 类结束。
};

#endif


// EShadowCVarBehavior：影子 cvar 的行为（首次使用时怎么处理）。
enum class EShadowCVarBehavior : uint8
{
	// 跳过所有消息
// NoMessaging：不提示。
	NoMessaging,
	// 首次使用时打警告
// Warn：警告。
	Warn,
	// 首次使用时打错误
// Error：错误。
	Error,
	// 首次使用时触发 ensure
// Ensure：ensure。
	Ensure,
	// 首次使用时触发断言
// Assert：断言。
	Assert,
// 枚举体结束。
};

// 用来影子（别名）一个 CVar 的类，两个名字都可用。要废弃 CVar 用 FAutoConsoleVariableDeprecated。要废弃 CCmd 用 FAutoConsoleCommandDeprecated。用法示例：FAutoConsoleVariableShadow ShadowCVar(TEXT("r.oldname"), TEXT("r.newname"));
// FAutoConsoleVariableShadow：影子 cvar。
class FAutoConsoleVariableShadow
{
// 公有区。
public:

// 参数 Name：本 cvar 的名字。参数 CVarToShadow：本 cvar 要影子的已有 cvar 名字。参数 LookupFailureBehavior：查找 CVarToShadow 失败时怎么处理，默认 ensure。
// 构造函数声明（实现在 .cpp）。
	CORE_API FAutoConsoleVariableShadow(const TCHAR* Name, const TCHAR* CVarToShadow, EShadowCVarBehavior LookupFailureBehavior = EShadowCVarBehavior::Ensure);
// 类结束。
};

// 用来废弃一个 CVar 的类：把已有 CVar 改名，然后用旧名建一个影子。用法示例：FAutoConsoleVariableDeprecated ShadowCVar(TEXT("r.oldname"), TEXT("r.newname"), TEXT("5.5"));
// FAutoConsoleVariableDeprecated：废弃 cvar。
class FAutoConsoleVariableDeprecated
{
// 公有区。
public:
// 参数 Name：本 cvar 的名字。参数 CVarToShadow：本 cvar 要影子的已有 cvar 名字。参数 DeprecatedAtVersion：首次废弃的 UE 版本。参数 UsageBehavior：用废弃名使用影子 cvar 时怎么处理，默认 ensure。参数 LookupFailureBehavior：查找 CVarToShadow 失败时怎么处理，默认 ensure。参数 DeprecationMessage：可选废弃消息，默认空（自动生成）。
// 构造函数声明（实现在 .cpp）。
	CORE_API FAutoConsoleVariableDeprecated(const TCHAR* Name, const TCHAR* CVarToShadow, const TCHAR* DeprecatedAtVersion, EShadowCVarBehavior UsageBehavior=EShadowCVarBehavior::Ensure, EShadowCVarBehavior LookupFailureBehavior = EShadowCVarBehavior::Ensure, const TCHAR* DeprecationMessage = TEXT(""));
// 类结束。
};

// 用来废弃一个 ConsoleCommand 的类：用法同 FAutoConsoleVariableDeprecated
// FAutoConsoleCommandDeprecated：废弃命令。
class FAutoConsoleCommandDeprecated
{
// 公有区。
public:
// 参数 Name：本 cvar 的名字。参数 CVarToShadow：本 cvar 要影子的已有 cvar 名字。参数 DeprecatedAtVersion：首次废弃的 UE 版本。参数 UsageBehavior：用废弃名使用影子 cvar 时怎么处理，默认 ensure。参数 LookupFailureBehavior：查找 CVarToShadow 失败时怎么处理，默认 ensure。参数 DeprecationMessage：可选废弃消息，默认空（自动生成）。
// 构造函数声明（实现在 .cpp）。
	CORE_API FAutoConsoleCommandDeprecated(const TCHAR* Name, const TCHAR* CVarToShadow, const TCHAR* DeprecatedAtVersion, EShadowCVarBehavior UsageBehavior = EShadowCVarBehavior::Ensure, EShadowCVarBehavior LookupFailureBehavior = EShadowCVarBehavior::Ensure, const TCHAR* DeprecationMessage = TEXT(""));
// 类结束。
};


// 创建带范围的 cvar 的类。min 和 max 可用 <name>.min 和 <name>.max 修改
// TAutoRangedConsoleVariable：带范围的自动注册 cvar。
template<typename T>
// 继承 TAutoConsoleVariable<T>。
class TAutoRangedConsoleVariable : public TAutoConsoleVariable<T>
{
// 公有区。
public:
// 构造函数：多一个 MinValue / MaxValue / Parent 参数。
	CORE_API TAutoRangedConsoleVariable(const TCHAR* Name, T DefaultValue, T MinValue, T MaxValue, const TCHAR* Help, uint32 Flags = ECVF_Default, IConsoleVariable* Parent = nullptr);
// 类结束。
};


// LogConsoleResponse：控制台响应日志分类。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogConsoleResponse, Log, All);

