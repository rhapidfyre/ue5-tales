// Copyright Take Five Games, LLC 2023 - All rights reserved

#include "Characters/CreatorCharacterBase.h"


// Sets default values
ACreatorCharacterBase::ACreatorCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACreatorCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ACreatorCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}
