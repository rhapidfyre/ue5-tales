
#include "Weapons/RangedWeaponBase.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "lib/datastructures/WeaponData.h"


ARangedWeaponBase::ARangedWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
	ProjectileDirection = CreateDefaultSubobject<UArrowComponent>("ProjectileDirection");
}

void ARangedWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void ARangedWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ARangedWeaponBase::UpdateWeapon()
{
	Super::UpdateWeapon();
}

bool ARangedWeaponBase::doAttack()
{
	// If parent's criteria is good to go, we can proceed
	if ( Super::doAttack() )
	{
		return true;
	}
	return false;
}

void ARangedWeaponBase::Server_FireWeapon_Implementation(FVector fwdVector)
{
	
}

void ARangedWeaponBase::SpawnProjectile(FVector fwdVector)
{

	/*
	AProjectileBase* bullet = GetWorld()->SpawnActorDeferred<AProjectileBase>(AProjectileBase::StaticClass(), spawnTransform,
		nullptr, Cast<APawn>(GetOwner()), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (IsValid(bullet))
	{
		bullet->SetProjectileScale(10.f);
		bullet->ProjectileMovement->InitialSpeed = 1000;
		bullet->ProjectileMovement->Velocity = fwdVector * bullet->ProjectileMovement->InitialSpeed;
		bullet->FinishSpawning(spawnTransform);

		// This is the sound effect that plays for players outside the anim render distance
		// Such as a distant gunfire or a distant cannon blast
		const FStWeaponSoundData soundData = weaponData.WeaponSounds;
		if (IsValid(soundData.UseSoundWeaponAttack))
			soundEffectWithDelay(soundData.UseSoundWeaponAttack, soundData.DelaySoundWeaponAttack);
	}
	*/
}

