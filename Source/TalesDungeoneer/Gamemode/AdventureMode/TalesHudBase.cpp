// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "TalesHudBase.h"

ATalesHudBase::ATalesHudBase()
{
	
}

void ATalesHudBase::DrawHUD()
{
	Super::DrawHUD();
}

void ATalesHudBase::BeginPlay()
{
	Super::BeginPlay();
}

void ATalesHudBase::NotifyHud(FString MessageTitle, FString NewMessage, int Priority)
{
	FStHudNotification NewNotification(MessageTitle, NewMessage, Priority);
}
