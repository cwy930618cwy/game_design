// 版权所有 Epic Games, Inc. 保留所有权利。
// 版权声明，引擎每个源文件第一行都是它，交付时译成中文。

// 头文件保护：保证同一个 .cpp 里多次 include 本文件时只展开一次。
#pragma once

// 引入「单体头样板」宏定义。
// 真实文件：Core\Public\Misc\MonolithicHeaderBoilerplate.h（11 行）
// 里面定义 MONOLITHIC_HEADER_BOILERPLATE() 宏：
//   - 如果是引擎模块 + 没抑制警告 + 不是直接头编译 → 展开成 COMPILE_WARNING(...)
//   - 否则 → 展开成空
#include "Misc/MonolithicHeaderBoilerplate.h"
// 展开 MONOLITHIC_HEADER_BOILERPLATE() 宏。
// 作用：如果有人 include 了 Core.h（单体头），编译器会警告
// 「Monolithic headers should not be used by this module. Please change it to explicitly include the headers it needs.」
// 为什么：Core.h 一次拉 294 个头，编译很慢；UE 鼓励按需 include（IWYU）。
MONOLITHIC_HEADER_BOILERPLATE()

/*----------------------------------------------------------------------------
	Low level includes.
----------------------------------------------------------------------------*/
/* 分节注释：下面是「底层 include」区（第 12~100 行）。 */

// IWYU pragma: begin_exports
// IWYU 指令：从这里开始，本文件 include 的所有头都算「导出」——
// 别的文件 include 了 Core.h，就等于也 include 了下面这 294 个头。
// 为什么需要：Core.h 自己不定义任何东西，只是「转发」，必须告诉 IWYU 别删。

