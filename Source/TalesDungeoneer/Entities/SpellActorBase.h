// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilityEffectBase.h"

#include "SpellActorBase.generated.h"

/**
 * This is the base for all spells in the game.
 * Every spell has stuff in common.. For example, every spell has a
 * school of magic, a visual effect, a sound effect and a world location.
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ASpellActorBase : public AAbilityEffectBase
{
	GENERATED_BODY()

public:

	ASpellActorBase();
	
	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;

};
