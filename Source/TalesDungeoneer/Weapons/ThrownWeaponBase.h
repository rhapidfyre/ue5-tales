
#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"

#include "ThrownWeaponBase.generated.h"

UCLASS(Blueprintable)
class TALESDUNGEONEER_API AThrownWeaponBase : public AWeaponBase
{
	GENERATED_BODY()

public:

	AThrownWeaponBase();

	virtual bool doAttack() override;
	
protected:

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void updateWeapon() override;
	virtual bool checkForHit() override;
	virtual void startAttackTimer() override;
	virtual void cancelAttackTimer() override;

	/** Deducts one item from the thrown weapon slot (primary/secondary), and on success,
	 * it will spawn a projectile of the same mesh, in it's fully activated state, and
	 * launch it as a new projectile actor (throwing a torch, or an axe for example).
	 * @param fwdVector The trajectory at initial spawn from the throwing actor
	 */
	virtual void SpawnProjectile(FVector fwdVector);

};