// 最小可用集（148 个常用头）。
#include "CoreMinimal.h"
// 容器前置声明（FString / FAnsiString / FUtf8String）。
#include "Containers/ContainersFwd.h"
// 时间跨度（FTimespan）。
#include "Misc/Timespan.h"
// 日期时间（FDateTime）。
#include "Misc/DateTime.h"
// 平台 C 运行时（printf / malloc）。
#include "HAL/PlatformCrt.h"
// 通用平台内存（FGenericPlatformMemory）。
#include "GenericPlatform/GenericPlatformMemory.h"
// 平台内存（FPlatformMemory）。
#include "HAL/PlatformMemory.h"
// 字符类型（TCHAR / ANSICHAR）。
#include "Misc/Char.h"
// 条件启用（TEnableIf<T>）。
#include "Templates/EnableIf.h"
// 逻辑运算（TAnd / TOr / TNot）。
#include "Templates/AndOrNot.h"
// 算术类型判断（TIsArithmetic<T>）。
#include "Templates/IsArithmetic.h"
// 浮点判断（TIsFloatingPoint<T>）。
#include "Templates/IsFloatingPoint.h"
// 整型判断（TIsIntegral<T>）。
#include "Templates/IsIntegral.h"
// 指针判断（TIsPointer<T>）。
#include "Templates/IsPointer.h"
// POD 判断（TIsPODType<T>）。
#include "Templates/IsPODType.h"
// UE 核心类型判断（TIsUECoreType<T>）。
#include "Templates/IsUECoreType.h"
// 有符号判断（TIsSigned<T>）。
#include "Templates/IsSigned.h"
// 可平凡拷贝赋值判断。
#include "Templates/IsTriviallyCopyAssignable.h"
// 可平凡拷贝构造判断。
#include "Templates/IsTriviallyCopyConstructible.h"
// 通用平台杂项（FGenericPlatformMisc）。
#include "GenericPlatform/GenericPlatformMisc.h"
// 平台杂项（FPlatformMisc）。
#include "HAL/PlatformMisc.h"
// 日志详细级别（ELogVerbosity）。
#include "Logging/LogVerbosity.h"
// 变参宏（UE_LOG 可变参数）。
#include "Misc/VarArgs.h"
// 通用平台字符串比较（Stricmp）。
#include "GenericPlatform/GenericPlatformStricmp.h"
// 通用平台字符串操作（Strcpy / Strlen）。
#include "GenericPlatform/GenericPlatformString.h"
// 平台字符串（FPlatformString）。
#include "HAL/PlatformString.h"
// 通用平台堆栈回溯（FGenericPlatformStackWalk）。
#include "GenericPlatform/GenericPlatformStackWalk.h"
// 平台堆栈回溯（FPlatformStackWalk）。
#include "HAL/PlatformStackWalk.h"
// 通用平台数学（Sqrt / Sin / Cos）。
#include "GenericPlatform/GenericPlatformMath.h"
// 平台数学（FPlatformMath）。
#include "HAL/PlatformMath.h"
// 通用平台命名管道（FGenericPlatformNamedPipe）。
#include "GenericPlatform/GenericPlatformNamedPipe.h"
// 平台命名管道（FPlatformNamedPipe）。
#include "HAL/PlatformNamedPipe.h"
// 通用平台时间（FPlatformTime）。
#include "GenericPlatform/GenericPlatformTime.h"
// 平台时间（FPlatformTime）。
#include "HAL/PlatformTime.h"
// 通用平台进程（FPlatformProcess）。
#include "GenericPlatform/GenericPlatformProcess.h"
// 平台进程（FPlatformProcess）。
#include "HAL/PlatformProcess.h"
// 通用平台互斥量（FGenericPlatformMutex）。
#include "GenericPlatform/GenericPlatformMutex.h"
// 临界区（FCriticalSection）。
#include "HAL/CriticalSection.h"
// 通用平台原子操作。
#include "GenericPlatform/GenericPlatformAtomics.h"
// 平台原子操作（FPlatformAtomics）。
#include "HAL/PlatformAtomics.h"
// 通用平台线程局部存储。
#include "GenericPlatform/GenericPlatformTLS.h"
// 平台线程局部存储（FPlatformTLS）。
#include "HAL/PlatformTLS.h"
// 通用平台文件系统（IPlatformFile）。
#include "GenericPlatform/GenericPlatformFile.h"
// 平台文件（FPlatformFile）。
#include "HAL/PlatformFile.h"
// 通用平台亲和性（线程绑核）。
#include "GenericPlatform/GenericPlatformAffinity.h"
// 平台亲和性（FPlatformAffinity）。
#include "HAL/PlatformAffinity.h"
// 平台 include 汇总（各平台特有的系统头）。
#include "HAL/PlatformIncludes.h"
// 平台文件管理器（FPlatformFileManager）。
#include "HAL/PlatformFileManager.h"
// 断言宏（check / ensure）。
#include "Misc/AssertionMacros.h"
// 内存操作（FMemory::Malloc / Free）。
#include "HAL/UnrealMemory.h"
// 指针可转换判断。
#include "Templates/PointerIsConvertibleFromTo.h"
// 对齐模板（Align）。
#include "Templates/AlignmentTemplates.h"
// 移除引用（TRemoveReference<T>）。
#include "Templates/RemoveReference.h"
// 整型常量（TIntegralConstant）。
#include "Templates/IntegralConstant.h"
// 类判断（TIsClass<T>）。
#include "Templates/IsClass.h"
// 类型兼容字节（TTypeCompatibleBytes<T>）。
#include "Templates/TypeCompatibleBytes.h"
// 连续容器判断（TIsContiguousContainer<T>）。
#include "Traits/IsContiguousContainer.h"
// C 字符串工具（FCString）。
#include "Misc/CString.h"
// 强类型枚举判断（TIsEnumClass<T>）。
#include "Templates/IsEnumClass.h"
// 通用平台属性（FGenericPlatformProperties）。
#include "GenericPlatform/GenericPlatformProperties.h"
// 平台属性（FPlatformProperties）。
#include "HAL/PlatformProperties.h"
// 引擎版本基类（FEngineVersionBase）。
#include "Misc/EngineVersionBase.h"
// 文本命名空间前置声明（FTextNamespace）。
#include "Internationalization/TextNamespaceFwd.h"
// 小于比较（TLess<T>）。
#include "Templates/Less.h"
// 大于比较（TGreater<T>）。
#include "Templates/Greater.h"
// 日志分类（DECLARE_LOG_CATEGORY_EXTERN）。
#include "Logging/LogCategory.h"
// 日志宏（UE_LOG）。
#include "Logging/LogMacros.h"
// 引擎内置名字（NAME_None）。
#include "UObject/UnrealNames.h"
// 输出设备（FOutputDevice）。
#include "Misc/OutputDevice.h"
// 消息对话框（FMessageDialog）。
#include "Misc/MessageDialog.h"
// 控制台命令执行器（FExec）。
#include "Misc/Exec.h"
// 数值极限（TNumericLimits<T>）。
#include "Math/NumericLimits.h"
// 数学工具（Clamp / Lerp / Abs）。
#include "Math/UnrealMathUtility.h"
// UE 类型萃取（TIsSame / TIsDerivedFrom）。
#include "Templates/UnrealTypeTraits.h"
// UE 模板工具（MoveTemp / Forward / Swap）。
#include "Templates/UnrealTemplate.h"
// 类型退化（TDecay<T>）。
#include "Templates/Decay.h"
// 调用模板（Invoke）。
#include "Templates/Invoke.h"
// 函数对象（TFunction<T>）。
#include "Templates/Function.h"
// 内存操作模板（DefaultConstructItems）。
#include "Templates/MemoryOps.h"

