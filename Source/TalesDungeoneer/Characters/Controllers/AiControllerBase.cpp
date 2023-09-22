// Fill out your copyright notice in the Description page of Project Settings.


#include "AiControllerBase.h"

#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "TalesDungeoneer/Characters/NpcCharacterBase.h"

class ACharacterBase;
// Sets default values
AAiControllerBase::AAiControllerBase()
{
	if (!IsValid(AiPerception))
		AiPerception = CreateDefaultSubobject<UAIPerceptionComponent>("AiPerception");
	AiPerception->SetAutoActivate(true);
}

void AAiControllerBase::BeginPlay()
{
	Super::BeginPlay();
	ANpcCharacterBase* CharacterBase = Cast<ANpcCharacterBase>( GetCharacter() );
	if (IsValid(CharacterBase))
	{
		 _PatrolArea = CharacterBase->GetNpcIsPatroller();
	}
}

void AAiControllerBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IsValid(BehaviorTree))
	{
		RunBehaviorTree(BehaviorTree);
	}
	
	// Setup Sight
	AiPerception->SetDominantSense(UAISense_Sight::StaticClass());
	const FAISenseID SenseId = UAISense::GetSenseID<UAISense_Sight>();
	UAISenseConfig_Sight* SenseConfig = Cast<UAISenseConfig_Sight>
		(AiPerception->GetSenseConfig(SenseId));
	if (IsValid(SenseConfig))
	{
		SenseConfig->SightRadius = GainSightRadius;
		SenseConfig->LoseSightRadius = LoseSightRadius;
		FAISenseAffiliationFilter AffiliationFilter;
		AffiliationFilter.bDetectEnemies = true;
		AffiliationFilter.bDetectFriendlies = true;
		AffiliationFilter.bDetectNeutrals = true;
		SenseConfig->DetectionByAffiliation = AffiliationFilter;
	}
}

void AAiControllerBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AAiControllerBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

