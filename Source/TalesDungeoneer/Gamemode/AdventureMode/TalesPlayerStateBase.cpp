// Fill out your copyright notice in the Description page of Project Settings.


#include "TalesPlayerStateBase.h"

void ATalesPlayerStateBase::UpdatePlayerName()
{
	OnPlayerNameUpdated.Broadcast();
}
