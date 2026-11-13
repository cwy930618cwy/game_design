// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CoreTypes.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "TenkaichiReplicationGraphTypes.h"
#include "TenkaichiReplicationGraphSettings.generated.h"

/**
 * Tenkaichi 复制图的默认设置
 */
UCLASS(config=Game, MinimalAPI)
class UTenkaichiReplicationGraphSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	// 构造函数：设置分类名 + 默认复制图类。
	UTenkaichiReplicationGraphSettings();

public:

	// 是否禁用复制图。
	UPROPERTY(config, EditAnywhere, Category = ReplicationGraph)
	bool bDisableReplicationGraph = true;

	// 默认复制图类。
	UPROPERTY(config, EditAnywhere, Category = ReplicationGraph, meta = (MetaClass = "/Script/TenkaichiGame.TenkaichiReplicationGraph"))
	FSoftClassPath DefaultReplicationGraphClass;

	// 是否启用 FastShared 路径（快速共享路径）。
	UPROPERTY(EditAnywhere, Category = FastSharedPath, meta = (ConsoleVariable = "Tenkaichi.RepGraph.EnableFastSharedPath"))
	bool bEnableFastSharedPath = true;

	// FastShared 移动更新用多少带宽。这个独立于 NetDriver 的目标带宽计算。
	UPROPERTY(EditAnywhere, Category = FastSharedPath, meta = (ForceUnits=Kilobytes, ConsoleVariable = "Tenkaichi.RepGraph.TargetKBytesSecFastSharedPath"))
	int32 TargetKBytesSecFastSharedPath = 10;

	// FastShared 路径的剔除距离百分比。
	UPROPERTY(EditAnywhere, Category = FastSharedPath, meta = (ConsoleVariable = "Tenkaichi.RepGraph.FastSharedPathCullDistPct"))
	float FastSharedPathCullDistPct = 0.80f;

	// 破坏信息的最大距离。
	UPROPERTY(EditAnywhere, Category = DestructionInfo, meta = (ForceUnits = cm, ConsoleVariable = "Tenkaichi.RepGraph.DestructInfo.MaxDist"))
	float DestructionInfoMaxDist = 30000.f;

	// 空间网格单元大小。
	UPROPERTY(EditAnywhere, Category=SpatialGrid, meta=(ForceUnits=cm, ConsoleVariable = "Tenkaichi.RepGraph.CellSize"))
	float SpatialGridCellSize = 10000.0f;

	// 复制空间的"最小 X"。这只是初始值，如果有 Actor 出现在这个范围之外，系统会重置自己。
	UPROPERTY(EditAnywhere, Category=SpatialGrid, meta=(ForceUnits=cm, ConsoleVariable = "Tenkaichi.RepGraph.SpatialBiasX"))
	float SpatialBiasX = -200000.0f;

	// 复制空间的"最小 Y"。这只是初始值，如果有 Actor 出现在这个范围之外，系统会重置自己。
	UPROPERTY(EditAnywhere, Category=SpatialGrid, meta=(ForceUnits=cm, ConsoleVariable = "Tenkaichi.RepGraph.SpatialBiasY"))
	float SpatialBiasY = -200000.0f;

	// 是否禁用空间重建。
	UPROPERTY(EditAnywhere, Category=SpatialGrid, meta = (ConsoleVariable = "Tenkaichi.RepGraph.DisableSpatialRebuilds"))
	bool bDisableSpatialRebuilds = true;

	// 把动态、空间化的 Actor 分散到多少个桶里。
	// 数字越高 = 桶越多 = 有效复制频率越低。
	// 这发生在单个 Actor 做它们自己的 NetUpdateFrequency 检查之前。
	UPROPERTY(EditAnywhere, Category = DynamicSpatialFrequency, meta = (ConsoleVariable = "Tenkaichi.RepGraph.DynamicActorFrequencyBuckets"))
	int32 DynamicActorFrequencyBuckets = 3;

	// 特定类的自定义设置数组。
	UPROPERTY(config, EditAnywhere, Category = ReplicationGraph)
	TArray<FRepGraphActorClassSettings> ClassSettings;
};