
#include "ProjectileSpellBase.h"


AProjectileSpellBase::AProjectileSpellBase()
{
	PrimaryActorTick.bCanEverTick = true;

	ProjectileMovement = CreateDefaultSubobject
				<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
}

void AProjectileSpellBase::BeginPlay()
{
	Super::BeginPlay();
	
}
