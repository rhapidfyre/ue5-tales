
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

bool AThrownWeaponBase::CheckForHit(TArray<AActor*>& HitActors)
{
	return Super::CheckForHit(HitActors);
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
