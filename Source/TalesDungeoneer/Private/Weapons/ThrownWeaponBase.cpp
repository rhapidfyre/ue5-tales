
#include "Weapons/ThrownWeaponBase.h"


AThrownWeaponBase::AThrownWeaponBase()
{
}

bool AThrownWeaponBase::doAttack()
{
	return Super::doAttack();
}

void AThrownWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AThrownWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AThrownWeaponBase::UpdateWeapon()
{
	Super::UpdateWeapon();
}

void AThrownWeaponBase::SpawnProjectile(FVector fwdVector)
{
	
}
