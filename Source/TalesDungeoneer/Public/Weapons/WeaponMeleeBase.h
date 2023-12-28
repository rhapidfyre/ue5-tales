#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"

#include "WeaponMeleeBase.generated.h"


/**
 * NEW CLASS
 * Base C++ class for all melee weapons. Contains all of the melee weapon logic.
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API AWeaponMeleeBase : public AWeaponBase
{
	GENERATED_BODY()
	
public:
	AWeaponMeleeBase() {};
	virtual bool doAttack() override;
	
protected:
	virtual void UpdateWeapon() override;
	virtual void BeginPlay() override;
	virtual void TargetHitByWeapon(AActor* HitActor);
	virtual void InitiateAttack();

	virtual bool GetIsAttackValid(AActor* HitActor) override;

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName SocketStartName = "trace_start";
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName SocketEndName = "trace_end";

private:

	UFUNCTION() void DoAttackTracing();

	// Manages the attack start, end & reset delay
	FTimerHandle AttackTimer_;

	float AttackTimerTickRate_ = 0.02f;

	int TicksDelayed_ = 0;
	int TimerTicksRemaining_ = 0;

	// Contains an array of all actors hit during the last/current attack
	TArray<AActor*> HitTargets_;
};
