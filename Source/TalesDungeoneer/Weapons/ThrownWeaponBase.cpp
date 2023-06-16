
#include "ThrownWeaponBase.h"

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

void AThrownWeaponBase::updateWeapon()
{
	Super::updateWeapon();
}

bool AThrownWeaponBase::checkForHit()
{
	return Super::checkForHit();
}

void AThrownWeaponBase::startAttackTimer()
{
	Super::startAttackTimer();
}

void AThrownWeaponBase::cancelAttackTimer()
{
	Super::cancelAttackTimer();
}

void AThrownWeaponBase::SpawnProjectile(FVector fwdVector)
{
	
}
