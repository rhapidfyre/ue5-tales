// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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

	/**
	 * @brief Accessor for setting the movement vector instead of accessing the
	 *			projectile motion component directly.
	 * @param SpeedVector The speed of the projectile upon spawning
	 */
	UFUNCTION(BlueprintCallable) void SetProjectileSpeed(FVector SpeedVector);

	/**
	 * @brief Accessor for setting the gravity boolean instead of accessing the
	 *			projectile motion component directly.
	 * @param AllowGravity True will result in gravity affecting the projectile.
	 */
	UFUNCTION(BlueprintCallable) void SetGravityConsidered(bool AllowGravity = false);

	// Fires the projectile. Called when all setup is complete.
	UFUNCTION(BlueprintCallable) void FireProjectile();

	UFUNCTION(BlueprintPure) FVector GetImpactPosition() const;

protected:

	virtual void BeginPlay() override;

	/**
	 * @brief Performs a hit trace from the origination point, to either the target actor
	 *		  or the target location - Whichever is appropriate.
	 * @param HitResult The hit result information from the hit
	 * @return True if the hit was successful; False if it failed.
	 */
	virtual bool PerformSingleHitTrace(FHitResult& HitResult);

	/**
	 * @brief Performs a multi hit trace from the origination point, to either the
	 *        target actor or the target location - Whichever is appropriate.
	 *        The trace is stopped once it has hit something that blocks visibility.
	 * @param HitResults The hit results from the hit
	 * @return True if the hit was successful; False if it failed.
	 */
	virtual bool PerformMultiHitTrace(TArray<FHitResult>& HitResults);

	virtual void AbilityComplete(bool WasSuccessful) override;
	
public: // variables (members)

	// If true, uses a physics object. If false, uses A to B line tracing.
	UPROPERTY(BlueprintReadWrite, Category = "Actor Settings")
	bool bUsePhysics = true;

	FVector _SpeedVector = FVector(1.0f);
	float _GravityScale = 1.f;
};
