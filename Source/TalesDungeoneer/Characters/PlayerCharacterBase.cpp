// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterBase.h"

#include "PickupActorBase.h"


// Sets default values
APlayerCharacterBase::APlayerCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	SetCharacterTeam(ECharacterTeam::PLAYER);
}
