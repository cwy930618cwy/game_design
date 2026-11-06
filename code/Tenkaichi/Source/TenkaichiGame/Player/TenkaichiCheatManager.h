// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "GameFramework/CheatManager.h"
#include "TenkaichiCheatManager.generated.h"

class UTenkaichiAbilitySystemComponent;


#ifndef USING_CHEAT_MANAGER
#define USING_CHEAT_MANAGER (1 && !UE_BUILD_SHIPPING)
#endif // #ifndef USING_CHEAT_MANAGER

DECLARE_LOG_CATEGORY_EXTERN(LogTenkaichiCheat, Log, All);


/**
 * UTenkaichiCheatManager
 *
 *	本工程使用的作弊管理器基类。
 */
UCLASS(config = Game, Within = PlayerController, MinimalAPI)
class UTenkaichiCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:

	UTenkaichiCheatManager();

	virtual void InitCheatManager() override;

	// 把文本同时输出到控制台和日志。
	static void CheatOutputText(const FString& TextToOutput);

	// 为当前玩家在服务器上执行一条作弊指令。
	UFUNCTION(exec)
	void Cheat(const FString& Msg);

	// 为所有玩家在服务器上执行一条作弊指令。
	UFUNCTION(exec)
	void CheatAll(const FString& Msg);

	// 开始下一场比赛（直接调用系统工具，无需经过服务器）。
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void PlayNextGame();

	UFUNCTION(Exec)
	virtual void ToggleFixedCamera();

	UFUNCTION(Exec)
	virtual void CycleDebugCameras();

	UFUNCTION(Exec)
	virtual void CycleAbilitySystemDebug();

	// 强制取消所有已激活的技能（用于排查技能打断问题）。
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void CancelActivatedAbilities();

	// 给当前玩家的技能系统组件加上一个动态标签。
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void AddTagToSelf(FString TagName);

	// 移除当前玩家技能系统组件上的一个动态标签。
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void RemoveTagFromSelf(FString TagName);

	// 对当前玩家造成指定伤害。
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void DamageSelf(float DamageAmount);

	// 对玩家当前瞄准的目标造成伤害。
	virtual void DamageTarget(float DamageAmount) override;

	// 对当前玩家进行一定治疗。
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void HealSelf(float HealAmount);

	// 对玩家当前瞄准的目标进行一定治疗。
	virtual void HealTarget(float HealAmount);

	// 对当前玩家造成足以致死的伤害。
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void DamageSelfDestruct();

	// 无敌模式（开启/关闭）。
	virtual void God() override;

	// 无限生命（生命值不低于 1）。
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void UnlimitedHealth(int32 Enabled = -1);

protected:

	virtual void EnableDebugCamera() override;
	virtual void DisableDebugCamera() override;
	bool InDebugCamera() const;

	virtual void EnableFixedCamera();
	virtual void DisableFixedCamera();
	bool InFixedCamera() const;

	void ApplySetByCallerDamage(UTenkaichiAbilitySystemComponent* TenkaichiASC, float DamageAmount);
	void ApplySetByCallerHeal(UTenkaichiAbilitySystemComponent* TenkaichiASC, float HealAmount);

	UTenkaichiAbilitySystemComponent* GetPlayerAbilitySystemComponent() const;
};