
#include "RangedWeaponBase.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TalesDungeoneer/Entities/ProjectileBase.h"
#include "TalesDungeoneer/lib/datastructures/WeaponData.h"


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

void ARangedWeaponBase::updateWeapon()
{
	Super::updateWeapon();
}

bool ARangedWeaponBase::doAttack()
{
	// If parent's criteria is good to go, we can proceed
	if ( Super::doAttack() )
	{
		startAttackTimer();
		return true;
	}
	return false;
}

void ARangedWeaponBase::Server_FireWeapon_Implementation(FVector fwdVector)
{
	const FStWeaponData weaponData = getWeaponData();

	if (weaponData.HitDetectDelay > 0.f)
	{
		FTimerDelegate timerArg = FTimerDelegate::CreateUObject(
			this, &ARangedWeaponBase::SpawnProjectile, fwdVector);
		FTimerHandle projTimer;
		GetWorld()->GetTimerManager().SetTimer(projTimer, timerArg, weaponData.HitDetectDelay, false);
	}
	else
	{
		SpawnProjectile(fwdVector);
	}
}

void ARangedWeaponBase::SpawnProjectile(FVector fwdVector)
{
	const FStWeaponData weaponData = getWeaponData();
	const FTransform barrelTransform = ProjectileDirection->GetComponentTransform();
	const FTransform spawnTransform(barrelTransform.GetLocation());
	
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
}


bool ARangedWeaponBase::checkForHit(TArray<AActor*>& HitActors)
{
	return true;
}

void ARangedWeaponBase::startAttackTimer()
{
	Super::startAttackTimer();
}

void ARangedWeaponBase::cancelAttackTimer()
{
	Super::cancelAttackTimer();
	
	// When cancelAttackTimer is called, that's when the gun fires.
	ACharacter* charRef = Cast<ACharacter>(GetOwner());
	if (IsValid(charRef))
	{
		// If NPC, get focal point
		// If player, get camera manager
		const AController* aController = charRef->GetController();
		if (IsValid(aController))
		{
			// Is Player
			if (aController->IsLocalPlayerController())
			{
				APlayerCameraManager* camManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(),0);
				if (IsValid(camManager))
					Server_FireWeapon(camManager->GetActorForwardVector());
			}
			// Is another player, or is NPC
			else
			{
				
			}
		}
		// If not NPC nor player, it can't shoot
		else
		{
			
		}
	}
}
