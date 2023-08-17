// Copyright Epic Games, Inc. All Rights Reserved.

#include "TalesDungeoneerGameMode.h"
#include "TalesDungeoneer/Characters/CharacterBase.h"

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
