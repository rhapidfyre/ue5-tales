// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "TalesDungeoneer/lib/datastructures/AbilityData.h"

#include "ProjectileBase.generated.h"

// Forward Declaration
class ACharacterBase;

// A simple projectile class for bullets, cannonballs, etc
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	
	AProjectileBase();

	// If the projectile uses an ability effect
	AProjectileBase(FName AbilityName);

	virtual void OnConstruction(const FTransform& Transform) override;

	/**
	 * @brief Used if the projectile mimics a spell or ability effect
	 * @param AbilityName The name of the ability to mimic
	 */
	void SetProjectileData(FName AbilityName);

	UFUNCTION(BlueprintCallable)
	void SetGravityConsidered(bool AllowGravity = true);
	
	UFUNCTION(BlueprintCallable)
	void SetProjectileScale(float newScale = 1.f);
	
	UFUNCTION(BlueprintCallable)
	void SetProjectileSpeed(FVector SpeedVector = FVector(0.f));
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetTargetActor(AActor* TargetActor) { _TargetActor = TargetActor; }
	AActor* GetTargetActor() const { return _TargetActor; };

	FName GetAbilityName() const { return _AbilityName; };
	
protected:

	virtual void BeginPlay() override;

	virtual void Destroyed() override;

private:
	
	void SetupDefaults();
	
	void SetFromAbilityData();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_ImpactEffects(FVector HitVector);

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UArrowComponent* Arrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* SphereCollision;
	
	UFUNCTION(NetMulticast, Unreliable) void OnRep_AbilityName();
	
private:
	/**
	 * @brief Called by the engine when this actor collides with a pawn or visibility
	 * @param HitComp The Component hit by the collision
	 * @param OtherActor The actor collided with
	 * @param OtherComp The component collided with
	 * @param NormalImpulse The impulse coming off the normal
	 * @param Hit The FHitResult data
	 */
	UFUNCTION()
	void CheckCollision(UPrimitiveComponent* HitComp, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, FVector NormalImpulse,
						const FHitResult& Hit);

	/**
	 * @brief Called by the engine when this actor overlaps
	 * @param OverlappedComponent The component overlapping the collision
	 * @param OtherActor The actor overlapped by the collision
	 * @param OtherComp The component oaverlapped by the collision
	 * @param OtherBodyIndex Other Body Index
	 * @param bFromSweep Was this overlap a result of a sweep request
	 * @param SweepResult The result of the overlap
	 */
	UFUNCTION()
	void CheckOverlap(UPrimitiveComponent* OverlappedComponent,
					  AActor* OtherActor, UPrimitiveComponent* OtherComp,
					  int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ApplyHitEffect(ACharacterBase* HitCharacter, FVector HitVector);

	UPROPERTY() AActor* _TargetActor;
	
	UPROPERTY() FStAbilityData _AbilityData;

	UPROPERTY(ReplicatedUsing=OnRep_AbilityName) FName _AbilityName = FName();
	
	UPROPERTY(Replicated) float _GravityScale = 1.f;
	UPROPERTY(Replicated) FVector _SpeedVector = FVector(0.f);

	UPROPERTY() FVector _OriginLocation;
	
};
