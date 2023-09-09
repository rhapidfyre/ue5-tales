// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "NpcCharacterBase.h"

#include "InteractiveNpcCharacterBase.generated.h"


/* An interactive NPC is any NPC character who is a non combatant, and will
 * interact with the player if they are activated, such as a banker, a merchant
 * or a quest giver.
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API AInteractiveNpcCharacterBase : public ANpcCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInteractiveNpcCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
