// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "NpcCharacterBase.h"

#include "CombatNpcCharacterBase.generated.h"

/* A combat NPC is any NPC character whose primary purpose is for combat. The
 * NPCs team and sensory data settings determine who the NPC fights.
 */
UCLASS()
class TALESDUNGEONEER_API ACombatNpcCharacterBase : public ANpcCharacterBase
{
	GENERATED_BODY()

public:
	
	ACombatNpcCharacterBase();
	
	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;

};
