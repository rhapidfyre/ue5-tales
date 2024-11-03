// Starcache Studios, LLC (c) 2024

#include "Actors/TalesRespawnBase.h"


// Sets default values
ATalesRespawnBase::ATalesRespawnBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATalesRespawnBase::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ATalesRespawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
