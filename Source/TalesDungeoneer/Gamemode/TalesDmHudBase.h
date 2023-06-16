// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TalesHudBase.h"

#include "TalesDmHudBase.generated.h"

UCLASS()
class TALESDUNGEONEER_API ATalesDmHudBase : public ATalesHudBase
{
	GENERATED_BODY()
	
public:
	ATalesDmHudBase();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	virtual void BeginPlay() override;
};
