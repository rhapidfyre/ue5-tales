// Fill out your copyright notice in the Description page of Project Settings.


#include "AiControllerBase.h"

#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"

// Sets default values
AAiControllerBase::AAiControllerBase()
{
	if (!IsValid(AiPerception))
		AiPerception = CreateDefaultSubobject<UAIPerceptionComponent>("AiPerception");
	if (!IsValid(AiStimuli))
		AiStimuli = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("AiStimuli");

	// Setup Sight
	AiPerception->SetDominantSense(UAISense_Sight::StaticClass());
	const FAISenseID SenseId = UAISense::GetSenseID<UAISense_Sight>();
	UAISenseConfig_Sight* SenseConfig = Cast<UAISenseConfig_Sight>
		(AiPerception->GetSenseConfig(SenseId));
	if (IsValid(SenseConfig))
	{
		SenseConfig->SightRadius = GainSightRadius;
		SenseConfig->LoseSightRadius = LoseSightRadius;
	}
	
	
	AiStimuli->RegisterForSense(UAISense_Sight::StaticClass());
	AiStimuli->bAutoRegister = true;
	
}

void AAiControllerBase::BeginPlay()
{
	Super::BeginPlay();
}

void AAiControllerBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AAiControllerBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AAiControllerBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

