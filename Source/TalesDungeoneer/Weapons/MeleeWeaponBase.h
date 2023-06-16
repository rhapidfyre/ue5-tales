
#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "Components/CapsuleComponent.h"

#include "MeleeWeaponBase.generated.h"

UCLASS(Blueprintable)
class TALESDUNGEONEER_API AMeleeWeaponBase : public AWeaponBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMeleeWeaponBase();

	virtual bool doAttack() override;
	
	UPROPERTY(EditAnywhere) UCapsuleComponent* mHitDetector;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure) TArray<AActor*> getOverlappingResources() const;
	
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void updateWeapon() override;
	virtual bool checkForHit() override;
	virtual void startAttackTimer() override;
	virtual void cancelAttackTimer() override;
	
private:
	UFUNCTION()
	void onMeleeWeaponHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
