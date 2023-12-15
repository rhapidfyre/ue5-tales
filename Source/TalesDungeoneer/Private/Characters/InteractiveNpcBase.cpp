// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Characters/InteractiveNpcBase.h"

#include "Perception/AISense_Sight.h"


// Sets default values
AInteractiveNpcCharacterBase::AInteractiveNpcCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AInteractiveNpcCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	if (!bCanBeTargetedByCombatants)
	{
		AiStimuli->UnregisterFromSense(UAISense_Sight::StaticClass());
		AiStimuli->UnregisterFromPerceptionSystem();
		AiStimuli->Deactivate();
	}
}

// Called every frame
void AInteractiveNpcCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

