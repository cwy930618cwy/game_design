// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入低层内存追踪器：ENABLE_LOW_LEVEL_MEM_TRACKER / LLM_ENABLED_STAT_TAGS 等宏。
#include "HAL/LowLevelMemTracker.h"
// 引入统计系统：DECLARE_STAT / DEFINE_STAT / DECLARE_STATS_GROUP。
#include "Stats/Stats.h"

// 如果启用了低层内存追踪且启用了统计系统。
#if ENABLE_LOW_LEVEL_MEM_TRACKER && STATS
// 说明：LLM 统计实现宏；这些宏在 LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG 里公开使用，在 LLM 实现里私有使用。
/**
 * LLM 统计实现宏；这些宏在 LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG 里公开使用，在 LLM 实现里私有使用
 */
// DECLARE_LLM_MEMORY_STAT：声明并定义一个 LLM 内存统计计数器。
	#define DECLARE_LLM_MEMORY_STAT(CounterName,StatId,GroupId) \
// 声明统计（int64 类型，无标志，物理 LLM 内存计数器）。
		DECLARE_STAT(CounterName,StatId,GroupId,EStatDataType::ST_int64, EStatFlags::None, FPlatformMemory::MCR_PhysicalLLM); \
// 定义统计（static，本编译单元内可见）。
		static DEFINE_STAT(StatId)
// DECLARE_LLM_MEMORY_STAT_EXTERN：声明一个外部定义的 LLM 内存统计计数器。
	#define DECLARE_LLM_MEMORY_STAT_EXTERN(CounterName,StatId,GroupId, API) \
// 声明统计（int64 类型，无标志，物理 LLM 内存计数器）。
		DECLARE_STAT(CounterName,StatId,GroupId,EStatDataType::ST_int64, EStatFlags::None, FPlatformMemory::MCR_PhysicalLLM); \
// 外部定义（带 API 导出标记）。
		extern API DEFINE_STAT(StatId);

// 声明统计组 LLM FULL（完整 LLM 数据）。
	DECLARE_STATS_GROUP(TEXT("LLM FULL"), STATGROUP_LLMFULL, STATCAT_Advanced);
// 声明统计组 LLM Platform（平台层数据）。
	DECLARE_STATS_GROUP(TEXT("LLM Platform"), STATGROUP_LLMPlatform, STATCAT_Advanced);
// 声明统计组 LLM Summary（汇总数据）。
	DECLARE_STATS_GROUP(TEXT("LLM Summary"), STATGROUP_LLM, STATCAT_Advanced);
// 声明统计组 LLM Overhead（LLM 自身开销）。
	DECLARE_STATS_GROUP(TEXT("LLM Overhead"), STATGROUP_LLMOverhead, STATCAT_Advanced);
// 声明统计组 LLM Assets（资产数据）。
	DECLARE_STATS_GROUP(TEXT("LLM Assets"), STATGROUP_LLMAssets, STATCAT_Advanced);

// 说明：LLM 汇总统计，被 ELLMTagNames 引用。
	/*
	* LLM 汇总统计，被 ELLMTagNames 引用
	*/
// Engine：引擎自身内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Engine"), STAT_EngineSummaryLLM, STATGROUP_LLM, CORE_API);
// Project：项目内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Project"), STAT_ProjectSummaryLLM, STATGROUP_LLM, CORE_API);
// Networking：网络内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Networking"), STAT_NetworkingSummaryLLM, STATGROUP_LLM, CORE_API);
// Audio：音频内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Audio"), STAT_AudioSummaryLLM, STATGROUP_LLM, CORE_API);
// Total：总追踪内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Total"), STAT_TrackedTotalSummaryLLM, STATGROUP_LLM, CORE_API);
// Meshes：网格内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Meshes"), STAT_MeshesSummaryLLM, STATGROUP_LLM, CORE_API);
// Physics：物理内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Physics"), STAT_PhysicsSummaryLLM, STATGROUP_LLM, CORE_API);
// PhysX：PhysX 物理内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("PhysX"), STAT_PhysXSummaryLLM, STATGROUP_LLM, CORE_API);
// Chaos：Chaos 物理内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Chaos"), STAT_ChaosSummaryLLM, STATGROUP_LLM, CORE_API);
// UObject：UObject 内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("UObject"), STAT_UObjectSummaryLLM, STATGROUP_LLM, CORE_API);
// Animation：动画内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Animation"), STAT_AnimationSummaryLLM, STATGROUP_LLM, CORE_API);
// StaticMesh：静态网格内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("StaticMesh"), STAT_StaticMeshSummaryLLM, STATGROUP_LLM, CORE_API);
// Materials：材质内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Materials"), STAT_MaterialsSummaryLLM, STATGROUP_LLM, CORE_API);
// Particles：粒子内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Particles"), STAT_ParticlesSummaryLLM, STATGROUP_LLM, CORE_API);
// Niagara：Niagara 特效内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Niagara"), STAT_NiagaraSummaryLLM, STATGROUP_LLM, CORE_API);
// UI：UI 内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("UI"), STAT_UISummaryLLM, STATGROUP_LLM, CORE_API);
// Navigation：导航内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Navigation"), STAT_NavigationSummaryLLM, STATGROUP_LLM, CORE_API);
// Textures：纹理内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Textures"), STAT_TexturesSummaryLLM, STATGROUP_LLM, CORE_API);
// MediaStreaming：媒体流内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("MediaStreaming"), STAT_MediaStreamingSummaryLLM, STATGROUP_LLM, CORE_API);

