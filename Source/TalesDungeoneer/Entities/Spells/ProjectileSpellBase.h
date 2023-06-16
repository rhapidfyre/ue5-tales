// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/ProjectileMovementComponent.h"
#include "TalesDungeoneer/Entities/SpellActorBase.h"

#include "ProjectileSpellBase.generated.h"

/**
 * A Projectile Spell is a spell that is fired, like a bullet, from one point
 * to another. The behavior of the projectile is determined by the individual
 * spell being used.
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API AProjectileSpellBase : public ASpellActorBase
{
	GENERATED_BODY()

public:

	AProjectileSpellBase();

protected:

	virtual void BeginPlay() override;

public: // variables (members)

	// If true, uses a physics object. If false, uses A to B line tracing.
	UPROPERTY(BlueprintReadWrite, Category = "Actor Settings")
	bool bUsePhysics = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UProjectileMovementComponent* ProjectileMovement;
};
