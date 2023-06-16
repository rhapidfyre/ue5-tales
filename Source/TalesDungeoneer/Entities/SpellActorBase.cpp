// Fill out your copyright notice in the Description page of Project Settings.


#include "SpellActorBase.h"


// Sets default values
ASpellActorBase::ASpellActorBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASpellActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpellActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

