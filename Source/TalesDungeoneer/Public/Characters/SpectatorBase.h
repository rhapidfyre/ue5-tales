// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"

#include "SpectatorBase.generated.h"

UCLASS()
class TALESDUNGEONEER_API ASpectatorBase : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASpectatorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
