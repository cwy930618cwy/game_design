// 版权所有 Epic Games, Inc. 保留所有权利。
// 版权声明，引擎每个源文件第一行都是它，交付时译成中文。

// 头文件保护：保证同一个 .cpp 里多次 include 本文件时只展开一次。
#pragma once

/*----------------------------------------------------------------------------
	Low level includes.
----------------------------------------------------------------------------*/
/* 分节注释：下面是「底层 include」区。 */

// 引入基础类型层（平台宏 + 基础整数类型 + 构建开关宏）。
// 真实文件：Core\Public\CoreTypes.h（15 行）
// 里面又 include 了 HAL/Platform.h / UMemoryDefines.h / CoreMiscDefines.h / CoreDefines.h。
#include "CoreTypes.h"

/*----------------------------------------------------------------------------
	Forward declarations
----------------------------------------------------------------------------*/
/* 分节注释：下面是「前置声明」区。 */

// 引入前置声明总表。
// 真实文件：Core\Public\CoreFwd.h（44 行）
// 里面声明：FName / FArchive / FText / FGuid 等 34 个类型。
#include "CoreFwd.h"

// 引入 UObject 层级前置声明。
// 真实文件：Core\Public\UObject\UObjectHierarchyFwd.h（56 行）
// 里面声明：UObject / UClass / UStruct / UFunction / FProperty 等。
// 注意：CoreFwd.h 第 8 行已经 include 过它了，这里重复 include 是引擎原样
// （有 #pragma once 保护，不会重复展开）。
#include "UObject/UObjectHierarchyFwd.h"

// 引入容器前置声明。
// 真实文件：Core\Public\Containers\ContainersFwd.h（77 行）
// 里面声明：FString / FAnsiString / FUtf8String / 分配器。
// 同样：CoreFwd.h 第 6 行已 include 过，这里重复是引擎原样。
#include "Containers/ContainersFwd.h"

/*----------------------------------------------------------------------------
	Commonly used headers
----------------------------------------------------------------------------*/
/* 分节注释：下面是「常用头」区——148 个 include 全在这里。 */

