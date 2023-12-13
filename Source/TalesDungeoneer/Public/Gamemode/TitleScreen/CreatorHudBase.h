// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "CreatorHudBase.generated.h"


UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ACreatorHudBase : public AHUD
{
	GENERATED_BODY()
	
public:
	ACreatorHudBase();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	virtual void BeginPlay() override;
};