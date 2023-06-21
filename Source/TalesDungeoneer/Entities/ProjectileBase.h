// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

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

	virtual void OnConstruction(const FTransform& Transform) override;

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

	FVector GetOriginLocation() const { return _OriginLocation; }

	UFUNCTION(BlueprintCallable)
	void SetMaxTravelDistance(float MaxTravelDistance = 2048.f);
	float GetMaxTravelDistance() const { return _MaxTravelDistance; }
	
protected:

	virtual void BeginPlay() override;

	virtual void Destroyed() override;

	virtual void ApplyHitEffect(AActor* HitCharacter, FVector HitVector);

private:

	void SetupDefaults();

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UArrowComponent* Arrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* SphereCollision;
	
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

	UPROPERTY() AActor* _TargetActor;
	UPROPERTY() float _MaxTravelDistance = 2048.f;
	
	UPROPERTY(Replicated) float _GravityScale = 1.f;
	UPROPERTY(Replicated) FVector _SpeedVector = FVector(0.f);

	UPROPERTY() FVector _OriginLocation;
	
};
