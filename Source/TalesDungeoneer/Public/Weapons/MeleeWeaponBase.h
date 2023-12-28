
#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "Components/CapsuleComponent.h"

#include "MeleeWeaponBase.generated.h"

/*
 *	THIS IS OLD STOP USING IT
 *	Use "WeaponMeleeBase"
 *
 */
UCLASS(Blueprintable)
class TALESDUNGEONEER_API AMeleeWeaponBase : public AWeaponBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMeleeWeaponBase();

	virtual bool doAttack() override;

	// Used for detecting the actual hit(s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCapsuleComponent* mHitDetector;
	
	UFUNCTION()
	virtual void startHitDetection();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure) TArray<AActor*> getOverlappingResources();
	
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void UpdateWeapon() override;

	virtual void TargetHitByWeapon(AActor* HitActor);
	
private:
	
	UFUNCTION()
	void onMeleeWeaponHit(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY()
	FTimerHandle _DelayTimer;

	// Keeps track of targets hit by this weapon
	UPROPERTY()	TSet<AActor*> _HitTargets;
	
};
