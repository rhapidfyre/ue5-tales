// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gamemode/AdventureMode/TalesDungeoneerGameMode.h"
#include "Characters/CharacterBase.h"

#include "UObject/ConstructorHelpers.h"

ATalesDungeoneerGameMode::ATalesDungeoneerGameMode()
{
	
}

void ATalesDungeoneerGameMode::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (IsValid(DefaultCharacter))
		DefaultPawnClass = DefaultCharacter;
}
