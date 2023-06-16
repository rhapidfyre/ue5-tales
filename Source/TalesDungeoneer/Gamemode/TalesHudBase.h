// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "TalesHudBase.generated.h"

UCLASS()
class TALESDUNGEONEER_API ATalesHudBase : public AHUD
{
	GENERATED_BODY()
	
public:
	ATalesHudBase();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	virtual void BeginPlay() override;
};