// Core 基础宏（STUBBED / CPP / STRUCTCPP）。
#include "Misc/CoreDefines.h"

// Container forward declarations
// 分节注释：容器前置声明。
// 容器分配策略（FDefaultAllocator / TInlineAllocator）。
#include "Containers/ContainerAllocationPolicies.h"

// UObject 层级前置声明。
#include "UObject/UObjectHierarchyFwd.h"

// 全局量（GLog / GError / GIsEditor / GIsServer）。
#include "CoreGlobals.h"

/*----------------------------------------------------------------------------
	Includes.
----------------------------------------------------------------------------*/
/* 分节注释：下面是「正式 include」区（第 106~314 行，200 个）。 */

// 文件管理器（IFileManager）。
#include "HAL/FileManager.h"
// 作用域调试信息（FScopedDebugInfo）。
#include "ProfilingDebugging/ScopedDebugInfo.h"
// 模块化特性接口（IModularFeature）。
#include "Features/IModularFeature.h"
// 外部性能分析器（FExternalProfiler）。
#include "ProfilingDebugging/ExternalProfiler.h"
// 内存分配基类（FMalloc）。
#include "HAL/MemoryBase.h"
// 字节序转换（ByteSwap）。
#include "Misc/ByteSwap.h"
// 压缩（FCompression）。
#include "Misc/Compression.h"
// 字符串工具（FStringUtility）。
#include "Misc/StringUtility.h"
// 解析工具（FParse）。
#include "Misc/Parse.h"
// 字符串编码转换（TCHAR_TO_UTF8）。
#include "Containers/StringConv.h"
// CRC 校验（FCrc）。
#include "Misc/Crc.h"
// 对象版本（FObjectVersion）。
#include "UObject/ObjectVersion.h"
// 类型哈希（GetTypeHash）。
#include "Templates/TypeHash.h"
// 枚举当字节用（TEnumAsByte<T>）。
#include "Containers/EnumAsByte.h"
// 序列化归档（FArchive）。
#include "Serialization/Archive.h"
// 归档代理（FArchiveProxy）。
#include "Serialization/ArchiveProxy.h"
// 名字转字符串归档（FNameAsStringProxyArchive）。
#include "Serialization/NameAsStringProxyArchive.h"
// 排序（Sort / StableSort）。
#include "Templates/Sorting.h"
// 动态数组（TArray<T>）。
#include "Containers/Array.h"
// 脚本数组（FScriptArray）。
#include "Containers/ScriptArray.h"
// MRU 数组（TMRUArray<T>，最近使用优先）。
#include "Containers/MRUArray.h"
// 间接数组（TIndirectArray<T>）。
#include "Containers/IndirectArray.h"
// 事务接口（ITransaction）。
#include "Misc/ITransaction.h"
// 数组构建器（TArrayBuilder<T>）。
#include "Containers/ArrayBuilder.h"
// 位数组（TBitArray）。
#include "Containers/BitArray.h"
// 位读取器（FBitReader）。
#include "Serialization/BitReader.h"
// 位写入器（FBitWriter）。
#include "Serialization/BitWriter.h"
// 稀疏数组（TSparseArray）。
#include "Containers/SparseArray.h"
// 字符串类（FString）。
#include "Containers/UnrealString.h"
// 名字类型（FName）。
#include "UObject/NameTypes.h"
// 整数点（FIntPoint）。
#include "Math/IntPoint.h"
// 结构体构建器（TStructBuilder）。
#include "Misc/StructBuilder.h"
// 反转算法（Algo::Reverse）。
#include "Algo/Reverse.h"
// 事件（FEvent）。
#include "HAL/Event.h"
// 作用域事件（FScopedEvent）。
#include "Misc/ScopedEvent.h"
// 可运行对象（FRunnable）。
#include "HAL/Runnable.h"
// 可运行线程（FRunnableThread）。
#include "HAL/RunnableThread.h"
// 单线程事件（FSingleThreadEvent）。
#include "Misc/SingleThreadEvent.h"
// 单线程可运行（FSingleThreadRunnable）。
#include "Misc/SingleThreadRunnable.h"
// 线程管理器（FThreadManager）。
#include "HAL/ThreadManager.h"
// 队列工作接口（IQueuedWork）。
#include "Misc/IQueuedWork.h"
// 线程池（FQueuedThreadPool）。
#include "Misc/QueuedThreadPool.h"
// 线程安全计数器（FThreadSafeCounter）。
#include "HAL/ThreadSafeCounter.h"
// 64 位线程安全计数器（FThreadSafeCounter64）。
#include "HAL/ThreadSafeCounter64.h"
// 线程安全布尔（FThreadSafeBool）。
#include "HAL/ThreadSafeBool.h"
// 空计数器（FNoopCounter）。
#include "Misc/NoopCounter.h"
// 作用域锁（FScopeLock）。
#include "Misc/ScopeLock.h"
// TLS 自动清理（FTlsAutoCleanup）。
#include "HAL/TlsAutoCleanup.h"
// 线程单例（FThreadSingleton）。
#include "HAL/ThreadSingleton.h"
// 数组视图（TArrayView<T>）。
#include "Containers/ArrayView.h"
// Core 杂项（FCoreMisc）。
#include "Misc/CoreMisc.h"
// 命令行解析（FCommandLine）。
#include "Misc/CommandLine.h"
// 文件助手（FFileHelper）。
#include "Misc/FileHelper.h"
// 路径工具（FPaths）。
#include "Misc/Paths.h"
// 静态数组（TStaticArray<T, N>）。
#include "Containers/StaticArray.h"
// 静态位数组（TStaticBitArray<N>）。
#include "Containers/StaticBitArray.h"
// 集合（TSet<T>）。
#include "Containers/Set.h"
// 映射（TMap<K, V>）。
#include "Containers/Map.h"
// 映射构建器（TMapBuilder<K, V>）。
#include "Containers/MapBuilder.h"
// 双向链表（TDoubleLinkedList<T>）。
#include "Containers/List.h"
// 资源数组基类（FResourceArrayInterface）。
#include "Containers/ResourceArray.h"
// 动态 RHI 资源数组（FDynamicRHIResourceArray）。
#include "Containers/DynamicRHIResourceArray.h"
// 引用计数（FRefCountedObject）。
#include "Templates/RefCounting.h"
// 弱对象指针模板（TWeakObjectPtr<T>）。
#include "UObject/WeakObjectPtrTemplates.h"
// 脚本委托（FScriptDelegate）。
#include "UObject/ScriptDelegates.h"
// 委托设置（UE_DELEGATE_... 宏）。
#include "Delegates/DelegateSettings.h"
// 共享指针（TSharedPtr / TSharedRef）。
#include "Templates/SharedPointer.h"
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
// 委托（TDelegate<T>）。
#include "Delegates/Delegate.h"
// 线程基类（FThreadingBase）。
#include "HAL/ThreadingBase.h"
// 文化指针（FCulturePtr）。
#include "Internationalization/CulturePointer.h"
// 文本本地化管理器（FTextLocalizationManager）。
#include "Internationalization/TextLocalizationManager.h"
// 文本数据接口（ITextData）。
#include "Internationalization/ITextData.h"
// 数组判断（TIsArray<T>）。
#include "Templates/IsArray.h"
// 移除数组维度（TRemoveExtent<T>）。
#include "Templates/RemoveExtent.h"
// 独占指针（TUniquePtr<T>）。
#include "Templates/UniquePtr.h"
// 本地化文本（FText）。
#include "Internationalization/Text.h"
// 文本本地化全局量（GTextLocalizationManager）。
#include "Internationalization/TextLocalizationManagerGlobals.h"
// 独占对象（TUniqueObj<T>）。
#include "Templates/UniqueObj.h"
// 国际化（FInternationalization）。
#include "Internationalization/Internationalization.h"
// 文化（FCulture）。
#include "Internationalization/Culture.h"
// GUID（FGuid）。
#include "Misc/Guid.h"
// 网络 GUID（FNetworkGUID）。
#include "Misc/NetworkGuid.h"
// 颜色（FColor）。
#include "Math/Color.h"
// 颜色列表（GColorList）。
#include "Math/ColorList.h"
// 整数向量（FIntVector）。
#include "Math/IntVector.h"
// 二维向量（FVector2D）。
#include "Math/Vector2D.h"
// 整数矩形（FIntRect）。
#include "Math/IntRect.h"
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
// 平面（FPlane）。
#include "Math/Plane.h"
// 球（FSphere）。
#include "Math/Sphere.h"
// 胶囊体（FCapsuleShape）。
#include "Math/CapsuleShape.h"
// 旋转体（FRotator）。
#include "Math/Rotator.h"
// 范围边界（TRangeBound<T>）。
#include "Math/RangeBound.h"
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
// 插值曲线点（FInterpCurvePoint<T>）。
#include "Math/InterpCurvePoint.h"
// 插值曲线（FInterpCurve<T>）。
#include "Math/InterpCurve.h"
// 曲线编辑器接口（FCurveEdInterface）。
#include "Math/CurveEdInterface.h"
// 32 位浮点（FFloat32）。
#include "Math/Float32.h"
// 16 位浮点（FFloat16）。
#include "Math/Float16.h"
// 16 位浮点颜色（FFloat16Color）。
#include "Math/Float16Color.h"
// 半精度二维向量（FVector2DHalf）。
#include "Math/Vector2DHalf.h"
// 变换（FTransform）。
#include "Math/Transform.h"
// 二维凸包（FConvexHull2D）。
#include "Math/ConvexHull2d.h"
// 数学总表（FMath 完整定义）。
#include "Math/UnrealMath.h"
// 球谐函数数学（FSHVector）。
#include "Math/SHMath.h"
// 随机数流（FRandomStream）。
#include "Math/RandomStream.h"
// 日志抑制接口（FLogSuppressionInterface）。
#include "Logging/LogSuppressionInterface.h"
// 作用域日志分类覆盖（FLogScopedCategoryAndVerbosityOverride）。
#include "Logging/LogScopedCategoryAndVerbosityOverride.h"
// 输出设备汇总（FOutputDevices）。
#include "HAL/OutputDevices.h"
// 输出重定向器（FOutputDeviceRedirector）。
#include "Misc/OutputDeviceRedirector.h"
// 空输出设备（FOutputDeviceNull）。
#include "Misc/OutputDeviceNull.h"
// 内存输出设备（FOutputDeviceMemory）。
#include "Misc/OutputDeviceMemory.h"
// 文件输出设备（FOutputDeviceFile）。
#include "Misc/OutputDeviceFile.h"
// 调试输出设备（FOutputDeviceDebug）。
#include "Misc/OutputDeviceDebug.h"
// 归档输出设备包装（FOutputDeviceArchiveWrapper）。
#include "Misc/OutputDeviceArchiveWrapper.h"
// 错误输出设备（FOutputDeviceError）。
#include "Misc/OutputDeviceError.h"
// ANSI 错误输出设备（FOutputDeviceAnsiError）。
#include "Misc/OutputDeviceAnsiError.h"
// 缓冲输出设备（FBufferedOutputDevice）。
#include "Misc/BufferedOutputDevice.h"
// 作用域日志详细级别覆盖（FLogScopedVerbosityOverride）。
#include "Logging/LogScopedVerbosityOverride.h"
// 统计杂项（SCOPE_SECONDS_COUNTER）。
#include "Stats/StatsMisc.h"
// 无锁链表（TLockFreeList）。
#include "Containers/LockFreeList.h"
// 无锁固定大小分配器（TLockFreeFixedSizeAllocator）。
#include "Containers/LockFreeFixedSizeAllocator.h"
// 分块数组（TChunkedArray<T>）。
#include "Containers/ChunkedArray.h"
// 统计（DECLARE_STATS_GROUP）。
#include "Stats/Stats.h"
// Core 统计（DECLARE_CYCLE_STAT）。
#include "Misc/CoreStats.h"
// 时间守卫（FTimeGuard）。
#include "Misc/TimeGuard.h"
// 内存栈（FMemStack）。
#include "Misc/MemStack.h"
// 异步工作（FAsyncWork）。
#include "Async/AsyncWork.h"
// 内存归档（FMemoryArchive）。
#include "Serialization/MemoryArchive.h"
// 内存写入器（FMemoryWriter）。
#include "Serialization/MemoryWriter.h"
// 大内存写入器（FLargeMemoryWriter）。
#include "Serialization/LargeMemoryWriter.h"
// 大内存读取器（FLargeMemoryReader）。
#include "Serialization/LargeMemoryReader.h"
// 缓冲归档（FBufferArchive）。
#include "Serialization/BufferArchive.h"
// 内存读取器（FMemoryReader）。
#include "Serialization/MemoryReader.h"
// 数组读取器（FArrayReader）。
#include "Serialization/ArrayReader.h"
// 数组写入器（FArrayWriter）。
#include "Serialization/ArrayWriter.h"
// 缓冲读取器（FBufferReader）。
#include "Serialization/BufferReader.h"
// 缓冲写入器（FBufferWriter）。
#include "Serialization/BufferWriter.h"
// 变体（FVariant）。
#include "Misc/Variant.h"
// 通配符字符串（FWildcardString）。
#include "Misc/WildcardString.h"
// 环形缓冲（TCircularBuffer<T>）。
#include "Containers/CircularBuffer.h"
// 环形队列（TCircularQueue<T>）。
#include "Containers/CircularQueue.h"
// 队列（TQueue<T>）。
#include "Containers/Queue.h"
// 定时器（FTicker）。
#include "Containers/Ticker.h"
// 性能分析助手（SCOPE_CYCLE_COUNTER）。
#include "ProfilingDebugging/ProfilingHelpers.h"
// 配置缓存（GConfig）。
#include "Misc/ConfigCacheIni.h"
// 控制台变量管理器（IConsoleManager）。
#include "HAL/IConsoleManager.h"
// 输出设备助手（FOutputDeviceHelper）。
#include "Misc/OutputDeviceHelper.h"
// 慢任务栈（FSlowTaskStack）。
#include "Misc/SlowTaskStack.h"
// 反馈上下文（FFeedbackContext）。
#include "Misc/FeedbackContext.h"
// 慢任务（FSlowTask）。
#include "Misc/SlowTask.h"
// 作用域慢任务（FScopedSlowTask）。
#include "Misc/ScopedSlowTask.h"
// 自动化测试（IMPLEMENT_SIMPLE_AUTOMATION_TEST）。
#include "Misc/AutomationTest.h"
// 作用域回调（TScopedCallback）。
#include "Templates/ScopedCallback.h"
// Core 委托（FCoreDelegates）。
#include "Misc/CoreDelegates.h"
// 回调设备（FCallbackDevice）。
#include "Misc/CallbackDevice.h"
// 对象缩略图（FObjectThumbnail）。
#include "Misc/ObjectThumbnail.h"
// 本地时间戳目录访问器（FLocalTimestampDirectoryVisitor）。
#include "Misc/LocalTimestampDirectoryVisitor.h"
// 自定义版本（FCustomVersionRegistry）。
#include "Serialization/CustomVersion.h"
// 蓝图对象版本（FBlueprintsObjectVersion）。
#include "UObject/BlueprintsObjectVersion.h"
// 构建对象版本（FBuildObjectVersion）。
#include "UObject/BuildObjectVersion.h"
// Core 对象版本（FCoreObjectVersion）。
#include "UObject/CoreObjectVersion.h"
// 编辑器对象版本（FEditorObjectVersion）。
#include "UObject/EditorObjectVersion.h"
// 框架对象版本（FFrameworkObjectVersion）。
#include "UObject/FrameworkObjectVersion.h"
// 移动对象版本（FMobileObjectVersion）。
#include "UObject/MobileObjectVersion.h"
// 网络对象版本（FNetworkingObjectVersion）。
#include "UObject/NetworkingObjectVersion.h"
// 在线对象版本（FOnlineObjectVersion）。
#include "UObject/OnlineObjectVersion.h"
// 物理对象版本（FPhysicsObjectVersion）。
#include "UObject/PhysicsObjectVersion.h"
// 平台对象版本（FPlatformObjectVersion）。
#include "UObject/PlatformObjectVersion.h"
// 渲染对象版本（FRenderingObjectVersion）。
#include "UObject/RenderingObjectVersion.h"
// 序列器对象版本（FSequencerObjectVersion）。
#include "UObject/SequencerObjectVersion.h"
// VR 对象版本（FVRobjectVersion）。
#include "UObject/VRObjectVersion.h"
// 应用信息（FApp）。
#include "Misc/App.h"
// 控制台输出设备（FOutputDeviceConsole）。
#include "Misc/OutputDeviceConsole.h"
// 监控进程（FMonitoredProcess）。
#include "Misc/MonitoredProcess.h"
// 属性（TAttribute<T>）。
#include "Misc/Attribute.h"
// 可选值（TOptional<T>）。
#include "Misc/Optional.h"
// 枚举类标志（ENUM_CLASS_FLAGS）。
#include "Misc/EnumClassFlags.h"
// 枚举范围（TEnumRange<T>）。
#include "Misc/EnumRange.h"
// IWYU pragma: end_exports
// IWYU 指令：导出区到此结束。

// 兼容块：UE5.5 之前的 include 顺序（老代码依赖 IsTriviallyDestructible.h）。
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_5
#include "Templates/IsTriviallyDestructible.h"
#endif