// 否则（未启用 LLM 或未启用统计）。
#else
// DECLARE_LLM_MEMORY_STAT：空宏。
	#define DECLARE_LLM_MEMORY_STAT(CounterName,StatId,GroupId)
// DECLARE_LLM_MEMORY_STAT_EXTERN：空宏。
	#define DECLARE_LLM_MEMORY_STAT_EXTERN(CounterName,StatId,GroupId, API)

// 条件编译结束。
#endif //ENABLE_LOW_LEVEL_MEM_TRACKER && STATS

// 说明：LLM 统计作用域宏（LLM 关闭或 LLM 统计标签关闭时都是空操作）。
/**
 * LLM 统计作用域宏（LLM 关闭或 LLM 统计标签关闭时都是空操作）
 */
// 如果启用了 LLM 且启用了 LLM 统计标签。
#if ENABLE_LOW_LEVEL_MEM_TRACKER && LLM_ENABLED_STAT_TAGS
// LLM_SCOPED_TAG_WITH_STAT：用统计名打一个 LLM 作用域标签。
#define LLM_SCOPED_TAG_WITH_STAT(Stat, Tracker) FLLMScope SCOPE_NAME(GET_STATFNAME(Stat), true /* bIsStatTag */, ELLMTagSet::None, Tracker);
// LLM_SCOPED_TAG_WITH_STAT_IN_SET：同上，但指定标签集。
#define LLM_SCOPED_TAG_WITH_STAT_IN_SET(Stat, Set, Tracker) FLLMScope SCOPE_NAME(GET_STATFNAME(Stat), true /* bIsStatTag */, Set, Tracker);
// LLM_SCOPED_TAG_WITH_STAT_NAME：用字符串名打标签。
#define LLM_SCOPED_TAG_WITH_STAT_NAME(StatName, Tracker) FLLMScope SCOPE_NAME(StatName, true /* bIsStatTag */, ELLMTagSet::None, Tracker);
// LLM_SCOPED_TAG_WITH_STAT_NAME_IN_SET：同上，但指定标签集。
#define LLM_SCOPED_TAG_WITH_STAT_NAME_IN_SET(StatName, Set, Tracker) FLLMScope SCOPE_NAME(StatName, true /* bIsStatTag */, Set, Tracker);
// LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG：声明 + 打一个平台层统计标签。
#define LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG(Stat) DECLARE_LLM_MEMORY_STAT(TEXT(#Stat), Stat, STATGROUP_LLMPlatform); LLM_SCOPED_TAG_WITH_STAT(Stat, ELLMTracker::Platform);
// LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG_IN_SET：同上，但指定标签集。
#define LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG_IN_SET(Stat, Set) DECLARE_LLM_MEMORY_STAT(TEXT(#Stat), Stat, STATGROUP_LLMPlatform); LLM_SCOPED_TAG_WITH_STAT_IN_SET(Stat, Set, ELLMTracker::Platform);
// LLM_SCOPED_SINGLE_STAT_TAG：声明 + 打一个完整层统计标签。
#define LLM_SCOPED_SINGLE_STAT_TAG(Stat) DECLARE_LLM_MEMORY_STAT(TEXT(#Stat), Stat, STATGROUP_LLMFULL); LLM_SCOPED_TAG_WITH_STAT(Stat, ELLMTracker::Default);
// LLM_SCOPED_SINGLE_STAT_TAG_IN_SET：同上，但指定标签集。
#define LLM_SCOPED_SINGLE_STAT_TAG_IN_SET(Stat, Set) DECLARE_LLM_MEMORY_STAT(TEXT(#Stat), Stat, STATGROUP_LLMFULL); LLM_SCOPED_TAG_WITH_STAT_IN_SET(Stat, Set, ELLMTracker::Default);
// LLM_SCOPED_PAUSE_TRACKING_WITH_STAT_AND_AMOUNT：暂停追踪并记录指定数量。
#define LLM_SCOPED_PAUSE_TRACKING_WITH_STAT_AND_AMOUNT(Stat, Amount, Tracker) FLLMPauseScope SCOPE_NAME(GET_STATFNAME(Stat), true /* bIsStatTag */, Amount, Tracker, ELLMAllocType::None);
// LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH：用对象路径名打动态标签。
#define LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH(Object, Set) LLM_SCOPE_DYNAMIC(FName(*Object->GetPathName()), \
// 续行：默认追踪器 + 标签集 + 动态标签构造器。
					ELLMTracker::Default, Set, FLLMDynamicTagConstructorStatString(Object->GetPathName()))
// LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH_FNAME：用 FName 路径打动态标签。
#define LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH_FNAME(ObjectPath, Set) LLM_SCOPE_DYNAMIC(ObjectPath, \
// 续行：默认追踪器 + 标签集 + 动态标签构造器。
					ELLMTracker::Default, Set, FLLMDynamicTagConstructorStatString((ObjectPath).ToString()))

// 说明：追踪资产时，每个资产后推送统计以更新线程。
// 追踪资产时，每个资产后推送统计以更新线程
// 说明：目前未使用，但以后可能用于优化。
// 目前未使用，但以后可能用于优化
// 如果允许资产标签。
#if LLM_ALLOW_ASSETS_TAGS
// 被注释掉的旧实现。
//#define LLM_PUSH_STATS_FOR_ASSET_TAGS() if (FLowLevelMemTracker::Get().IsTagSetActive(ELLMTagSet::Assets)) {}
// LLM_PUSH_STATS_FOR_ASSET_TAGS：空宏。
#define LLM_PUSH_STATS_FOR_ASSET_TAGS()
// 否则。
#else
// LLM_PUSH_STATS_FOR_ASSET_TAGS：空宏。
#define LLM_PUSH_STATS_FOR_ASSET_TAGS()
// 条件编译结束。
#endif

// 否则（未启用 LLM 或未启用统计标签）。
#else
// LLM_SCOPED_TAG_WITH_STAT：空宏。
#define LLM_SCOPED_TAG_WITH_STAT(...)
// LLM_SCOPED_TAG_WITH_STAT_IN_SET：空宏。
#define LLM_SCOPED_TAG_WITH_STAT_IN_SET(...)
// LLM_SCOPED_TAG_WITH_STAT_NAME：空宏。
#define LLM_SCOPED_TAG_WITH_STAT_NAME(...)
// LLM_SCOPED_TAG_WITH_STAT_NAME_IN_SET：空宏。
#define LLM_SCOPED_TAG_WITH_STAT_NAME_IN_SET(...)
// LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG：空宏。
#define LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG(...)
// LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG_IN_SET：空宏。
#define LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG_IN_SET(...)
// LLM_SCOPED_SINGLE_STAT_TAG：空宏。
#define LLM_SCOPED_SINGLE_STAT_TAG(...)
// LLM_SCOPED_SINGLE_STAT_TAG_IN_SET：空宏。
#define LLM_SCOPED_SINGLE_STAT_TAG_IN_SET(...)
// LLM_SCOPED_PAUSE_TRACKING_WITH_STAT_AND_AMOUNT：空宏。
#define LLM_SCOPED_PAUSE_TRACKING_WITH_STAT_AND_AMOUNT(...)
// LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH：空宏。
#define LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH(...)
// LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH_FNAME：空宏。
#define LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH_FNAME(...)
// LLM_PUSH_STATS_FOR_ASSET_TAGS：空宏。
#define LLM_PUSH_STATS_FOR_ASSET_TAGS()
// 条件编译结束。
#endif

// LLM_SCOPED_TAG_WITH_OBJECT_IN_SET：已废弃（5.3），改用 LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH。
#define LLM_SCOPED_TAG_WITH_OBJECT_IN_SET(Object, Set) UE_DEPRECATED_MACRO(5.3, "Use LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH instead") LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH(Object, Set);


