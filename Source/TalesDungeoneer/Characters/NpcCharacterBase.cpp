// Fill out your copyright notice in the Description page of Project Settings.


#include "NpcCharacterBase.h"


// Sets default values
ANpcCharacterBase::ANpcCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>("OverheadWidget");
	OverheadWidget->SetupAttachment(GetRootComponent());
	//OverheadWidget->SetIsReplicated(true);
}

void ANpcCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	SetCharacterTeam(ECharacterTeam::ENEMY);
}