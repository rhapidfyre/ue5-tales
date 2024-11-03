// Starcache Studios, LLC (c) 2024

#pragma once

#include "TalesHudBase.h"

#include "TalesPlayerHudBase.generated.h"

UCLASS()
class TALESDUNGEONEER_API ATalesPlayerHudBase : public ATalesHudBase
{
	GENERATED_BODY()

public:
	ATalesPlayerHudBase();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	virtual void BeginPlay() override;
};
