// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#include "Gamemode/TitleScreen/CreatorGameModeBase.h"


ACreatorGameModeBase::ACreatorGameModeBase()
{
	
}

void ACreatorGameModeBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (IsValid(BpCharacter))
	{
		DefaultPawnClass = BpCharacter;
	}
}
