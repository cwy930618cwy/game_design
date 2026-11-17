// 版权所有 Epic Games, Inc. 保留所有权利。
// 版权声明，引擎每个源文件第一行都是它，交付时译成中文。

// 头文件保护：保证同一个 .cpp 里多次 include 本文件时只展开一次。
#pragma once

// From Core:
// 分节注释：下面全是 Core 模块自己的头（252 个），按字母序排。

// 反转算法（Algo::Reverse）。
#include "Algo/Reverse.h"
// 异步工作（FAsyncWork，后台任务）。
#include "Async/AsyncWork.h"
// 异步结果（TFuture<T>，异步任务的返回值）。
#include "Async/Future.h"
// 并行循环（ParallelFor，多线程 for）。
#include "Async/ParallelFor.h"
// 任务图接口（FTaskGraphInterface，任务调度）。
#include "Async/TaskGraphInterfaces.h"
// 动态数组（TArray<T>）。
#include "Containers/Array.h"
// 数组视图（TArrayView<T>，不拥有数据的数组视图）。
#include "Containers/ArrayView.h"
// 位数组（TBitArray）。
#include "Containers/BitArray.h"
// 分块数组（TChunkedArray<T>）。
#include "Containers/ChunkedArray.h"
// 容器分配策略（FDefaultAllocator / TInlineAllocator）。
#include "Containers/ContainerAllocationPolicies.h"
// 容器前置声明（FString / FAnsiString / FUtf8String）。
#include "Containers/ContainersFwd.h"
// 动态 RHI 资源数组（FDynamicRHIResourceArray）。
#include "Containers/DynamicRHIResourceArray.h"
// 枚举当字节用（TEnumAsByte<T>）。
#include "Containers/EnumAsByte.h"
// 间接数组（TIndirectArray<T>，元素存在堆上）。
#include "Containers/IndirectArray.h"
// 双向链表（TDoubleLinkedList<T>）。
#include "Containers/List.h"
// 无锁固定大小分配器（TLockFreeFixedSizeAllocator）。
#include "Containers/LockFreeFixedSizeAllocator.h"
// 无锁链表（TLockFreeList）。
#include "Containers/LockFreeList.h"
// 映射（TMap<K, V>）。
#include "Containers/Map.h"
// 队列（TQueue<T>）。
#include "Containers/Queue.h"
// 资源数组基类（FResourceArrayInterface）。
#include "Containers/ResourceArray.h"
// 脚本数组（FScriptArray，蓝图用）。
#include "Containers/ScriptArray.h"
// 集合（TSet<T>）。
#include "Containers/Set.h"
// 稀疏数组（TSparseArray）。
#include "Containers/SparseArray.h"
// 静态数组（TStaticArray<T, N>，栈上固定大小）。
#include "Containers/StaticArray.h"
// 字符串编码转换（TCHAR_TO_UTF8 等）。
#include "Containers/StringConv.h"
// 定时器（FTicker，每帧回调）。
#include "Containers/Ticker.h"
// 字符串类（FString）。
#include "Containers/UnrealString.h"
// 前置声明总表（FName / FArchive / FText 等 34 个）。
#include "CoreFwd.h"
// 全局量（GLog / GError / GIsEditor / GIsServer）。
#include "CoreGlobals.h"
// 最小可用集（148 个常用头）。
#include "CoreMinimal.h"
// 基础类型层（平台宏 + int32 等）。
#include "CoreTypes.h"
// 委托（TDelegate<T>）。
#include "Delegates/Delegate.h"
// 委托基类（FDelegateBase）。
#include "Delegates/DelegateBase.h"
// 委托设置（UE_DELEGATE_... 宏）。
#include "Delegates/DelegateSettings.h"
// 委托实例接口（IDelegateInstance）。
#include "Delegates/IDelegateInstance.h"
// 整数序列（TIntegerSequence）。
#include "Delegates/IntegerSequence.h"
// 多播委托基类（FMulticastDelegateBase）。
#include "Delegates/MulticastDelegateBase.h"
// 模块化特性接口（IModularFeature）。
#include "Features/IModularFeature.h"
// 模块化特性注册表（IModularFeatures）。
#include "Features/IModularFeatures.h"
// 通用平台亲和性（线程绑核）。
#include "GenericPlatform/GenericPlatformAffinity.h"
// 通用平台原子操作。
#include "GenericPlatform/GenericPlatformAtomics.h"
// 通用平台文件系统（IPlatformFile）。
#include "GenericPlatform/GenericPlatformFile.h"
// 通用平台数学（Sqrt / Sin / Cos）。
#include "GenericPlatform/GenericPlatformMath.h"
// 通用平台内存（FPlatformMemory）。
#include "GenericPlatform/GenericPlatformMemory.h"
// 通用平台杂项（FPlatformMisc）。
#include "GenericPlatform/GenericPlatformMisc.h"
// 通用平台进程（FPlatformProcess，启动进程 / 取环境变量）。
#include "GenericPlatform/GenericPlatformProcess.h"
// 通用平台属性（FGenericPlatformProperties）。
#include "GenericPlatform/GenericPlatformProperties.h"
// 通用平台字符串比较（Stricmp）。
#include "GenericPlatform/GenericPlatformStricmp.h"
// 通用平台字符串操作（Strcpy / Strlen）。
#include "GenericPlatform/GenericPlatformString.h"
// 通用平台时间（FPlatformTime，取高精度时间）。
#include "GenericPlatform/GenericPlatformTime.h"
// 通用平台线程局部存储。
#include "GenericPlatform/GenericPlatformTLS.h"
// 临界区（FCriticalSection，线程锁）。
#include "HAL/CriticalSection.h"
// 事件（FEvent，线程同步）。
#include "HAL/Event.h"
// 文件管理器（IFileManager，读写文件 / 遍历目录）。
#include "HAL/FileManager.h"
// 控制台变量管理器（IConsoleManager，注册 cvar）。
#include "HAL/IConsoleManager.h"
// 内存分配基类（FMalloc）。
#include "HAL/MemoryBase.h"
// 平台亲和性（FPlatformAffinity，线程绑核）。
#include "HAL/PlatformAffinity.h"
// 平台原子操作（FPlatformAtomics）。
#include "HAL/PlatformAtomics.h"
// 平台 C 运行时（printf / malloc）。
#include "HAL/PlatformCrt.h"
// 平台数学（FPlatformMath）。
#include "HAL/PlatformMath.h"
// 平台内存（FPlatformMemory）。
#include "HAL/PlatformMemory.h"
// 平台杂项（FPlatformMisc）。
#include "HAL/PlatformMisc.h"
// 平台进程（FPlatformProcess）。
#include "HAL/PlatformProcess.h"
// 平台属性（FPlatformProperties）。
#include "HAL/PlatformProperties.h"
// 平台字符串（FPlatformString）。
#include "HAL/PlatformString.h"
// 平台时间（FPlatformTime）。
#include "HAL/PlatformTime.h"
// 平台线程局部存储（FPlatformTLS）。
#include "HAL/PlatformTLS.h"
// 可运行对象（FRunnable，线程体基类）。
#include "HAL/Runnable.h"
// 可运行线程（FRunnableThread）。
#include "HAL/RunnableThread.h"
// 线程安全布尔（FThreadSafeBool）。
#include "HAL/ThreadSafeBool.h"
// 线程安全计数器（FThreadSafeCounter）。
#include "HAL/ThreadSafeCounter.h"
// 线程单例（FThreadSingleton）。
#include "HAL/ThreadSingleton.h"
// TLS 自动清理（FTlsAutoCleanup）。
#include "HAL/TlsAutoCleanup.h"
// 内存操作（FMemory::Malloc / Free / Memcpy）。
#include "HAL/UnrealMemory.h"
// 文化指针（FCulturePtr）。
#include "Internationalization/CulturePointer.h"
// 可收集文本数据（FGatherableTextData）。
#include "Internationalization/GatherableTextData.h"
// 国际化（FInternationalization）。
#include "Internationalization/Internationalization.h"
// 国际化元数据（FInternationalizationMetaData）。
#include "Internationalization/InternationalizationMetadata.h"
// 本地化文本（FText）。
#include "Internationalization/Text.h"
// 文本本地化管理器（FTextLocalizationManager）。
#include "Internationalization/TextLocalizationManager.h"
// 文本命名空间前置声明（FTextNamespace）。
#include "Internationalization/TextNamespaceFwd.h"
// 日志分类（DECLARE_LOG_CATEGORY_EXTERN）。
#include "Logging/LogCategory.h"
// 日志宏（UE_LOG）。
#include "Logging/LogMacros.h"
// 日志详细级别（ELogVerbosity）。
#include "Logging/LogVerbosity.h"
// 令牌化消息（FTokenizedMessage，编辑器消息）。
#include "Logging/TokenizedMessage.h"
// 轴（FAxis）。
#include "Math/Axis.h"
// 轴对齐包围盒（FBox）。
#include "Math/Box.h"
// 二维包围盒（FBox2D）。
#include "Math/Box2D.h"
// 球包围盒（FBoxSphereBounds）。
#include "Math/BoxSphereBounds.h"
// 胶囊体（FCapsuleShape）。
#include "Math/CapsuleShape.h"
// 裁剪投影矩阵（FClipProjectionMatrix）。
#include "Math/ClipProjectionMatrix.h"
// 颜色（FColor）。
#include "Math/Color.h"
// 颜色列表（GColorList，预定义颜色）。
#include "Math/ColorList.h"
// 二维凸包（FConvexHull2D）。
#include "Math/ConvexHull2d.h"
// 曲线编辑器接口（FCurveEdInterface）。
#include "Math/CurveEdInterface.h"
// 边（FEdge）。
#include "Math/Edge.h"
// 16 位浮点（FFloat16）。
#include "Math/Float16.h"
// 16 位浮点颜色（FFloat16Color）。
#include "Math/Float16Color.h"
// 32 位浮点（FFloat32）。
#include "Math/Float32.h"
// 插值曲线（FInterpCurve<T>）。
#include "Math/InterpCurve.h"
// 插值曲线点（FInterpCurvePoint<T>）。
#include "Math/InterpCurvePoint.h"
// 区间（TInterval<T>）。
#include "Math/Interval.h"
// 整数点（FIntPoint）。
#include "Math/IntPoint.h"
// 整数矩形（FIntRect）。
#include "Math/IntRect.h"
// 整数向量（FIntVector）。
#include "Math/IntVector.h"
// 逆旋转矩阵（FInverseRotationMatrix）。
#include "Math/InverseRotationMatrix.h"
// 矩阵（FMatrix）。
#include "Math/Matrix.h"
// 镜像矩阵（FMirrorMatrix）。
#include "Math/MirrorMatrix.h"
// 数值极限（TNumericLimits<T>）。
#include "Math/NumericLimits.h"
// 有向包围盒（FOrientedBox）。
#include "Math/OrientedBox.h"
// 正交投影矩阵（FOrthoMatrix）。
#include "Math/OrthoMatrix.h"
// 透视投影矩阵（FPerspectiveMatrix）。
#include "Math/PerspectiveMatrix.h"
// 平面（FPlane）。
#include "Math/Plane.h"
// 四元数（FQuat）。
#include "Math/Quat.h"
// 四元数旋转平移矩阵（FQuatRotationTranslationMatrix）。
#include "Math/QuatRotationTranslationMatrix.h"
// 随机数流（FRandomStream）。
#include "Math/RandomStream.h"
// 范围（TRange<T>）。
#include "Math/Range.h"
// 范围边界（TRangeBound<T>）。
#include "Math/RangeBound.h"
// 范围集合（TRangeSet<T>）。
#include "Math/RangeSet.h"
// 绕点旋转矩阵（FRotationAboutPointMatrix）。
#include "Math/RotationAboutPointMatrix.h"
// 旋转矩阵（FRotationMatrix）。
#include "Math/RotationMatrix.h"
// 旋转平移矩阵（FRotationTranslationMatrix）。
#include "Math/RotationTranslationMatrix.h"
// 旋转体（FRotator）。
#include "Math/Rotator.h"
// 缩放矩阵（FScaleMatrix）。
#include "Math/ScaleMatrix.h"
// 缩放旋转平移矩阵（FScaleRotationTranslationMatrix）。
#include "Math/ScaleRotationTranslationMatrix.h"
// 球谐函数数学（FSHVector，光照用）。
#include "Math/SHMath.h"
// 球（FSphere）。
#include "Math/Sphere.h"
// 变换（FTransform）。
#include "Math/Transform.h"
// 变换微积分（TransformCalculus）。
#include "Math/TransformCalculus.h"
// 二维变换微积分（TransformCalculus2D）。
#include "Math/TransformCalculus2D.h"
// 平移矩阵（FTranslationMatrix）。
#include "Math/TranslationMatrix.h"
// 双向量（FTwoVectors）。
#include "Math/TwoVectors.h"
// 数学总表（FMath 完整定义）。
#include "Math/UnrealMath.h"
// 数学工具（Clamp / Lerp / Abs）。
#include "Math/UnrealMathUtility.h"
// 三维向量（FVector）。
#include "Math/Vector.h"
// 二维向量（FVector2D）。
#include "Math/Vector2D.h"
// 半精度二维向量（FVector2DHalf）。
#include "Math/Vector2DHalf.h"
// 四维向量（FVector4）。
#include "Math/Vector4.h"
// 向量寄存器（VectorRegister，SIMD）。
#include "Math/VectorRegister.h"
// 应用信息（FApp，取项目名 / 版本）。
#include "Misc/App.h"
// 断言宏（check / ensure）。
#include "Misc/AssertionMacros.h"
// 属性（TAttribute<T>，可绑定值）。
#include "Misc/Attribute.h"
// 自动化测试（IMPLEMENT_SIMPLE_AUTOMATION_TEST）。
#include "Misc/AutomationTest.h"
// 缓冲输出设备（FBufferedOutputDevice）。
#include "Misc/BufferedOutputDevice.h"
// 字节序转换（ByteSwap）。
#include "Misc/ByteSwap.h"
// 字符类型（TCHAR）。
#include "Misc/Char.h"
// 命令行解析（FCommandLine）。
#include "Misc/CommandLine.h"
// 编译结果（ECompilationResult）。
#include "Misc/CompilationResult.h"
// 压缩（FCompression，zlib / Oodle）。
#include "Misc/Compression.h"
// 配置缓存（GConfig，读写 ini）。
#include "Misc/ConfigCacheIni.h"
// Core 委托（FCoreDelegates，引擎生命周期回调）。
#include "Misc/CoreDelegates.h"
// Core 杂项（FCoreMisc）。
#include "Misc/CoreMisc.h"
// Core 统计（DECLARE_CYCLE_STAT）。
#include "Misc/CoreStats.h"
// CRC 校验（FCrc）。
#include "Misc/Crc.h"
// C 字符串工具（FCString）。
#include "Misc/CString.h"
// 日期时间（FDateTime）。
#include "Misc/DateTime.h"
// 引擎版本（FEngineVersion）。
#include "Misc/EngineVersion.h"
// 引擎版本基类（FEngineVersionBase）。
#include "Misc/EngineVersionBase.h"
// 枚举类标志（ENUM_CLASS_FLAGS）。
#include "Misc/EnumClassFlags.h"
// 控制台命令执行器（FExec）。
#include "Misc/Exec.h"
// 反馈上下文（FFeedbackContext，进度条）。
#include "Misc/FeedbackContext.h"
// 文件助手（FFileHelper，读写文本文件）。
#include "Misc/FileHelper.h"
// 过滤器集合（TFilterCollection<T>）。
#include "Misc/FilterCollection.h"
// GUID（FGuid）。
#include "Misc/Guid.h"
// 过滤器接口（IFilter）。
#include "Misc/IFilter.h"
// 队列工作接口（IQueuedWork）。
#include "Misc/IQueuedWork.h"
// 内存栈（FMemStack，临时分配）。
#include "Misc/MemStack.h"
// 消息对话框（FMessageDialog）。
#include "Misc/MessageDialog.h"
// 网络 GUID（FNetworkGUID）。
#include "Misc/NetworkGuid.h"
// 空计数器（FNoopCounter）。
#include "Misc/NoopCounter.h"
// 对象缩略图（FObjectThumbnail）。
#include "Misc/ObjectThumbnail.h"
// 可选值（TOptional<T>）。
#include "Misc/Optional.h"
// 输出设备（FOutputDevice）。
#include "Misc/OutputDevice.h"
// 错误输出设备（FOutputDeviceError）。
#include "Misc/OutputDeviceError.h"
// 输出重定向器（FOutputDeviceRedirector）。
#include "Misc/OutputDeviceRedirector.h"
// 解析工具（FParse）。
#include "Misc/Parse.h"
// 路径工具（FPaths，拼路径 / 取文件名）。
#include "Misc/Paths.h"
// 线程池（FQueuedThreadPool）。
#include "Misc/QueuedThreadPool.h"
// 作用域事件（FScopedEvent）。
#include "Misc/ScopedEvent.h"
// 作用域锁（FScopeLock）。
#include "Misc/ScopeLock.h"
// 安全哈希（FSHA1 / FSHA256）。
#include "Misc/SecureHash.h"
// 单线程可运行（FSingleThreadRunnable）。
#include "Misc/SingleThreadRunnable.h"
// 慢任务（FSlowTask，进度条）。
#include "Misc/SlowTask.h"
// 慢任务栈（FSlowTaskStack）。
#include "Misc/SlowTaskStack.h"
// 结构体构建器（TStructBuilder）。
#include "Misc/StructBuilder.h"
// 时间跨度（FTimespan）。
#include "Misc/Timespan.h"
// 变参宏（UE_LOG 可变参数）。
#include "Misc/VarArgs.h"
// 模块样板（IMPLEMENT_MODULE 的辅助宏）。
#include "Modules/Boilerplate/ModuleBoilerplate.h"
// 模块接口（IModuleInterface）。
#include "Modules/ModuleInterface.h"
// 模块管理器（FModuleManager）。
#include "Modules/ModuleManager.h"
// 直方图（FHistogram，性能统计）。
#include "ProfilingDebugging/Histogram.h"
// 性能分析助手（SCOPE_CYCLE_COUNTER）。
#include "ProfilingDebugging/ProfilingHelpers.h"
// 资源大小（FResourceSizeEx）。
#include "ProfilingDebugging/ResourceSize.h"
// 序列化归档（FArchive）。
#include "Serialization/Archive.h"
// 归档代理（FArchiveProxy）。
#include "Serialization/ArchiveProxy.h"
// 位读取器（FBitReader）。
#include "Serialization/BitReader.h"
// 位写入器（FBitWriter）。
#include "Serialization/BitWriter.h"
// 缓冲读取器（FBufferReader）。
#include "Serialization/BufferReader.h"
// 自定义版本（FCustomVersionRegistry）。
#include "Serialization/CustomVersion.h"
// 内存归档（FMemoryArchive）。
#include "Serialization/MemoryArchive.h"
// 内存读取器（FMemoryReader）。
#include "Serialization/MemoryReader.h"
// 内存写入器（FMemoryWriter）。
#include "Serialization/MemoryWriter.h"
// 统计（DECLARE_STATS_GROUP）。
#include "Stats/Stats.h"
// 统计杂项（SCOPE_SECONDS_COUNTER）。
#include "Stats/StatsMisc.h"
// 对齐模板（Align）。
#include "Templates/AlignmentTemplates.h"
// 逻辑运算（TAnd / TOr / TNot）。
#include "Templates/AndOrNot.h"
// 类型退化（TDecay<T>）。
#include "Templates/Decay.h"
// 条件启用（TEnableIf<T>）。
#include "Templates/EnableIf.h"
// 函数对象（TFunction<T>）。
#include "Templates/Function.h"
// 大于比较（TGreater<T>）。
#include "Templates/Greater.h"
// 整型常量（TIntegralConstant）。
#include "Templates/IntegralConstant.h"
// 调用模板（Invoke）。
#include "Templates/Invoke.h"
// 算术类型判断（TIsArithmetic<T>）。
#include "Templates/IsArithmetic.h"
// 数组判断（TIsArray<T>）。
#include "Templates/IsArray.h"
// 类判断（TIsClass<T>）。
#include "Templates/IsClass.h"
// 强类型枚举判断（TIsEnumClass<T>）。
#include "Templates/IsEnumClass.h"
// 浮点判断（TIsFloatingPoint<T>）。
#include "Templates/IsFloatingPoint.h"
// 整型判断（TIsIntegral<T>）。
#include "Templates/IsIntegral.h"
// POD 判断（TIsPODType<T>）。
#include "Templates/IsPODType.h"
// 指针判断（TIsPointer<T>）。
#include "Templates/IsPointer.h"
// 有符号判断（TIsSigned<T>）。
#include "Templates/IsSigned.h"
// 可平凡拷贝赋值判断。
#include "Templates/IsTriviallyCopyAssignable.h"
// 可平凡拷贝构造判断。
#include "Templates/IsTriviallyCopyConstructible.h"
// 小于比较（TLess<T>）。
#include "Templates/Less.h"
// 内存操作模板（DefaultConstructItems）。
#include "Templates/MemoryOps.h"
// 指针可转换判断。
#include "Templates/PointerIsConvertibleFromTo.h"
// 引用计数（FRefCountedObject）。
#include "Templates/RefCounting.h"
// 移除数组维度（TRemoveExtent<T>）。
#include "Templates/RemoveExtent.h"
// 移除引用（TRemoveReference<T>）。
#include "Templates/RemoveReference.h"
// 作用域回调（TScopedCallback）。
#include "Templates/ScopedCallback.h"
// 共享指针（TSharedPtr / TSharedRef）。
#include "Templates/SharedPointer.h"
// 排序（Sort / StableSort）。
#include "Templates/Sorting.h"
// 元组（TTuple<T...>）。
#include "Templates/Tuple.h"
// 类型兼容字节（TTypeCompatibleBytes<T>）。
#include "Templates/TypeCompatibleBytes.h"
// 类型哈希（GetTypeHash）。
#include "Templates/TypeHash.h"
// 独占对象（TUniqueObj<T>）。
#include "Templates/UniqueObj.h"
// 独占指针（TUniquePtr<T>）。
#include "Templates/UniquePtr.h"
// UE 模板工具（MoveTemp / Forward / Swap）。
#include "Templates/UnrealTemplate.h"
// UE 类型萃取（TIsSame / TIsDerivedFrom）。
#include "Templates/UnrealTypeTraits.h"
// 值或错误（TValueOrError<T, E>）。
#include "Templates/ValueOrError.h"
// 连续容器判断（TIsContiguousContainer<T>）。
#include "Traits/IsContiguousContainer.h"
// 调试序列化标志（EDebugSerializationFlags）。
#include "UObject/DebugSerializationFlags.h"
// 名字类型（FName）。
#include "UObject/NameTypes.h"
// 对象版本（FObjectVersion）。
#include "UObject/ObjectVersion.h"
// 属性端口标志（EPropertyPortFlags）。
#include "UObject/PropertyPortFlags.h"
// 脚本委托（FScriptDelegate）。
#include "UObject/ScriptDelegates.h"
// 引擎内置名字（NAME_None）。
#include "UObject/UnrealNames.h"
// UObject 层级前置声明。
#include "UObject/UObjectHierarchyFwd.h"
// 弱对象指针模板（TWeakObjectPtr<T>）。
#include "UObject/WeakObjectPtrTemplates.h"

// 兼容块：UE5.5 之前的 include 顺序（老代码依赖 IsTriviallyDestructible.h）。
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_5
#include "Templates/IsTriviallyDestructible.h"
#endif

// 兼容块：UE5.6 之前的 include 顺序（老代码依赖数学表达式求值器）。
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "Math/BasicMathExpressionEvaluator.h"
#include "Misc/ExpressionParserTypes.h"
#endif