// 变参宏（UE_LOG 的可变参数支持）。
#include "Misc/VarArgs.h"
// 日志详细级别枚举（ELogVerbosity::Log / Warning / Error 等）。
#include "Logging/LogVerbosity.h"
// 输出设备基类（FOutputDevice，日志输出的抽象基类）。
#include "Misc/OutputDevice.h"
// 平台 C 运行时头（printf / malloc / memcpy 等 C 标准库封装）。
#include "HAL/PlatformCrt.h"
// 平台杂项（FPlatformMisc，获取设备信息 / 系统语言 / 剪贴板等）。
#include "HAL/PlatformMisc.h"
// 断言宏（check / checkf / ensure / verify）。
#include "Misc/AssertionMacros.h"
// 类型萃取：判断是不是指针（TIsPointer<T>）。
#include "Templates/IsPointer.h"
// 平台内存（FPlatformMemory，查询物理内存 / 内存统计）。
#include "HAL/PlatformMemory.h"
// 平台原子操作（FPlatformAtomics，原子加减 / 比较交换）。
#include "HAL/PlatformAtomics.h"
// 控制台命令执行器（FExec）。
#include "Misc/Exec.h"
// 内存分配基类（FMalloc / FMemory）。
#include "HAL/MemoryBase.h"
// 内存操作（FMemory::Malloc / Free / Memcpy / Memset）。
#include "HAL/UnrealMemory.h"
// 类型萃取：判断是不是算术类型（TIsArithmetic<T>）。
#include "Templates/IsArithmetic.h"
// 逻辑运算模板（TAnd / TOr / TNot）。
#include "Templates/AndOrNot.h"
// 类型萃取：判断是不是 POD 类型（TIsPODType<T>）。
#include "Templates/IsPODType.h"
// 类型萃取：判断是不是 UE 核心类型（TIsUECoreType<T>）。
#include "Templates/IsUECoreType.h"
// 类型萃取：判断是不是可平凡拷贝构造（TIsTriviallyCopyConstructible<T>）。
#include "Templates/IsTriviallyCopyConstructible.h"
// UE 类型萃取总表（TIsSame / TIsDerivedFrom 等）。
#include "Templates/UnrealTypeTraits.h"
// 条件启用模板（TEnableIf<T>）。
#include "Templates/EnableIf.h"
// 移除引用（TRemoveReference<T>）。
#include "Templates/RemoveReference.h"
// 整型常量模板（TIntegralConstant<T, V>）。
#include "Templates/IntegralConstant.h"
// 类型萃取：判断是不是类（TIsClass<T>）。
#include "Templates/IsClass.h"
// 类型兼容字节容器（TTypeCompatibleBytes<T>）。
#include "Templates/TypeCompatibleBytes.h"
// 类型萃取：判断是不是连续容器（TIsContiguousContainer<T>）。
#include "Traits/IsContiguousContainer.h"
// UE 模板工具总表（MoveTemp / Forward / Swap 等）。
#include "Templates/UnrealTemplate.h"
// 数值极限（TNumericLimits<T>::Max / Min）。
#include "Math/NumericLimits.h"
// 平台数学（FPlatformMath，Sqrt / Sin / Cos 等）。
#include "HAL/PlatformMath.h"
// 类型萃取：判断是不是可平凡拷贝赋值（TIsTriviallyCopyAssignable<T>）。
#include "Templates/IsTriviallyCopyAssignable.h"
// 内存操作模板（DefaultConstructItems / DestructItems 等）。
#include "Templates/MemoryOps.h"
// 容器分配策略（FDefaultAllocator / TInlineAllocator 等）。
#include "Containers/ContainerAllocationPolicies.h"
// 类型萃取：判断是不是强类型枚举（TIsEnumClass<T>）。
#include "Templates/IsEnumClass.h"
// 平台属性（FPlatformProperties，判断是不是服务器 / 客户端构建）。
#include "HAL/PlatformProperties.h"
// 兼容块：UE5.4 之前的 include 顺序（老代码依赖 Compression.h 被间接带进来）。
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_4
#include "Misc/Compression.h"
#endif
// 引擎版本号基类（FEngineVersionBase）。
#include "Misc/EngineVersionBase.h"
// 文本命名空间前置声明（FTextNamespace）。
#include "Internationalization/TextNamespaceFwd.h"
// 序列化归档（FArchive，读写二进制数据的抽象基类）。
#include "Serialization/Archive.h"
// 比较模板（TLess<T>）。
#include "Templates/Less.h"
// 排序模板（Sort / StableSort）。
#include "Templates/Sorting.h"
// 字符类型（TCHAR / ANSICHAR / UTF8CHAR 及转换）。
#include "Misc/Char.h"
// 通用平台字符串比较（Stricmp）。
#include "GenericPlatform/GenericPlatformStricmp.h"
// 通用平台字符串操作（Strcpy / Strlen 等）。
#include "GenericPlatform/GenericPlatformString.h"
// 平台字符串（FPlatformString，平台特化）。
#include "HAL/PlatformString.h"
// C 字符串工具（FCString，Strcmp / Atoi / Itoa 等）。
#include "Misc/CString.h"
// CRC 校验（FCrc，计算 CRC32）。
#include "Misc/Crc.h"
// 数学工具（FMath，Clamp / Lerp / Abs / Max / Min 等）。
#include "Math/UnrealMathUtility.h"
// 字符串类（FString / FAnsiString / FUtf8String）。
#include "Containers/UnrealString.h"
// 动态数组（TArray<T>）。
#include "Containers/Array.h"
// 帧号（FFrameNumber）。
#include "Misc/FrameNumber.h"
// 时间跨度（FTimespan）。
#include "Misc/Timespan.h"
// 兼容块：UE5.4 之前的 include 顺序（老代码依赖 CriticalSection.h）。
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_4
#include "HAL/CriticalSection.h"
#endif
// 字符串编码转换（TCHAR_TO_UTF8 / UTF8_TO_TCHAR 等）。
#include "Containers/StringConv.h"
// 引擎内置名字（NAME_None / NAME_Object 等）。
#include "UObject/UnrealNames.h"
// 名字类型（FName）。
#include "UObject/NameTypes.h"
// 解析工具（FParse，解析命令行 / 配置文件）。
#include "Misc/Parse.h"
// 兼容块：UE5.4 之前的 include 顺序（老代码依赖 Color.h）。
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_4
#include "Math/Color.h"
#endif
// 对齐模板（Align / AlignOf）。
#include "Templates/AlignmentTemplates.h"
// 结构体构建器（TStructBuilder）。
#include "Misc/StructBuilder.h"
// 类型退化（TDecay<T>）。
#include "Templates/Decay.h"
// 指针可转换判断（TPointerIsConvertibleFromTo<T, U>）。
#include "Templates/PointerIsConvertibleFromTo.h"
// 调用模板（Invoke）。
#include "Templates/Invoke.h"
// 函数对象（TFunction<T>，std::function 的 UE 版）。
#include "Templates/Function.h"
// 类型哈希（GetTypeHash）。
#include "Templates/TypeHash.h"
// 脚本数组（FScriptArray，蓝图用的动态数组）。
#include "Containers/ScriptArray.h"
// 位数组（TBitArray）。
#include "Containers/BitArray.h"
// 稀疏数组（TSparseArray）。
#include "Containers/SparseArray.h"
// 集合（TSet<T>）。
#include "Containers/Set.h"
// 反转算法（Algo::Reverse）。
#include "Algo/Reverse.h"
// 映射（TMap<K, V>）。
#include "Containers/Map.h"
// 兼容块：UE5.4 之前的 include 顺序（老代码依赖 ColorList.h）。
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_4
#include "Math/ColorList.h"
#endif
// 整数点（FIntPoint）。
#include "Math/IntPoint.h"
// 整数向量（FIntVector）。
#include "Math/IntVector.h"
// 日志分类（DECLARE_LOG_CATEGORY_EXTERN）。
#include "Logging/LogCategory.h"
// 日志宏（UE_LOG / UE_LOG_ACTIVE）。
#include "Logging/LogMacros.h"
// 二维向量（FVector2D）。
#include "Math/Vector2D.h"
// 整数矩形（FIntRect）。
#include "Math/IntRect.h"
// 字节序转换（ByteSwap）。
#include "Misc/ByteSwap.h"
// 枚举当字节用（TEnumAsByte<T>）。
#include "Containers/EnumAsByte.h"
// 平台线程局部存储（FPlatformTLS）。
#include "HAL/PlatformTLS.h"
// 全局量（GLog / GError / GIsEditor / GIsServer）。
#include "CoreGlobals.h"
// 共享指针（TSharedPtr / TSharedRef / TWeakPtr）。
#include "Templates/SharedPointer.h"
// 文化指针（FCulturePtr）。
#include "Internationalization/CulturePointer.h"
// 弱对象指针模板（TWeakObjectPtr<T>）。
#include "UObject/WeakObjectPtrTemplates.h"
// 委托设置（UE_DELEGATE_... 宏）。
#include "Delegates/DelegateSettings.h"
// 委托实例接口（IDelegateInstance）。
#include "Delegates/IDelegateInstance.h"
// 委托基类（FDelegateBase）。
#include "Delegates/DelegateBase.h"
// 多播委托基类（FMulticastDelegateBase）。
#include "Delegates/MulticastDelegateBase.h"
// 整数序列（TIntegerSequence）。
#include "Delegates/IntegerSequence.h"
// 元组（TTuple<T...>）。
#include "Templates/Tuple.h"
// 脚本委托（FScriptDelegate，蓝图用）。
#include "UObject/ScriptDelegates.h"
// 委托（TDelegate<T>）。
#include "Delegates/Delegate.h"
// 文本本地化管理器（FTextLocalizationManager）。
#include "Internationalization/TextLocalizationManager.h"
// 可选值（TOptional<T>）。
#include "Misc/Optional.h"
// 类型萃取：判断是不是数组（TIsArray<T>）。
#include "Templates/IsArray.h"
// 移除数组维度（TRemoveExtent<T>）。
#include "Templates/RemoveExtent.h"
// 独占指针（TUniquePtr<T>）。
#include "Templates/UniquePtr.h"
// 本地化文本（FText）。
#include "Internationalization/Text.h"
// 独占对象（TUniqueObj<T>）。
#include "Templates/UniqueObj.h"
// 国际化（FInternationalization，语言 / 文化管理）。
#include "Internationalization/Internationalization.h"
// 三维向量（FVector）。
#include "Math/Vector.h"
// 四维向量（FVector4）。
#include "Math/Vector4.h"
// 向量寄存器（VectorRegister，SIMD）。
#include "Math/VectorRegister.h"
// 双向量（FTwoVectors）。
#include "Math/TwoVectors.h"
// 边（FEdge）。
#include "Math/Edge.h"
// 对象版本（FObjectVersion，序列化版本号）。
#include "UObject/ObjectVersion.h"
// 兼容块：UE5.4 之前的 include 顺序（老代码依赖 Plane.h / Sphere.h）。
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_4
#include "Math/Plane.h"
#include "Math/Sphere.h"
#endif
// 胶囊体（FCapsuleShape）。
#include "Math/CapsuleShape.h"
// 旋转体（FRotator）。
#include "Math/Rotator.h"
// 日期时间（FDateTime）。
#include "Misc/DateTime.h"
// 范围边界（TRangeBound<T>）。
#include "Math/RangeBound.h"
// 自动化测试事件（FAutomationEvent）。
#include "Misc/AutomationEvent.h"
// 范围（TRange<T>）。
#include "Math/Range.h"
// 范围集合（TRangeSet<T>）。
#include "Math/RangeSet.h"
// 区间（TInterval<T>）。
#include "Math/Interval.h"
// 轴对齐包围盒（FBox）。
#include "Math/Box.h"
// 二维包围盒（FBox2D）。
#include "Math/Box2D.h"
// 球包围盒（FBoxSphereBounds）。
#include "Math/BoxSphereBounds.h"
// 有向包围盒（FOrientedBox）。
#include "Math/OrientedBox.h"
// 轴（FAxis）。
#include "Math/Axis.h"
// 矩阵（FMatrix）。
#include "Math/Matrix.h"
// 旋转平移矩阵（FRotationTranslationMatrix）。
#include "Math/RotationTranslationMatrix.h"
// 绕点旋转矩阵（FRotationAboutPointMatrix）。
#include "Math/RotationAboutPointMatrix.h"
// 缩放旋转平移矩阵（FScaleRotationTranslationMatrix）。
#include "Math/ScaleRotationTranslationMatrix.h"
// 旋转矩阵（FRotationMatrix）。
#include "Math/RotationMatrix.h"
// 四元数（FQuat）。
#include "Math/Quat.h"
// 透视投影矩阵（FPerspectiveMatrix）。
#include "Math/PerspectiveMatrix.h"
// 正交投影矩阵（FOrthoMatrix）。
#include "Math/OrthoMatrix.h"
// 平移矩阵（FTranslationMatrix）。
#include "Math/TranslationMatrix.h"
// 四元数旋转平移矩阵（FQuatRotationTranslationMatrix）。
#include "Math/QuatRotationTranslationMatrix.h"
// 逆旋转矩阵（FInverseRotationMatrix）。
#include "Math/InverseRotationMatrix.h"
// 缩放矩阵（FScaleMatrix）。
#include "Math/ScaleMatrix.h"
// 镜像矩阵（FMirrorMatrix）。
#include "Math/MirrorMatrix.h"
// 裁剪投影矩阵（FClipProjectionMatrix）。
#include "Math/ClipProjectionMatrix.h"
// 兼容块：UE5.4 之前的 include 顺序（老代码依赖插值曲线）。
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_4
#include "Math/InterpCurvePoint.h"
#include "Math/InterpCurve.h"
#include "Math/CurveEdInterface.h"
#endif
// 32 位浮点（FFloat32）。
#include "Math/Float32.h"
// 16 位浮点（FFloat16）。
#include "Math/Float16.h"
// 兼容块：UE5.4 之前的 include 顺序（老代码依赖 Float16Color / Vector2DHalf）。
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_4
#include "Math/Float16Color.h"
#include "Math/Vector2DHalf.h"
#endif
// 变换（FTransform，位置 + 旋转 + 缩放）。
#include "Math/Transform.h"
// 二维凸包（FConvexHull2D）。
#include "Math/ConvexHull2d.h"
// 数学总表（FMath 的完整定义）。
#include "Math/UnrealMath.h"

// 兼容块：UE5.5 之前的 include 顺序（老代码依赖 IsTriviallyDestructible.h）。
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_5
#include "Templates/IsTriviallyDestructible.h"
#endif