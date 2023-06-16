
#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"

#include "RangedWeaponBase.generated.h"

UCLASS(Blueprintable)
class TALESDUNGEONEER_API ARangedWeaponBase : public AWeaponBase
{
	GENERATED_BODY()

public:

	ARangedWeaponBase();

	UFUNCTION(BlueprintPure) int getAmmoRemaining() const { return mAmmoCount; }
	
	virtual bool doAttack() override;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_FireWeapon(FVector fwdVector);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void updateWeapon() override;
	virtual bool checkForHit() override;
	virtual void startAttackTimer() override;
	virtual void cancelAttackTimer() override;

	virtual void SpawnProjectile(FVector fwdVector);

private:
	int mAmmoCount = 1;
};
