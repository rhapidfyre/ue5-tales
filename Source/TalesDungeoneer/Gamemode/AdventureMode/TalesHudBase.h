// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "TalesHudBase.generated.h"


UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ATalesHudBase : public AHUD
{
	GENERATED_BODY()
	
public:
	ATalesHudBase();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	virtual void BeginPlay() override;
};
