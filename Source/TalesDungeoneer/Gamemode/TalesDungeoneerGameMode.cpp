// Copyright Epic Games, Inc. All Rights Reserved.

#include "TalesDungeoneerGameMode.h"
#include "TalesDungeoneer/Characters/CharacterBase.h"

#include "UObject/ConstructorHelpers.h"

ATalesDungeoneerGameMode::ATalesDungeoneerGameMode()
{
	
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(
			TEXT("/Game/TalesContent/Blueprints/TalesPlayerCharacter"));
	
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	
}
