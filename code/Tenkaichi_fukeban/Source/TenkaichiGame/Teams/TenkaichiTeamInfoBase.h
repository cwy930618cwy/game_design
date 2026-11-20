// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Info.h"
#include "System/GameplayTagStack.h"

#include "TenkaichiTeamInfoBase.generated.h"

namespace EEndPlayReason { enum Type : int; }

class UTenkaichiTeamCreationComponent;
class UTenkaichiTeamSubsystem;
class UObject;
struct FFrame;

// 队伍信息的基类：记录队号 + 队伍标签栈，并负责与队伍子系统注册
UCLASS(Abstract)
class ATenkaichiTeamInfoBase : public AInfo
{
	GENERATED_BODY()

public:
	ATenkaichiTeamInfoBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 获取这支队伍的队号
	int32 GetTeamId() const { return TeamId; }

	//~AActor 接口开始
	// 游戏开始时，尝试把自己的队伍信息注册到队伍子系统
	virtual void BeginPlay() override;
	// 结束时，若已注册则从队伍子系统注销
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~AActor 接口结束

protected:
	// 把"这支队伍"注册到指定的队伍子系统（由派生类可覆写的行为）
	virtual void RegisterWithTeamSubsystem(UTenkaichiTeamSubsystem* Subsystem);
	// 若已有队号，则尝试注册到世界里的队伍子系统
	void TryRegisterWithTeamSubsystem();

private:
	// 设置队号（服务器端；队号只能设置一次）
	void SetTeamId(int32 NewTeamId);

	// 客户端收到队号复制时触发，重新尝试注册
	UFUNCTION()
	void OnRep_TeamId();

public:
	// 允许"队伍创建组件"访问私有成员（建队时需要设队号/标签）
	friend UTenkaichiTeamCreationComponent;

	// 队伍的标签栈（复制到所有客户端）
	UPROPERTY(Replicated)
	FGameplayTagStackContainer TeamTags;

private:
	// 队号（只复制一次，客户端收到后触发 OnRep_TeamId）
	UPROPERTY(ReplicatedUsing=OnRep_TeamId)
	int32 TeamId;
};