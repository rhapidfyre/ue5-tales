// Fill out your copyright notice in the Description page of Project Settings.

#include "Gamemode/AdventureMode/TalesPlayerStateBase.h"


void ATalesPlayerStateBase::UpdatePlayerName()
{
	OnPlayerNameUpdated.Broadcast();
}
